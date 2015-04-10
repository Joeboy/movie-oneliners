Audio commands
==============

Split a stereo wav file into left and right channels. I use this because my
Zoom H4 creates stereo files even if I only have one mic plugged into it.

    ffmpeg -i stereo.wav -map_channel 0.0.0 left.wav -map_channel 0.0.1 right.wav

Normalize an audio file, because you suck at recording sound:

    normalize-audio myfile.wav
