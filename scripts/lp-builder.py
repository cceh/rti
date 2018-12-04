#!/usr/bin/python3
# -*- encoding: utf-8 -*-

"""Build the light position file for Dome 1.

Outputs to stdout a skeleton sample.lp file with light positions according to
the Ø 50cm prototype dome at CCeH.

This script is hard-coded to one specific dome but it may serve as example for
writing a more general script for other domes.

"""

import argparse
import math
import sys

DOME_DIAMETER = 490.0 - (2 * 13.0)
RINGS = [ (4, 85.0), (8, 218.0), (16, 330.0), (16, 416.0), (16, 457.0) ]

twopi = 2.0 * math.pi

def build_parser ():
    parser = argparse.ArgumentParser (description = __doc__)

    parser.add_argument ('-a', '--angle', metavar='ANGLE', type=float, default=0.0,
                         help='the rotation angle of the camera on the Dome (default 0°)')
    parser.add_argument ('-v', '--verbose', dest='verbose', action='count',
                         help='increase output verbosity', default=0)
    return parser


if __name__ == '__main__':

    parser = build_parser ()
    args = parser.parse_args ()

    start_phi = twopi * 45.0 / 360.0          # start each ring at 45°
    user_phi  = twopi * args.angle / 360.0    # rotated camera

    # num_leds = sum ([x[0] for x in RINGS])
    # sys.stdout.write ("%d\n" % num_leds)

    for num_leds, diameter in RINGS:
        phi = start_phi + user_phi
        step_phi = -twopi / num_leds
        dia = diameter / DOME_DIAMETER
        z = math.sqrt (1 - dia ** 2)

        for n in range (0, num_leds):
            sys.stdout.write ("%f %f %f\n" % (dia * math.sin (phi), dia * math.cos (phi), z))
            phi += step_phi
