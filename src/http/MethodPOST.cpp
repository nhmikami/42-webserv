#include "http/MethodPOST.hpp"
#include "utils/FileUtils.hpp"
#include "utils/Logger.hpp"

MethodPOST::MethodPOST(const Request& req, const ServerConfig& config, const LocationConfig* location)
	: AMethod(req, config, location) {}

MethodPOST::~MethodPOST(void) {}

HttpStatus MethodPOST::handleMethod(void) {
	if (_req.getBody().size() > _getMaxBodySize())
		return PAYLOAD_TOO_LARGE;

	std::string full_path = FileUtils::resolvePath(_getRootPath(), _stripLocationPrefix(_req.getPath()));

	if (_req.getContentType().find("multipart/form-data") != std::string::npos) {
		HttpStatus uploadStatus = _handleMultipart();
        if (uploadStatus != CREATED)
            return uploadStatus;
        if (_isCGI(full_path))
            return _runCGI(full_path, true);
        return CREATED;
	}

	if (FileUtils::exists(full_path) && _isCGI(full_path) && FileUtils::isFile(full_path))
		return _runCGI(full_path, false);

	if (FileUtils::isDirectory(full_path)) {
		return BAD_REQUEST;
	}

	bool fileExisted = FileUtils::exists(full_path);
	if (!fileExisted) {
		std::string parent = full_path.substr(0, full_path.find_last_of('/'));
		if (!FileUtils::exists(parent) || !FileUtils::isDirectory(parent))
			return NOT_FOUND;
		if (!FileUtils::isWritable(parent)) {
			return FORBIDDEN;
		}
	}
	else if (!FileUtils::isWritable(full_path)) {
		return FORBIDDEN;
	}

	if (_writeToFile(full_path, _req.getBody())) {
		if (!fileExisted) {
			_res.addHeader("Location", _buildAbsoluteUrl(FileUtils::normalizePath(_req.getPath())));
			_res.setBody("File created successfully");
			_res.addHeader("Content-Type", "text/plain");
			return CREATED;
		}
		_res.setBody("File updated successfully");
		_res.addHeader("Content-Type", "text/plain");
		return OK;
	}

	return SERVER_ERR;
}

bool MethodPOST::_writeToFile(const std::string& path, const std::string& body) {
	return _writeToFile(path, body.c_str(), body.size());
}

bool MethodPOST::_writeToFile(const std::string& path, const char* buffer, size_t size) {
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	int fd = open(path.c_str(), flags, 0644);
	if (fd < 0)
		return false;

	size_t total_written = 0;
	while (size > 0) {
		ssize_t written = write(fd, buffer + total_written, size);
		if (written < 0) {
			close(fd);
			return false;
		}
		total_written += written;
		size -= written;
	}

	close(fd);
	return true;
}

std::string MethodPOST::_buildAbsoluteUrl(const std::string& targetPath) {
	std::string host = _req.getHost();
	if (host.empty())
		host = "localhost";
	return "http://" + host + targetPath;
}

std::string MethodPOST::_extractFilename(const std::string& filename) {
	std::string base = filename;
	size_t lastSlash = base.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		base = base.substr(lastSlash + 1);
	return base;
}

const std::map<std::string, std::string>& MethodPOST::getFormFields(void) const {
	return _formFields;
}

HttpStatus MethodPOST::_runCGI(const std::string& path, bool useFormFields) {
	if (!FileUtils::isReadable(path) || !FileUtils::isExecutable(path))
		return FORBIDDEN;

	std::map<std::string, std::string> executors = _getCgiExecutors();
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos || dotPos >= path.length() - 1)
		return SERVER_ERR;
	
	std::string	ext = path.substr(dotPos);
	std::string	executor = executors[ext];

	if (useFormFields)
		_cgiHandler = new CgiHandler(_req, path, executor, _formFields);
	else
		_cgiHandler = new CgiHandler(_req, path, executor);
	
	_cgiHandler->start();

	return CGI_PENDING;
}

HttpStatus MethodPOST::_handleMultipart(void) {
	std::string uploadLoc = _getUploadLocation();
	if (!FileUtils::exists(uploadLoc) || !FileUtils::isDirectory(uploadLoc) || !FileUtils::isWritable(uploadLoc)) {
		return FORBIDDEN;
	}

	std::string contentType = _req.getContentType();
	std::string boundary = ParseUtils::extractAttribute(contentType, "boundary");
	if (boundary.empty())
		return BAD_REQUEST;

	std::string sep = "--" + boundary;
	const std::string &body = _req.getBody();
	
	size_t pos = body.find(sep);
	if (pos == std::string::npos)
		return BAD_REQUEST;
	pos += sep.size();

	while (true) {
		if (pos + 2 <= body.size() && body.compare(pos, 2, "--") == 0)
			break ;

		if (pos + 2 <= body.size() && body.compare(pos, 2, "\r\n") == 0)
			pos += 2;

		size_t headersEnd = body.find("\r\n\r\n", pos);
		if (headersEnd == std::string::npos)
			return BAD_REQUEST;

		std::string headers = body.substr(pos, headersEnd - pos);
		size_t fileStart = headersEnd + 4;
		std::string filename = ParseUtils::extractAttribute(headers, "filename");
		std::string fieldName = ParseUtils::extractAttribute(headers, "name");
		size_t nextBoundary = body.find(sep, fileStart);
		if (nextBoundary == std::string::npos)
			return BAD_REQUEST;

		// Skip parts without a field name
		if (fieldName.empty()) {
			pos = nextBoundary + sep.size();
			continue;
		}

		if (!filename.empty()) {
			// Handle file upload field
			size_t fileEnd = nextBoundary;
			if (fileEnd >= 2 && body.compare(fileEnd - 2, 2, "\r\n") == 0)
				fileEnd -= 2;

			filename = _extractFilename(filename);
			std::string outPath = FileUtils::resolvePath(uploadLoc, filename);
			size_t dataSize = (fileEnd > fileStart) ? fileEnd - fileStart : 0;
			if (!_writeToFile(outPath, body.c_str() + fileStart, dataSize))
				return SERVER_ERR;
			
			// Store uploaded filename for CGI access with __FILENAME suffix to avoid naming conflicts
			_formFields[fieldName + "__FILENAME"] = filename;
		} else {
			// Handle regular form field
			size_t dataEnd = nextBoundary;
			if (dataEnd >= 2 && body.compare(dataEnd - 2, 2, "\r\n") == 0)
				dataEnd -= 2;
			std::string fieldValue = body.substr(fileStart, dataEnd - fileStart);
			_formFields[fieldName] = fieldValue;
		}
		pos = nextBoundary + sep.size();
	}
	_res.setBody("Files uploaded successfully\n");
	_res.addHeader("Content-Type", "text/plain");
	return CREATED;
}
