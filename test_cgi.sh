#!/bin/sh

set -u

BASE_URL="${1:-http://127.0.0.1:8082}"
TMP_DIR="$(mktemp -d)"
PASS=0
FAIL=0

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT INT TERM

check_contains() {
    file="$1"
    needle="$2"
    label="$3"

    if grep -Fq "$needle" "$file"; then
        return 0
    fi

    echo "    [KO] missing: $label"
    return 1
}

run_case() {
    name="$1"
    method="$2"
    path="$3"
    data="$4"
    expect_body_1="$5"
    expect_body_2="$6"
    expect_body_3="$7"
    expect_header="$8"

    headers="$TMP_DIR/${name}.headers"
    body="$TMP_DIR/${name}.body"

    echo "- $name"

    if [ "$method" = "GET" ]; then
        if ! curl -sS -D "$headers" -o "$body" "$BASE_URL$path"; then
            echo "  [KO] request failed"
            FAIL=$((FAIL + 1))
            return
        fi
    else
        if ! curl -sS -D "$headers" -o "$body" \
            -X POST \
            -H "Content-Type: application/x-www-form-urlencoded" \
            --data "$data" \
            "$BASE_URL$path"; then
            echo "  [KO] request failed"
            FAIL=$((FAIL + 1))
            return
        fi
    fi

    ok=1

    if ! grep -qi '^HTTP/.* 200' "$headers"; then
        echo "    [KO] HTTP status != 200"
        ok=0
    fi

    check_contains "$body" "$expect_body_1" "$expect_body_1" || ok=0
    check_contains "$body" "$expect_body_2" "$expect_body_2" || ok=0
    check_contains "$body" "$expect_body_3" "$expect_body_3" || ok=0

    if [ -n "$expect_header" ]; then
        check_contains "$headers" "$expect_header" "$expect_header" || ok=0
    fi

    if [ "$ok" -eq 1 ]; then
        echo "  [OK]"
        PASS=$((PASS + 1))
    else
        echo "  [KO]"
        FAIL=$((FAIL + 1))
        echo "    Body sample:"
        head -n 8 "$body" | sed 's/^/    /'
    fi
}

echo "Running CGI checks against: $BASE_URL"

run_case "sh_get" "GET" "/test.sh?hello=world&lang=fr" "" \
    "CGI Shell - OK" \
    "REQUEST_METHOD</strong>: GET" \
    "QUERY_STRING</strong>: hello=world&lang=fr" \
    ""

run_case "sh_post" "POST" "/test.sh" "name=alice&role=test" \
    "CGI Shell - OK" \
    "REQUEST_METHOD</strong>: POST" \
    "name=alice&role=test" \
    ""

run_case "pl_get" "GET" "/test.pl?hello=world&lang=fr" "" \
    "CGI Perl - OK" \
    "REQUEST_METHOD</strong>: GET" \
    "QUERY_STRING</strong>: hello=world&lang=fr" \
    "Set-Cookie: cgi_perl_test=ok"

run_case "pl_post" "POST" "/test.pl" "name=alice&role=test" \
    "CGI Perl - OK" \
    "REQUEST_METHOD</strong>: POST" \
    "name=alice&role=test" \
    "Set-Cookie: cgi_perl_test=ok"

echo ""
echo "Summary: $PASS passed, $FAIL failed"

if [ "$FAIL" -ne 0 ]; then
    exit 1
fi

exit 0
