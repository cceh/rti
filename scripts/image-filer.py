#!/usr/bin/python3
# -*- encoding: utf-8 -*-

"""Reads images from a camera's memory card or other flat directory structure,
finds runs of images and files them into subdirectories.  A run is found by the
:term:`EXIF` timestamp in the image files.

The script also builds a full :ref:`light position map <sample.lp>`
:file:`sample.lp` from a light position skeleton file and files it alongside the
images files.

Some cameras put a maximum of 1000 pictures into a folder.  After that they
automatically open a new folder.  If this happened during a shooting, include
both directories on the commandline.  The script will sort it out for you.

"""

import argparse
import datetime
import math
import itertools
import operator
import os
import shutil
import sys
import textwrap

from PIL import Image
from PIL.ExifTags import TAGS, GPSTAGS

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


def build_parser ():
    parser = argparse.ArgumentParser (
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent (__doc__)
    )

    parser.add_argument ('-v', '--verbose', dest='verbose', action='count',
                         help='increase output verbosity', default=0)
    parser.add_argument ('-l', '--lights', metavar='LP', type=str, default='skeleton.lp',
                         help='the skeleton light positions file (default: skeleton.lp)')

    parser.add_argument ('files', metavar='FILES', type=str, nargs='+',
                         help='the image files to file (wildcards supported)')
    return parser


if __name__ == '__main__':

    parser = build_parser ()
    args = parser.parse_args ()

    # read lights file
    lights = []
    with open (args.lights, 'r') as fp_lights:
        for line in fp_lights:
            x, y, z = line.split ()
            lights.append ( (float (x), float (y), float (z)) )

    # get timestamps from images
    files = [ (fn, get_datetime (fn)) for fn in args.files ]
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
        run = 'runs/' + files[0][1].strftime ('%Y-%m-%d-%H-%M-%S')
        os.makedirs (run, exist_ok = True)

        if len (files) < len (lights):
            print ("error: not enough files in %s\n" % run)
            break
        if len (files) > len (lights):
            print ("error: too many files in %s\n" % run)
            break

        if args.verbose:
            print ("filing into %s" % run)

        with open ("%s/sample.lp" % run, 'w') as fp:
            fp.write ("%d\n" % len (files))
            for l, f in zip (lights, files):
                filename = f[0]
                x, y, z = l
                shutil.copy2 (filename, run)
                fp.write ("%s %f %f %f\n" % (os.path.basename (filename), x, y, z))
                if args.verbose:
                    print (".", end = '')

        if args.verbose:
            print ("")
