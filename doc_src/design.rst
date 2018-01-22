Electrical Design
=================

The main goals of this design are:

- bright and affordable LEDs,

- user-solderable off-the-shelf components.

.. figure:: ../images/DSC_2827.jpg

   The front panel.

.. figure:: ../images/DSC_2829.jpg

   The back panel.

.. figure:: ../images/DSC_2842.jpg

   The mounted PCB.



LEDs
----

Consider following points before selecting the LED make.

Performance:

  Luminous LEDs allow faster work.  Don't consider LEDs with less than 100 lm
  (lumen) or without the lumens specified.

Light quality:

  Especially important if you want to take color pictures.

Price:

  High performance LEDs are (still) pretty expensive.

Mounting:

  High performance LEDs are designed for reflow soldering, which makes them hard
  to solder by hand.  Before use you must solder each LED to a small board or
  buy them pre-soldered (expensive).  Make sure you are able to solder the LED
  type before you buy lots of them.


For the prototype we selected LEDs of the make:

*Osram Duris S8 GW P9LMS1.EM-NSNU-57S5-0*

.. _P9LMS1.EM-NSNU-57S5-0: http://www.osram-os.com/Graphics/XPic8/00199270_0.pdf

They come in 5 × 5 mm packages, which we hand-soldered onto a standard 2.54"
striped PCB and then cut the board into little pieces.


Alternative LEDs
----------------

LED alternatives, since the original type might be discontinued.

========== ========================== ====  ==== === ==== ==== ====== ======= =============
Series     Part.No.                     lm     V  mA Case    € MinQty Distr.  No
========== ========================== ====  ==== === ==== ==== ====== ======= =============
Duris S 10 GW P7LP32.EM-RSRU-XX52-1   1400  38.0 300 7070 1.90     50 RS      8792889
Duris S 10 GW P7LM32.EM-QURQ-XX52-1   1050  28.5 300 7070 1.72     50 Mouser
Duris S 8  GW P9LT31.EM-PSPU-XX52-1    610  31.0 150 5050 0.96     25 DigiKey 475-3200-1-ND
Duris S 8  GW P9LT32.EM-PSPU-XX52-1    610   6.2 750 5050 0.96     25 DigiKey 475-3207-1-ND
Duris S 8  GW P9LR31.EM-PQPS-XX52-1    500  24.8 150 5050 0.76     25 DigiKey 475-3187-1-ND
Duris S 8  GW P9LR32.EM-PQPS-XX52-1    500   6.2 600 5050 0.80     25 DigiKey 475-3193-1-ND
Duris S 8  GW P9LMS1.EM-NSNU-57S5-0    395  19.8 200 4SMD 0.91     50 RS      8108054
Duris S 8  GW P9LMS2.EM-NQNS-57S5-0    350  19.8 200 4SMD 0.60     50 RS      8768969
========== ========================== ====  ==== === ==== ==== ====== ======= =============


Led Driver
----------

The LED driver section is very flexible.  Adjusting component values you can
drive almost any LED up to 1.5A / 80V.  You'll have to use separate power
supplies for VLED and VCPU if VLED is over 35V, which is the max. the 7805 can
handle.  Otherwise you can just tie VCPU to VLED.  The components you have to
adjust are the resistor of the constant current source and the base resistors of
the high-side BD140's.

The chosen LEDs have a forward voltage of:

=== ====
V_f    V
=== ====
min 18.6
typ 19.8
max 22.2
=== ====

@ a forward current of 200mA.

Cold LEDs have a higher forward voltage.  Ours are turned on for short periods
only, so they will be cold.

::

   LM 317 Drop-Out Voltage  (@ I_O = 200mA, T_j = 25°C)       =  1.65V
   LM 317 V_adjust                                            =  1.25V
   BD 140 C-E Saturation Voltage (@ I_C = -0.5A, I_B = -50mA) = -0.5V

This gives us a drop of at least 3.9V, ergo, the power supply should be at least
V_f max + 3.9V = 26.1V.


Constant Current Source
-----------------------

To get even luminosity I use an LM 317 as constant current source.  The
adjustment resistor value is given by::

  R_adj = 1.25V / I_O
  R_adj = 1.25V / 200mA = 6.25ohm, 0.25W

The nearest standard value is 6.2ohm E24 (or 4.7ohm + 1.5ohm E12).

N\.B. the constant current source also drives the bases of the high-side BD140
transistors, which sink 10mA with the chosen resistors (but would need 40mA of
base current to switch 1.5A LEDs).


Microcontroller
---------------

The project uses an ATmega328p microcontroller because it was prototyped on an
Arduino Nano.


Board Layout
------------

The board is layed out as 1/2 Eurocard (100 × 80 mm).

The back of the board holds a few standard connectors and is designed to be
flush against the back panel.  You may choose not to mount the connectors to the
board, in which case you may select the connectors you want and wire them to the
board.


Case
----

It is highly recommended to put the board into a case.  The make of the case is
up to you.

We used a case made of 2 *Fischer Elektronik KO H 2* halves, which offers room
for a 100 × 100 mm PCB.  The CAD drawings of front and back panel where printed on
paper and then spotted through with a scriber, drilled and filed to shape.
