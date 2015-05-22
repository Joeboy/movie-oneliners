Make a 25 fps h.264 avi file out of sequentially numbered png files.

    ffmpeg -r 25 -i %04d.png -vcodec h264 -preset slow output.avi

Cleanup a video file and output it to a lossless h264 file.

    ffmpeg -i INPUT.MOV -c:v libx264 -preset veryslow -qp 0 -vf hqdn3d=0:0:5 cleaned.avi
