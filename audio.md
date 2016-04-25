Audio commands
==============

Split a stereo wav file into left and right channels. Useful because some
recorders seem to think you want your audio in an arbitrary stereo
configuration. Eg the H4 always records stereo even if you've only got one mic
plugged into it.

    ffmpeg -i stereo.wav -codec pcm_s24le -map_channel 0.0.0 left.wav -codec pcm_s24le -map_channel 0.0.1 right.wav

Normalize an audio file, because you suck at recording sound:

    normalize-audio myfile.wav
