#!/bin/bash

source ../.env

PIPELINE_ID="pip_qpUgXycjWF6YMeSL"
STREAM_ID=${1:-"stream-id"}
DAYDREAM_API_KEY=${DAYDREAM_API_KEY}

response=$(curl -s -X POST \
    "https://api.daydream.live/beta/streams/${STREAM_ID}/prompts" \
    -H "Authorization: Bearer ${DAYDREAM_API_KEY}" \
    -H "Content-Type: application/json" \
    -d '{
            "pipeline":"live-video-to-video",
            "model_id":"streamdiffusion",
            "params":{
                "model_id":"stabilityai/sd-turbo",
                "prompt":"crystal, rainbow refractions, laser line highlights, ice cubes with diamond reflections, vector art, synthwave, vaporwave, 3d render, octane render, unreal engine, cinematic lighting, high detail, 8k",
                "negative_prompt":""
            }
        }' \
)

echo $response

