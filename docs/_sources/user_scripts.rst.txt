=========
 Scripts
=========


.. _image-filer.py:

.. autoprogram:: scripts.image-filer:build_parser()
   :prog: scripts/image-filer.py


.. _ptm-encoder:

bin/ptm-encoder
===============

.. program:: ptm-encoder

Encode a series of JPEG files into one PTM file.

Reads a :file:`sample.lp` file with a list of the JPEG files to encode.  You can
build a :file:`sample.lp` file using the :ref:`image filer <image-filer.py>`
script.

.. code-block:: shell

   usage: ptm-encoder [OPTION...] sample.lp

.. option:: -f, --format

   Which PTM format to output (default: PTM_FORMAT_RGB).

.. option:: -l, --list

   Output a list of supported PTM formats, eg:

   - PTM_FORMAT_RGB,
   - PTM_FORMAT_LUM,
   - PTM_FORMAT_LRGB,
   - PTM_FORMAT_JPEG_RGB,
   - PTM_FORMAT_JPEG_LRGB.

.. option:: -o, --output=<FILE>

   Output to FILE instead of STDOUT.

.. option:: -v, --verbose

   Produce verbose output.

.. option:: -?, --help

   Give this help list.

.. option:: --usage

   Give a short usage message.

.. option:: -V, --version

   Print program version.


.. _ptm-decoder:

bin/ptm-decoder
===============

.. program:: ptm-decoder

Extract one JPEG out of a PTM file.

The output goes to stdout, so the program is easy to use in a web server.

.. code-block:: shell

   usage: ptm-decoder filename.ptm [u v] > filename.jpg

.. option:: u v

   Set the light position for the JPEG.  Defaults to 0.5 and 0.5, that is,
   lighted from top right.  See: :ref:`sample.lp <sample.lp>`.


.. _ptm-exploder:

bin/ptm-exploder
================

.. program:: ptm-exploder

Explode one PTM into multiple JPEGs lighted from different angles.

.. code-block:: shell

   usage: ptm-exploder filename.ptm sample.lp filename.jpeg

The :file:`sample.lp` file should be of the same :ref:`format <sample.lp>` used
by the :ref:`PTM encoder script<ptm-encoder>`, although the filename part is not
used by the exploder.  Instead the filename to use is provided in the 3rd
argument and is changed into into filename-NNN.jpeg for each image.


.. _sample.lp:

sample.lp file format
=====================

The :file:`sample.lp` file contains a light position map that relates picture
files to lighting positions.  It is needed by the :ref:`PTM encoder
<ptm-encoder>` program.

The format of this file is:

.. code-block:: none

   filename01 u01 v01 w01
   filename02 u02 v02 w02
   filename03 u03 v03 w03
   ...
   filenameNN uNN vNN wNN

The filenames are relative to the :file:`sample.lp` file.  :math:`u`, :math:`v`
and :math:`w` are the cartesian coordinates of the light source from the center
of the dome and lie on the unity sphere, ie. the radius of the dome is defined
as 1.  Every pair of :math:`u` and :math:`v` values must obey :math:`u^2 + v^2
\le 1`. :math:`w` may be specified, but is not used since it is redundant.
