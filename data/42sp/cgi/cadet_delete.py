#!/usr/bin/python3
import os, urllib.parse

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cadets/cadets.txt")
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "../cadets/uploads/")

# LER PARÂMETRO FILE
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
filename = params.get("file", [""])[0]
# Sanitize filename to prevent path traversal
filename = os.path.basename(filename)

# APAGAR FOTO
target = os.path.join(UPLOAD_DIR, filename)
if os.path.exists(target):
    os.remove(target)

# REMOVER ENTRADA DO cadets.txt
new_lines = []
with open(DATA_FILE, "r") as f:
    for line in f:
        if filename not in line:
            new_lines.append(line)

with open(DATA_FILE, "w") as f:
    f.writelines(new_lines)

# REDIRECIONAR PARA A LISTA
print("Status: 303 See Other")
print("Location: /cadets")
print()
