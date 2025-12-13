#!/usr/bin/env python3
import os
import cgi
import re

DATA_FILE = os.path.join(os.path.dirname(__file__), "cadets.txt")

form = cgi.FieldStorage()

name = form.getfirst("name", "unknown")
name = re.sub(r'[^\w\s-]', '', name).strip()[:50] #sanitize filename
if not name:
    print("Content-Type: text/html\n")
    print("<h1>Error: Invalid name</h1>")
    exit()

photo_field = form["photo"]
if not photo_field.filename:
    print("Content-Type: text/html\n")
    print("<h1>Error: No file uploaded</h1>")
    exit()

filename = os.path.basename(photo_field.filename)
filename = re.sub(r'[^\w\s.-]', '', filename)

with open(DATA_FILE, "a") as f:
    f.write(f"{name},{filename}\n")

# Redireciona para a página de cadets
print("Status: 303 See Other")
print("Location: /cadets")
print()

