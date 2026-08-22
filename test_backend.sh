#!/usr/bin/env bash
#
# Manual backend test script — no frontend required.
# Walks through: health check -> register -> login -> get JWT ->
# call the new /api/v1/maps/route endpoint.
#
# Usage:
#   chmod +x test_backend.sh
#   BASE_URL=http://localhost:8080 ./test_backend.sh
#
# Requires: curl, jq (for pretty JSON + extracting the token).
# On most systems: sudo apt install jq   /   brew install jq

set -euo pipefail

BASE_URL="${BASE_URL:-http://localhost:8080}"
EMAIL="test_$(date +%s)@example.com"   # unique each run, so register never 409s
PASSWORD="TestPassword123"

echo "=============================================="
echo "1. Health check"
echo "=============================================="
curl -s "$BASE_URL/api/health" | jq .
echo

echo "=============================================="
echo "2. Register a test user (self-registers as PASSENGER)"
echo "=============================================="
curl -s -X POST "$BASE_URL/api/v1/auth/register" \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"Test User\",\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\"}" | jq .
echo

echo "=============================================="
echo "3. Log in and capture the JWT"
echo "=============================================="
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"$EMAIL\",\"password\":\"$PASSWORD\"}")

echo "$LOGIN_RESPONSE" | jq .
TOKEN=$(echo "$LOGIN_RESPONSE" | jq -r .token)

if [ "$TOKEN" == "null" ] || [ -z "$TOKEN" ]; then
  echo "ERROR: Login did not return a token. Stopping here."
  exit 1
fi
echo
echo "Captured token: ${TOKEN:0:20}... (truncated)"
echo

echo "=============================================="
echo "4. Call the new Directions proxy endpoint"
echo "   (two arbitrary points a few km apart, as an example)"
echo "=============================================="
curl -s -G "$BASE_URL/api/v1/maps/route" \
  -H "Authorization: Bearer $TOKEN" \
  --data-urlencode "origin_lat=25.4358" \
  --data-urlencode "origin_lng=81.8463" \
  --data-urlencode "dest_lat=25.4520" \
  --data-urlencode "dest_lng=81.8700" | jq .
echo

echo "=============================================="
echo "5. Sanity check: same call WITHOUT a token"
echo "   (should be 401 — confirms JwtAuthFilter is actually protecting it)"
echo "=============================================="
curl -s -G "$BASE_URL/api/v1/maps/route" \
  --data-urlencode "origin_lat=25.4358" \
  --data-urlencode "origin_lng=81.8463" \
  --data-urlencode "dest_lat=25.4520" \
  --data-urlencode "dest_lng=81.8700" | jq .
echo

echo "Done."
