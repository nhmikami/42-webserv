#!/usr/bin/env python3
import os
import cgi
import re

DATA_FILE = os.path.join(os.path.dirname(__file__), "cadets.txt")

print("Content-Type: text/html")
print()

form = cgi.FieldStorage()

name = form.getfirst("name", "unknown")
name = re.sub(r'[^\w\s-]', '', name).strip()[:50] #sanitize filename
if not name:
    print("<h1>Error: Invalid name</h1>")
    exit()

photo_field = form["photo"]
if not photo_field.filename:
    print("<h1>Error: No file uploaded</h1>")
    exit()

filename = os.path.basename(photo_field.filename)
filename = re.sub(r'[^\w\s.-]', '', filename)

allowed_extensions = ['.jpg', '.jpeg', '.png', '.gif', '.webp']
file_ext = os.path.splitext(filename)[1].lower()
if file_ext not in allowed_extensions:
    print("<h1>Error: Only image files are allowed</h1>")
    exit()

upload_path = os.path.join(UPLOAD_DIR, filename)
if not os.path.exists(upload_path):
    print("<h1>Error: File upload failed</h1>")
    exit()

real_upload_path = os.path.realpath(upload_path)
real_upload_dir = os.path.realpath(UPLOAD_DIR)
if not real_upload_path.startswith(real_upload_dir):
    print("<h1>Error: Invalid file path</h1>")
    exit()

with open(DATA_FILE, "a") as f:
    f.write(f"{name},{filename}\n")

# Redireciona para a página de cadets
print("Status: 303 See Other")
print("Location: /cadets")
print()

