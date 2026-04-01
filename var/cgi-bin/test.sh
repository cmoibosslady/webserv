#!/bin/sh

escape_html() {
    printf '%s' "$1" | sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g'
}

read_body() {
    if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ] 2>/dev/null; then
        dd bs=1 count="$CONTENT_LENGTH" 2>/dev/null
    else
        cat
    fi
}

BODY=""
if [ "$REQUEST_METHOD" = "POST" ]; then
    BODY="$(read_body)"
fi

ESC_QUERY="$(escape_html "${QUERY_STRING:-}")"
ESC_BODY="$(escape_html "$BODY")"

printf 'Status: 200 OK\r\n'
printf 'Content-Type: text/html; charset=UTF-8\r\n\r\n'

cat <<EOF
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <title>CGI SH Test</title>
</head>
<body>
  <h1>CGI Shell - OK</h1>
  <p>Ce script confirme l'exécution CGI en <strong>sh</strong>.</p>

  <h2>Request</h2>
  <ul>
    <li><strong>REQUEST_METHOD</strong>: $(escape_html "${REQUEST_METHOD:-}")</li>
    <li><strong>SCRIPT_NAME</strong>: $(escape_html "${SCRIPT_NAME:-}")</li>
    <li><strong>SERVER_PROTOCOL</strong>: $(escape_html "${SERVER_PROTOCOL:-}")</li>
    <li><strong>QUERY_STRING</strong>: ${ESC_QUERY}</li>
    <li><strong>CONTENT_TYPE</strong>: $(escape_html "${CONTENT_TYPE:-}")</li>
    <li><strong>CONTENT_LENGTH</strong>: $(escape_html "${CONTENT_LENGTH:-}")</li>
  </ul>

  <h2>Payload POST</h2>
  <pre>${ESC_BODY}</pre>

  <h2>Headers utiles</h2>
  <ul>
    <li><strong>HTTP_HOST</strong>: $(escape_html "${HTTP_HOST:-}")</li>
    <li><strong>HTTP_USER_AGENT</strong>: $(escape_html "${HTTP_USER_AGENT:-}")</li>
    <li><strong>REMOTE_ADDR</strong>: $(escape_html "${REMOTE_ADDR:-}")</li>
  </ul>
</body>
</html>
EOF
