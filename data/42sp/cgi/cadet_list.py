#!/usr/bin/env python3
import os, html
import urllib.parse

DATA_FILE = os.path.join(os.path.dirname(__file__), "cadets.txt")
print("Content-Type: text/html")
print()

output = f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Cadet's List</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body class="bg">
    <div class="columns">
"""

try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            name, photo = line.strip().split(",")
            photo_encoded = urllib.parse.quote(photo)
            output += f"""
            <div class="card">
                <img src="/cadets/uploads/{photo_encoded}" alt="{html.escape(name)}">
                <div class="card-content">
                    <div class="subtitle">{html.escape(name)}</div>
                    <form method="POST" action="./cgi/cadet_delete.py">
                        <button type="submit" class="delete-btn">DELETE</button>
                    </form>
                </div>
            </div>
            """

except Exception:
    output += '<div class="no-cadets">No cadets registered.</div>'


output += """
</div>
</body>
</html>
"""

print(output)