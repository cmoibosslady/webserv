#!/usr/bin/env python3
import os
print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Environment Variables</h1>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}:</strong> {value}</li>")

