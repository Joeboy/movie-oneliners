#!/usr/bin/python
"""
Copy an image sequence to another directory, renaming the files such that the
first frame is 00001.png.
This is useful because Blender uses the frame number as the filename, but
ffmpeg fails if you try to encode an image sequence that doesn't start at
frame 1.
"""
import os
import re
from shutil import copyfile

def renumber_files(input_dir):
    filename_re = re.compile(r'^(\d{4,6})\.png$')

    all_files = os.listdir(input_dir)
    files = {}
    for f in all_files:
        m = filename_re.match(f)
        if m:
            index = int(m.groups(1)[0])
            if index in files.keys():
                raise Exception, "More than one file claiming to be frame %d" % index
            files[index] = f

    indices = sorted(files.keys())
    for i in range(indices[0], 1+indices[-1]):
        if i not in files.keys():
            raise Exception, "No file for frame %d" % i

    output_dir = os.path.join(input_dir, 'renumbered')
    if not os.path.isdir(output_dir):
        os.mkdir(output_dir)

    out_i = 1
    for i in sorted(files.keys()):
        copyfile(files[i], os.path.join(output_dir, "%05d.png" % out_i))
        out_i += 1

if __name__ == "__main__":
    renumber_files("./")
