
#!/usr/bin/env python3
import os
import sys
import re

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cgi/cadets.txt")

# Read form data from environment variables set by the C++ server
# The server parses multipart form data and provides fields via FORM_* environment variables
name = os.environ.get('FORM_NAME', 'unknown')
filename = os.environ.get('FORM_PHOTO_FILENAME', '')

# Sanitize the name
name = re.sub(r'[^\w\s-]', '', name).strip()[:50]
if not name:
    print("Content-Type: text/html\n")
    print("<h1>Error: Invalid name</h1>")
    exit()

if not filename:
    print("Content-Type: text/html\n")
    print("<h1>Error: No file uploaded</h1>")
    exit()

# Additional filename sanitization for safety
filename = os.path.basename(filename)
filename = re.sub(r'[^\w\s.-]', '', filename)

try:
    with open(DATA_FILE, "a") as f:
        f.write(f"{name},{filename}\n")
except (OSError, IOError):
    print("Content-Type: text/html")
    print()
    print("<h1>Error: Could not save data</h1>")
    sys.stdout.flush()
    exit()

print("Status: 303 See Other")
print("Location: /cadets_list")
print("Content-Type: text/html")
print()
print("<html><body>Redirecting...</body></html>")
sys.stdout.flush()
