# Disc-o Daydream

Initial research into the Daydream API and my attempt to get it running as a stream on an ESP32 in LVGL. 

### Concept

Since Daydream exports as a lower resolution video stream I went looking for a low resolution screen. There are a number of small circular screens that are capable of running on microcontrollers. I figured it could be possible to use this as a desktop dial to alter a Daydream stream or with the removal of some weight as a pendant necklace or pin. The key was getting the Daydream stream into a format that was capable of running on an ESP32. 

### Attempts

Several attempts were made, and I'll detail them below:

- LVGL Running FFMPEG - The examples provided by Waveshare were using LVGL v8.3, so my attempts followed that version. The LVGL built for the Waveshare knob seems to include a paired down LVGL, so when activating FFMPEG in `lv_conf.h` all you receive is header errors. 
- LVGL v9 - Moving to a full update of LVGL resulted in the constant rebooting of the microcontroller. I still prefer this method, just need to figure out what configuration setting is incorrect. 
- LVGL displaying MJPEG - Attempted to use Split JPG, and TJPGD to interpret the MJPEG file. This displayed a still image JPG but not a MJPEG. Also attempted to reload the JPG on a periodic basis as an attempt to brute force the video stream. Clunky. 
- LVGL displaying a JPG converted to `.c` format. Worked, wasn't animated. 
- LVGL displaying a prerecorded animation. Worked. Wasn't a live stream. 

## Bash Scripts

The folder `./bashScripts` contains some initial testing scripts to test the Daydream API. 

1. Copy ./.env.template to ./.env
2. Enter your Daydream API key into the `DAYDREAM_API_KEY=""` field
3. Run the following Bash scripts by entering `bash ./bashScripts/< Script Name > < Additional Params >`

- `createStream.sh` - Creates a Daydream stream, copy the generated fields to apply to later steps. (Note: During testing the Output Stream URL was found to be incorrect, this may be updated by the Daydream team time you see this note)
- `checkStreamStatus.sh` - Param: `< ID >` - This will output the JSON response which contains the `whep_url` which will be used in a later step to create the RTMP URL. 
- `submitPrompt.sh` - Param : `< ID >` - Also reads the .env parameters. This prompt details the larger prompt, but I didn't have any luck making this work. I opted for the `submitPromptShort.sh` to push the prompt updates. 
- `submitPromptShort.sh` - Param: `< ID >` - Additionally reads the .env parameters. Edit this file to change the prompt. 

## TouchDesigner Setup

TouchDesigner in this repo is only used to deliver a source video for the WHIP stream. It may be replaced by any framework that can output a 512x512 video. The application in ./simpleOutput creates a geometry on a black and white background and outputs to a Syphon/Spout stream. 

## OBS Setup

Communication from TouchDesigner to OBS is handled through the Spout (Windows) plugin. (Syphon is available for the MacOS platform) Alternatively, the NDI Op and Addon for OBS will work. 

1. Settings > Video > Base (Canvas) Resolution > _512x512_
2. Settings > Stream > Service > _[WHIP]_ > Server > _[Add WHIP Server from ./bashScripts/createStream.sh]_
3. Install the [Spout](https://github.com/Off-World-Live/obs-spout2-plugin) plugin for OBS
4. Restart OBS after installing the Spout addon
5. Sources > Spout2 Capture > Use first available sender (DaydreamVideoSource)
6. Start Streaming

## Daydream Video Stream

Verify the video stream is broadcasting by going to the LivePeer endpoint obtained by the _createStream.sh_ bash script. Append &lowLatency=force for faster streaming. 

- `https://www.lvpr.tv/?v=< Output Playback ID >&lowLatency=force`

## Server Setup

The Server is a NodeJS application which uses FFMpeg to convert the RTMP stream to an MJPEG stream. The intent of this is to get the RTMP stream which is broadcasting as an FLV VP9 codec into a format that LVGL can interpret. The application as it stands largely doesn't operate as intended, and the code in its current state is a mess. 

1. `cd server`
2. Copy the `.env.template` to `.env`
3. Fill in the values in `.env`
4. `npm i` (Built on NodeJS v20)
5. `npm start`

Main.js offers a few endpoints, it will attempt to display the .mjpeg file, or it will host a test JPG for loading by LVGL. Largely non-functional. 

## FFMPEG Setup

After struggling with the Node application I reverted to just creating the FFMPEG stream using the command line. To interpret the VP9 codec, you will require the most recent version (FFMPEG v8). 

1. Install FFMPEG ([Windows](https://www.gyan.dev/ffmpeg/builds/))
2. Add FFMPEG to environment variables. `ffmpeg` on the command line to test it is working. 
3. Issue the command `bash ./bashScripts/checkStreamStatus.sh < ID >` to grab the `whep_url`
4. To obtain the correct RTMP endpoint: In a notepad remove the `rtmp://` and replace with `http://`. Remove the trailing `/whep`
    - I created some code in the NodeJS application to handle this more efficiently: 
    ``` 
    const convertWHEPToRTMP = async (whepUrl) => {
        return whepUrl
            .replace('https://', 'rtmp://')
            .replace('/whep', ''); 
    }
    ```
5. Issue the following command to generate an MJPEG conversion:
    ```
    ffmpeg -y -i "< RTMP URL >" -c:v mjpeg -r 2 -f mjpeg output.mjpeg
    ```
6. The NodeJS Server will host the `output.mjpeg` file at `/output`

## LVGL Setup

Finally, I'm including the LVGL code running in Arduino that I attempted to read the MJPEG file with. This largely doesn't work, but I tried many different ways to get this up and running. I suspect there is a way to get this working. 

In this example I am using LVGL 8.3. I am working on getting LVGL v9.4 up and running, but the ESP32-S3 kept rebooting due to a setting discrepancy. 

### Hardware

The hardware I'm using for this project is a Waveshare Knob, but most ESP32-S3 models should be capable of running LVGL with enough memory to display a 360x360px image file. This particular model includes a nice CNC rotator mounted on a bearing which could be used to change the Daydream prompt settings, touchscreen, headphone and speaker input/output (unused in this project), and battery. The weight is significant, I wouldn't use this as a pendant necklace. 

Alternative round screens are listed below, without additional hardware they could be used as pins or necklaces and they could stream their content from a local server with Wifi capabilities. 

- [Waveshare ESP32-S3-Knob-Touch-LCD-1.8](https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8) (~$40)
- [LilyGo T-RGB](https://github.com/Xinyuan-LilyGO/LilyGo-T-RGB?spm=a2g0o.detail.1000023.12.2e2d602evBfKih) (~$30)
- [Waveshare 4" Round Display](https://www.waveshare.com/4inch-dsi-lcd-c.htm) (~$90) - Note: this is an HDMI screen and does not include a microcontroller

# Addendum: 

## NDI Setup

I also tried an additional setup method using NDI to broadcast from an iPad running a drawing application (or any content from the iPad). This method details this setup.

- PC
    - Install [OBS](https://obsproject.com/download)
    - Install [DistroAV](https://github.com/DistroAV/DistroAV)
- OBS
    - Setup WHIP URL in OBS > Settings > Stream (Dismiss Window)
    - Start Stream
- Browser
    - Launch Daydream URL 
- iPad 
    - Install [NDICapture](https://ndi.video/tools/hx-capture/) (Requires $ for > 1min of broadcasting)
    - Start NDI Broadcast
    - Load drawing application
    - Draw image
