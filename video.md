Make a 25 fps h.264 avi file out of sequentially numbered png files.

    ffmpeg -r 25 -i %04d.png -vcodec h264 -preset slow output.avi
