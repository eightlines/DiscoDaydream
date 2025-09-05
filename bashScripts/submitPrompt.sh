#!/bin/bash

source ./.env

PIPELINE_ID="pip_qpUgXycjWF6YMeSL"
STREAM_ID=${1:-"stream-id"}
DAYDREAM_API_KEY=${DAYDREAM_API_KEY}

response=$(curl -s -X POST \
  --url https://api.daydream.live/beta/streams/${STREAM_ID}/prompts \
  --header "Authorization: Bearer ${DAYDREAM_API_KEY}" \
  --header "Content-Type: application/json" \
  --data '{
  "pipeline": "live-video-to-video",
  "model_id": "streamdiffusion",
  "params": {
    "model_id": "stabilityai/sd-turbo",
    "prompt": "race car, drifting, smoke, detailed, cinematic lighting",
    "prompt_interpolation_method": "linear",
    "normalize_prompt_weights": true,
    "normalize_seed_weights": true,
    "negative_prompt": "person",
    "guidance_scale": 100,
    "delta": 123,
    "num_inference_steps": 0,
    "t_index_list": [
      0
    ],
    "width": 0,
    "height": 0,
    "lora_dict": {},
    "use_lcm_lora": true,
    "lcm_lora_id": "<string>",
    "acceleration": "<string>",
    "use_denoising_batch": true,
    "do_add_noise": true,
    "seed": 0,
    "seed_interpolation_method": "linear",
    "enable_similar_image_filter": true,
    "similar_image_filter_threshold": 123,
    "similar_image_filter_max_skip_frame": 0,
    "controlnets": [
        {
            "conditioning_scale": 100,
            "control_guidance_end": 1,
            "control_guidance_start": 0,
            "enabled": true,
            "model_id": "lllyasviel/sd-controlnet-openpose",
            "preprocessor": "openpose"
        },
        {
            "conditioning_scale": 39,
            "control_guidance_end": 1,
            "control_guidance_start": 0,
            "enabled": true,
            "model_id": "lllyasviel/sd-controlnet-hed",
            "preprocessor": "hed"
        },
        {
            "conditioning_scale": 29,
            "control_guidance_end": 1,
            "control_guidance_start": 0,
            "enabled": true,
            "model_id": "lllyasviel/sd-controlnet-canny",
            "preprocessor": "canny"
        },
        {
            "conditioning_scale": 0,
            "control_guidance_end": 1,
            "control_guidance_start": 0,
            "enabled": true,
            "model_id": "lllyasviel/sd-controlnet-depth",
            "preprocessor": "depth"
        }
    ],
    "ip_adapter": {
      "scale": 123,
      "enabled": true
    },
    "ip_adapter_style_image_url": "<string>",
    "weight_type": "linear"
  }
}'
)

echo $response
