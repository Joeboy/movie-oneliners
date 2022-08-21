Make a 25 fps h.264 avi file out of sequentially numbered png files.

    ffmpeg -r 25 -i %04d.png -vcodec h264 -preset slow output.avi

Make a prores file out of sequentially numbered png files. Not sure if these are the best settings?

    ffmpeg -i %04d.png -c:v prores_ks -qscale:v 10 -vendor ap10 -profile:v 3 -r 25 output.mov

Cleanup a video file and output it to a lossless h264 file.

    ffmpeg -i INPUT.MOV -c:v libx264 -preset veryslow -qp 0 -vf hqdn3d=0:0:5 cleaned.avi

Combine a video file with an audio file.

    ffmpeg -i audio.wav -i video.avi -map 0:0 -map 1:0 -acodec copy -vcodec copy -shortest output.avi

Take a video file with a weird audio format and convert the audio to PCM

ffmpeg -i video.webm -c:v copy -c:a pcm_s16le output.mkv
