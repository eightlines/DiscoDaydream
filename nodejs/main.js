const express = require('express');
const ffmpeg = require('fluent-ffmpeg');

const app = express();
const port = 3000;
const streamUrl = 'rtmp://rtmp.livepeer.com/live/1d10-pzad-jh0a-name'; // RTMP URL

const BOUNDARY = 'frameboundary';
const HEADERS = {
    'Content-Type': `multipart/x-mixed-replace; boundary=${BOUNDARY}`,
    'Cache-Control': 'no-cache',
    'Connection': 'keep-alive',
};

app.get('/mjpeg', (req, res) => {
    console.log('Client connected for MJPEG stream');
    res.writeHead(200, HEADERS);

    const ffmpegProcess = ffmpeg(streamUrl)
    .addInputOption('-re') // Read input at native frame rate
    .outputOptions([
        '-f image2pipe',
        '-vcodec mjpeg',
        '-q:v 3', // Quality level (1-31, lower is better)
        '-update 1', // Update the image only when there is a change
        '-r 10', // Frame rate
    ])
    .on('start', (commandLine) => {
        console.log('FFmpeg process started with command:', commandLine);
    })
    .on('error', (err) => {
        console.error('FFmpeg error:', err.message);
        console.error('FFmpeg stdout:', err.stdout);
        console.error('FFmpeg stderr:', err.stderr);

        if (!res.headersSent) {
            res.status(500).send('FFmpeg Stream Error');
        }
        
        res.destroy();
    })
    .on('end', () => {
        console.log('FFmpeg process ended');
        
        if (!res.headersSent) {
            res.status(500).send('FFmpeg Process Finished');
        }

        res.destroy();
    })
    .pipe({ end: false });
    
    req.on('close', () => {
        console.log('Client disconnected from MJPEG stream');
        ffmpegProcess.kill('SIGKILL');
    });
});

app.get('/', (req, res) => {
    res.send(`<h1>MJPEG Stream</h1>
        <p>View the stream here:</p>
        <img src="/mjpeg" style="width: 360px; height: 360px; border: 1px solid black;" />
    `);
});

app.listen(port, () => {
    console.log(`MJPEG server running at http://localhost:${port}`);
    console.log(`View stream at http://localhost:${port}/mjpeg`);
});