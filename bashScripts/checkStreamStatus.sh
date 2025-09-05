#!/bin/bash

source ./.env

STREAM_ID=${1:-"stream-id"}

response=$(curl -sL https://daydream.live/api/streams/${STREAM_ID}/status)

echo $response