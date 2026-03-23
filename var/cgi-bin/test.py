#!/usr/bin/env python3
import html
import os

print("<!doctype html>")
print("<html><head><meta charset='utf-8'><title>Environment Variables</title></head><body>")
print("<h1>Environment Variables</h1>")
print("<ul>")

for key, value in sorted(os.environ.items()):
    k = html.escape(str(key))
    v = html.escape(str(value))
    print(f"<li><strong>{k}</strong>: {v}</li>")

print("</ul>")
print("</body></html>")