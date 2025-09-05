const fs = require('fs');

const express = require('express');
const ffmpeg = require('fluent-ffmpeg');

const app = express();

const PORT = 3000;
const STREAM_ID = 'str_f1Tqcgjbnv38Q8Qu';
const ENDPOINT_STREAM_STATUS = `https://daydream.live/api/streams/${STREAM_ID}/status`;

let rtmpUrl = null;

app.get('/', (req, res) => {
    res.send(`<img src="/mjpeg" style="width: 360px; height: 360px; border: 1px solid black;" />`);
});

app.get('/mjpeg', (req, res) => {
    console.log('Client connected to /mjpeg');

    res.writeHead(200, {
        'Content-Type': 'multipart/x-mixed-replace; boundary=frameboundary',
        'Cache-Control': 'no-cache',
        'Connection': 'keep-alive'
    });

    const ffmpegProcess = ffmpeg(rtmpUrl)
    .addInputOption('-re')
    .outputOptions([
        '-f mjpeg',
        '-q:v 3'
    ])
    .on('start', (commandLine) => {
        console.log('FFmpeg process started with command:', commandLine);
    })
    .on('error', (err) => {
        console.error('FFmpeg error:', err.message);
        res.end();
    })
    .on('end', () => {
        console.log('FFmpeg process ended');
        res.end();
    })
    .on('data', (chunk) => {
        console.log('Received data chunk of size:', chunk.length);
    })
    .pipe();

    let frameCount = 0;
    ffmpegProcess.stdout.on('data', (chunk) => {
        console.log('Received data chunk of size:', chunk.length);

        frameCount++;
        console.log(`Received JPEG frame #${frameCount}, size: ${chunk.length}`);

        res.write(`--frameboundary\r\nContent-Type: image/jpeg\r\nContent-Length: ${chunk.length}\r\n\r\n`);
        res.write(chunk);
    });

    ffmpegProcess.stderr.on('data', (data) => {
        console.error('FFmpeg stderr:', data.toString());
    });

    req.on('close', () => {
        console.log('Client disconnected, killing FFmpeg process.');
        ffmpegProcess.kill('SIGKILL');
    });
});

const streamStatus = async () => {
    const res = await fetch(ENDPOINT_STREAM_STATUS, { method: 'GET' });
    const statusData = await res.json();
    return statusData;
}

const convertWHEPToRTMP = async (whepUrl) => {
    return whepUrl
    .replace('https://', 'rtmp://') // strip https:// and replace / with rtmp://
    .replace('/whep', ''); // strip /whep suffix
}

const init = async () => {
    const status = await streamStatus();
    const whepUrl = status.data.gateway_status.whep_url;
    rtmpUrl = await convertWHEPToRTMP(whepUrl);
    return;
}

app.listen(PORT, async () => {
    await init();
    console.log(`MJPEG server running at http://localhost:${PORT}`);
    console.log(`View stream at http://localhost:${PORT}/mjpeg`);
    console.log(`RTMP URL: ${rtmpUrl}`);
});
