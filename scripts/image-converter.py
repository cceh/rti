#!/usr/bin/python3
# -*- encoding: utf-8 -*-

"""This script is a wrapper that can pass image files through a converter before
calling the :ref:`ptm-encoder <ptm-encoder>`.

With the help of this script you may crop / resize / rotate your images before
processing them with the ptm-encoder.  Temporary files are created for the
encoder, the original files are kept intact.

Write a shell script and :command:`chmod +x` it.  The shell script must process
the source image, apply all wanted transformations and save the destination
image.  The source image path and filename will be $1 and the destination path
and filename will be $2.  The destination will point to a temp directory.

An example of a shell script follows:

.. code-block:: shell

   #!/bin/bash
   convert "$1" -crop 2800x2800+2236+1064 -resize 2000x2000 "$2"

An example of a command to build one PTM is then:

.. code-block:: console

   scripts/image-converter.py -f runs/a/convert.sh \\
      bin/ptm-encoder -f PTM_FORMAT_JPEG_LRGB -o a.ptm runs/a/sample.lp

Alternatively you can do without the shell script and put the contents of the
script in the command line.  The placeholders "{source}" and "{dest}" will be
replaced by the source and destination filenames.

The command to build one PTM is then:

.. code-block:: console

   scripts/image-converter.py -e 'convert "{source}" -crop 2800x2800+2236+1064 -resize 2000x2000 "{dest}"' \\
      bin/ptm-encoder -f PTM_FORMAT_JPEG_LRGB -o a.ptm runs/a/sample.lp

"""

import argparse
import datetime
import math
import itertools
import operator
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
import textwrap



def build_parser ():
    parser = argparse.ArgumentParser (
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent (__doc__)
    )

    parser.add_argument ('-v', '--verbose', dest='verbose', action='count',
                         help='increase output verbosity', default=0)

    group = parser.add_mutually_exclusive_group (required=True)
    group.add_argument ('-e', '--exec', metavar='"COMMANDLINE"', type=str,
                         help='execute COMMANDLINE with every image')
    group.add_argument ('-f', '--file', metavar='SCRIPT-FILE', type=str,
                         help='execute SCRIPT-FILE with every image')

    parser.add_argument ('encoder', metavar="ENCODER",
                         help='the PTM encoder executable')
    parser.add_argument ('encoder_args', nargs=argparse.REMAINDER, metavar="...",
                         help='arguments to the PTM encoder')

    return parser


class light (object):
    def __init__ (self, s, source_dir, dest_dir):
        self.filename, x, y, z = s.split ()
        self.basename = os.path.basename (self.filename)
        self.source = os.path.join (source_dir, self.basename)
        self.dest   = os.path.join (dest_dir,   self.basename)
        self.x = float (x)
        self.y = float (y)
        self.z = float (z)

    def __str__ (self):
        return "%s %f %f %f" % (self.basename, self.x, self.y, self.z)


if __name__ == '__main__':

    parser = build_parser ()
    args = parser.parse_args ()

    lights_fn  = args.encoder_args[-1]
    source_dir = os.path.dirname (lights_fn)
    dest_dir   = tempfile.mkdtemp ()

    if args.file:
        args.exec = "%s {source} {dest}" % args.file

    lights = []
    with open (lights_fn, 'r') as fp_lights:
        for line in fp_lights:
            try:
                lights.append (light (line, source_dir, dest_dir))
            except ValueError:
                pass

    lights_dest_fn = os.path.join (dest_dir, os.path.basename (lights_fn))

    with open (lights_dest_fn, 'w') as fp_lights:
        fp_lights.write (str (len (lights)) + "\n")
        for light in lights:
            fp_lights.write (str (light) + "\n")

    processes = []
    for light in lights:
        arg = shlex.split (args.exec.format (source=light.source, dest=light.dest))
        print (' '.join (arg))
        processes.append (subprocess.Popen (arg))

    for process in processes:
        process.wait ()

    args.encoder_args[-1] = lights_dest_fn

    arg = [args.encoder] + shlex.split (' '.join (args.encoder_args))
    print (' '.join (arg))
    subprocess.call (arg)

    shutil.rmtree (dest_dir)
