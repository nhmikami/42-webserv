#!/usr/bin/env python3
import os
import sys
import re
import cgi

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cgi/cadets.txt")

# Use Python's standard CGI module to parse form data
form = cgi.FieldStorage()

# Extract the 'name' field
name = form.getfirst("name", "unknown")
name = re.sub(r'[^\w\s-]', '', name).strip()[:50]
if not name:
    print("Content-Type: text/html")
    print()
    print("<h1>Error: Invalid name</h1>")
    sys.stdout.flush()
    exit()

# Extract the 'photo' field
photo_field = form["photo"] if "photo" in form else None
if not photo_field or not photo_field.filename:
    print("Content-Type: text/html")
    print()
    print("<h1>Error: No file uploaded</h1>")
    sys.stdout.flush()
    exit()

# Sanitize the filename
filename = os.path.basename(photo_field.filename)
filename = re.sub(r'[^\w\s.-]', '', filename)

# Save the cadet data
try:
    with open(DATA_FILE, "a") as f:
        f.write(f"{name},{filename}\n")
except (OSError, IOError):
    print("Content-Type: text/html")
    print()
    print("<h1>Error: Could not save data</h1>")
    sys.stdout.flush()
    exit()

# Redirect to the cadets list
print("Status: 303 See Other")
print("Location: /cadets_list")
print("Content-Type: text/html")
print()
print("<html><body>Redirecting...</body></html>")
sys.stdout.flush()