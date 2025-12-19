#!/usr/bin/env python3
import os, urllib.parse, re

DATA_FILE = os.path.join(os.path.dirname(__file__), "./cadets.txt")
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "../cadets/uploads/")

# READ PARAMETER FILE
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
filename = params.get("file", [""])[0]
log.write("filename")
log.write(filename)
filename = os.path.basename(filename)
filename = re.sub(r'[^\w\s.-]', '', filename)
log.write(filename)

if not filename:
    print("Status: 400 Bad Request")
    print("Content-Type: text/html")
    print(f"<h1>Error: Invalid filename</h1>")
    print(f"<p>Query: {query}</p>")
    print(f"<p>Raw: {filename}</p>")
    print()
    print("<h1>Error: Invalid filename</h1>")
    exit()

target = os.path.join(UPLOAD_DIR, filename)
real_target = os.path.realpath(target)
real_upload_dir = os.path.realpath(UPLOAD_DIR)

if not real_target.startswith(real_upload_dir):
    print("Status: 403 Forbidden")
    print("Content-Type: text/html")
    print()
    print("<h1>Error: Access denied</h1>")
    exit()

# DELETE FOTO
if os.path.exists(target):
    os.remove(target)

# REMOVE ENTRY FROM cadets.txt
new_lines = []
try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            if filename not in line:
                new_lines.append(line)
    with open(DATA_FILE, "w") as f:
        f.writelines(new_lines)
except IOError:
    print("Status: 500 Internal Server Error\r")
    print("Content-Type: text/html\r")
    print("\r")
    print("<h1>Error processing request</h1>")
    exit()

# REDIRECT FOR LIST
print("Status: 303 See Other\r")
print("Location: /cadets_list\r")
print("Content-Type: text/html\r")
print("\r")
print("<html><body>Redirecting...</body></html>")