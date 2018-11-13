#!/usr/bin/python3
# -*- encoding: utf-8 -*-

"""File image runs into subdirectories.

Reads images from a camera's memory card and files them into directories.  All
images with near enough timestamps get filed into the same directory.

Builds a sample.lp file containing a light position map for the ptm builder.

Usage:

  image_filer.py /mount/sdcard/dcim/*.jpeg

"""

import datetime
import glob
import math
import itertools
import operator
import os
import shutil
import sys

from PIL import Image
from PIL.ExifTags import TAGS, GPSTAGS

DOME_DIAMETER = 490.0 - (2 * 13.0)
RINGS = [ (4, 85.0), (8, 218.0), (16, 330.0), (16, 416.0), (16, 457.0) ]

twopi = 2.0 * math.pi
thres = datetime.timedelta (seconds = 5)


def get_datetime (path):
    datetimestr = Image.open (path)._getexif () [36867]
    # print (datetimestr) # 2018:11:11 22:38:29
    return datetime.datetime.strptime (datetimestr, "%Y:%m:%d %H:%M:%S")


def grouper (pair):
    if pair[1] is None:
        return False
    return abs (pair[0][1] - pair[1][1]) < thres


def pairwise (iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = itertools.tee (iterable)
    next (b, None)
    return itertools.zip_longest (a, b)


files = [ (fn, get_datetime (fn)) for fn in glob.glob (sys.argv[1]) ]
files.sort (key = operator.itemgetter (1)) # sort by time

grouped_files = []
last_group = []
for pair in pairwise (files):
    if not grouper (pair):
        last_group.append (pair[0])
        grouped_files.append (last_group)
        last_group = []
    else:
        last_group.append (pair[0])

for files in grouped_files:
    i = 0
    run = 'runs/' + files[0][1].strftime ('%Y-%m-%d-%H-%M-%S')
    print (run)
    os.makedirs (run, exist_ok = True)

    with open ("%s/sample.lp" % run, 'w') as fp:
        fp.write ("# run of %s\n" % files[0][1].isoformat ())
        fp.write ("%d\n" % len (files))
        for num_leds, diameter in RINGS:
            phi = 1.0 / 8.0 * twopi                 # start each ring at 45°
            step_phi = -twopi / num_leds
            dia = diameter / DOME_DIAMETER
            z = math.sqrt (1 - dia ** 2)

            for n in range (0, num_leds):
                filename = files[i][0]
                shutil.copy2 (filename, run)
                fp.write ("%s %f %f %f\n" % (os.path.basename (filename), dia * math.sin (phi), dia * math.cos (phi), z))
                phi += step_phi
                i += 1
