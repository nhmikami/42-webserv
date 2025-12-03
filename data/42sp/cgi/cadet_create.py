#!/usr/bin/python3
import os
import cgi

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cadets/cadets.txt")

print("Content-Type: text/html")
print()

form = cgi.FieldStorage()

name = form.getfirst("name", "unknown")
photo_field = form["photo"]

# O webserver já salvou a foto no upload_store
# photo_field.filename é o nome do arquivo

with open(DATA_FILE, "a") as f:
    f.write(f"{name},{photo_field.filename}\n")

# Redireciona para a página de cadets
print("Status: 303 See Other")
print("Location: /cadets")
print()

