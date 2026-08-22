#!/usr/bin/env bash
# jq-free version — prints raw JSON instead of pretty-printed.

set -euo pipefail

BASE_URL="${BASE_URL:-http://localhost:8080}"
EMAIL="test_$(date +%s)@example.com"
PASSWORD="TestPassword123"

echo "=============================================="
echo "1. Health check"
echo "=============================================="
curl -s "$BASE_URL/api/health"
echo; echo

echo "=============================================="
echo "2. Register a test user"
echo "=============================================="
curl -s -X POST "$BASE_URL/api/v1/auth/register" \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"Test User\",\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\"}"
echo; echo

echo "=============================================="
echo "3. Log in and capture the JWT"
echo "=============================================="
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\"}")

echo "$LOGIN_RESPONSE"
echo

TOKEN=$(echo "$LOGIN_RESPONSE" | sed -n 's/.*"token":"\([^"]*\)".*/\1/p')

if [ -z "$TOKEN" ]; then
  echo "ERROR: Could not extract token from login response. Stopping here."
  exit 1
fi
echo "Captured token: ${TOKEN:0:20}... (truncated)"
echo

echo "=============================================="
echo "4. Call the new Directions proxy endpoint"
echo "=============================================="
curl -s -G "$BASE_URL/api/v1/maps/route" \
  -H "Authorization: Bearer $TOKEN" \
  --data-urlencode "origin_lat=25.4358" \
  --data-urlencode "origin_lng=81.8463" \
  --data-urlencode "dest_lat=25.4520" \
  --data-urlencode "dest_lng=81.8700"
echo; echo

echo "=============================================="
echo "5. Sanity check: same call WITHOUT a token (should be 401)"
echo "=============================================="
curl -s -G "$BASE_URL/api/v1/maps/route" \
  --data-urlencode "origin_lat=25.4358" \
  --data-urlencode "origin_lng=81.8463" \
  --data-urlencode "dest_lat=25.4520" \
  --data-urlencode "dest_lng=81.8700"
echo; echo

echo "Done."
