#!/bin/bash

source ./.env

PIPELINE_ID="pip_qpUgXycjWF6YMeSL"
DAYDREAM_API_KEY=${DAYDREAM_API_KEY}

response=$(curl -s -X POST \
    --url https://api.daydream.live/v1/streams \
    -H "Authorization: Bearer $DAYDREAM_API_KEY" \
    -H 'Content-Type: application/json' \
    -d "{\"pipeline_id\":\"${PIPELINE_ID}\"}" \
)
echo $response
