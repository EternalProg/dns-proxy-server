#!/bin/bash
set -e

SERVER="127.0.0.1"
PORT="${1:-1053}"  # default to 1053 if not set

BINARY="./build/release/dnsproxy"
CONFIG_FILE="config.yaml"
BACKUP_FILE="backup-config.yaml"
DOMAIN="youtube.com"

# Compile
echo "Building project..."
./build.sh release

# Backup old config
cp "$CONFIG_FILE" "$BACKUP_FILE"

# Kill previous instance if any
pkill -f dnsproxy || true

###########################
# TEST 1: NOT_FOUND
###########################
echo "[TEST 1] NOT_FOUND"

cat > "$CONFIG_FILE" <<EOF
---
upstream_dns: 1.1.1.1
blacklist_response: not_found
blacklist:
  - $DOMAIN
EOF

$BINARY &
PID=$!
sleep 1

dig @$SERVER -p $PORT $DOMAIN +short | grep -q '^$' && {
    echo "❌ Expected NOT_FOUND (empty response)"
    kill $PID
    exit 1
}

echo "✅ NOT_FOUND returned empty response"
kill $PID
echo
sleep 1
###########################
# TEST 2: REFUSED
###########################
echo "[TEST 2] REFUSED"

cat > "$CONFIG_FILE" <<EOF
---
upstream_dns: 1.1.1.1
blacklist_response: refused
blacklist:
  - $DOMAIN
EOF

$BINARY &
PID=$!

output=$(dig @$SERVER -p $PORT $DOMAIN)
echo "$output" | grep -q "status: REFUSED" || {
    echo "❌ Expected REFUSED"
    kill $PID
}

echo "✅ REFUSED returned correct response"
kill $PID
echo
sleep 1
###########################
# TEST 3: RESOLVE
###########################
echo "[TEST 3] RESOLVE"

cat > "$CONFIG_FILE" <<EOF
---
upstream_dns: 1.1.1.1
blacklist_response: resolve
resolve_ip: 0.0.0.0
blacklist:
  - $DOMAIN
EOF

$BINARY &
PID=$!

resolved_ip=$(dig @$SERVER -p $PORT $DOMAIN +short)
if [[ "$resolved_ip" == "0.0.0.0" ]]; then
    echo "✅ RESOLVE returned correct IP"
else
    echo "❌ RESOLVE failed. Got $resolved_ip"
    kill $PID
    exit 1
fi

kill $PID
sleep 1
echo
###########################
# STRESS TEST
###########################
TOTAL_REQUEST=300
HALF_REQUEST=$((TOTAL_REQUEST / 2))
echo "[STRESS TEST] Sending $TOTAL_REQUEST queries..."

cat > "$CONFIG_FILE" <<EOF
---
upstream_dns: 1.1.1.1
blacklist_response: resolve
resolve_ip: 0.0.0.0
blacklist:
  - stress.com
EOF

$BINARY &
PID=$!

START=$(date +%s.%N)

for i in $(seq 1 $HALF_REQUEST); do
    dig @$SERVER -p $PORT stress.com +short > /dev/null &
done

for i in $(seq 1 $HALF_REQUEST); do
    dig @$SERVER -p $PORT youtube.com +short > /dev/null &
done

kill $PID
wait

END=$(date +%s.%N)
DURATION=$(echo "$END - $START" | bc)

echo "✅ $TOTAL_REQUEST requests completed in $DURATION seconds"

rm "$CONFIG_FILE"
mv "$BACKUP_FILE" "$CONFIG_FILE"