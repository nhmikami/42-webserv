#!/usr/bin/python3
import os

DATA_FILE = os.path.join(os.path.dirname(__file__), "../cadets/cadets.txt")
print("Content-Type: text/html")
print()

print("<ul>")

try:
    with open(DATA_FILE, "r") as f:
        for line in f:
            name, photo = line.strip().split(",")
            print(f"""
            <li>
                {name} –
                <a href="/cadets/uploads/{photo}" target="_blank">Photo</a>
                <form style='display:inline' method="POST" action="/cadet?delete={photo}">
                    <button type="submit">Delete</button>
                </form>
            </li>
            """)
except Exception:
    print("<li>No cadets registered.</li>")

print("</ul>")
