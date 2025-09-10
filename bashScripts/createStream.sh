#!/bin/bash

source ../.env

PIPELINE_ID="pip_qpUgXycjWF6YMeSL"
DAYDREAM_API_KEY=${DAYDREAM_API_KEY}
DAYDREAM_STREAM_URL="https://lvpr.tv/?v="

HIGHLIGHT='\033[0;32m'
NC='\033[0m'

response=$(curl -s -X POST \
  "https://api.daydream.live/v1/streams" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer ${DAYDREAM_API_KEY}" \
  -d "{\"pipeline_id\":\"${PIPELINE_ID}\"}")

id=$(echo "$response" | grep -o '"id":"[^"]*"')
output_stream_url=$(echo "$response" | grep -o '"output_stream_url":"[^"]*"' | sed 's/"output_stream_url":"//;s/"//')
output_playback_id=$(echo "$response" | grep -o '"output_playback_id":"[^"]*"' | sed 's/"output_playback_id":"//;s/"//')
whip_url=$(echo "$response" | grep -o '"whip_url":"[^"]*"' | sed 's/"whip_url":"//;s/"//')

echo -e "ID: ${HIGHLIGHT}$id${NC}"
echo -e "Output Playback ID: ${HIGHLIGHT}$output_playback_id${NC}"
echo -e "Output Playback URL: ${HIGHLIGHT}${DAYDREAM_STREAM_URL}$output_playback_id${NC}"
echo -e "Output Stream URL: ${HIGHLIGHT}$output_stream_url${NC}"
echo -e "WHIP URL: ${HIGHLIGHT}$whip_url${NC}"
# echo "Full Response: $response"