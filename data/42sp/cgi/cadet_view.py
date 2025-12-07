#!/usr/bin/python3
import os, urllib.parse

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cadets/cadets.txt")
UPLOAD_DIR = "/cadets/uploads/"

# ------------------------------
# LER NOME DO ARQUIVO
# ------------------------------
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
filename = params.get("file", [""])[0]
# Sanitize filename to prevent path traversal
filename = os.path.basename(filename)

# ------------------------------
# BUSCAR O NOME DO CADETE
# ------------------------------
cadet_name = "Unknown"

try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            name, photo = line.strip().split(",")
            if photo == filename:
                cadet_name = name
                break
except:
    pass

# ------------------------------
# HTML DE RESPOSTA
# ------------------------------
print("Content-Type: text/html")
print()

print(f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Cadet {cadet_name}</title>
    <link rel="stylesheet" href="/style.css">
</head>

<body class="bg">
    <p class="title">{cadet_name}</p>

    <img src="{UPLOAD_DIR}{filename}" style="max-width:400px; border-radius:12px; margin-top:20px">

    <form method="POST" action="/cadet_delete?file={filename}">
        <button class="btn" type="submit">Delete Cadet</button>
    </form>

</body>
</html>
""")
