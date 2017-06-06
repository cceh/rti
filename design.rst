Electrical Design
=================

The main goals of this design are:

- bright and affordable LEDs,

- user-solderable off-the-shelf components.

LEDs
----

High performance LEDs are designed for surface mounting, which makes them
difficult to solder by traditional means.  Most of them are also very small.
For the prototype we used LEDs of the make:

*Osram Duris S8 GW P9LMS1.EM-NSNU-57S5-0_*

.. _P9LMS1.EM-NSNU-57S5-0: http://www.osram-os.com/Graphics/XPic8/00199270_0.pdf

They come in 5*5 mm packages, which you should be able to hand-solder onto a
standard 2.54" grid PCB.


Alternative LEDs
----------------

LED alternatives, since the original type might be discontinued.

========== ========================== ====  ==== === ==== ==== === ======= =============
Series     Part.No.                     lm     V  mA Case    € Min Distr.  No
========== ========================== ====  ==== === ==== ==== === ======= =============
Duris S 10 GW P7LP32.EM-RSRU-XX52-1   1400  38.0 300 7070 1.90  50 RS      8792889
Duris S 10 GW P7LM32.EM-QURQ-XX52-1   1050  28.5 300 7070 1.72  50 Mouser
Duris S 8  GW P9LT31.EM-PSPU-XX52-1    610  31.0 150 5050 0.96  25 DigiKey 475-3200-1-ND
Duris S 8  GW P9LT32.EM-PSPU-XX52-1    610   6.2 750 5050 0.96  25 DigiKey 475-3207-1-ND
Duris S 8  GW P9LR31.EM-PQPS-XX52-1    500  24.8 150 5050 0.76  25 DigiKey 475-3187-1-ND
Duris S 8  GW P9LR32.EM-PQPS-XX52-1    500   6.2 600 5050 0.80  25 DigiKey 475-3193-1-ND
Duris S 8  GW P9LMS1.EM-NSNU-57S5-0    395  19.8 200 4SMD 0.91  50 RS      8108054
Duris S 8  GW P9LMS2.EM-NQNS-57S5-0    350  19.8 200 4SMD 0.60  50 RS      8768969
========== ========================== ====  ==== === ==== ==== === ======= =============


Led Driver
----------

The LED driver section is very flexible.  Adjusting component values you can
drive almost any LED up to 1.5A / 80V, (but you'll have to use a separate 5V
power supply if your LED power supply goes over 35V, which is the max. the 7805
can handle).  The components you have to adjust are the resistor of the constant
current source and the base resistors of the high-side BD140's.

The chosen LEDs have a forward voltage of:

=== ====
V_f    V
=== ====
min 18.6
typ 19.8
max 22.2
=== ====

@ a forward current of 200mA.

::

   LM 317 Drop-Out Voltage  (@ I_O = 200mA, T_j = 25°C)       =  1.65V
   LM 317 V_adjust                                            =  1.25V
   BD 140 C-E Saturation Voltage (@ I_C = -0.5A, I_B = -50mA) = -0.5V

This gives us a minimum drop of 3.9V.  A 24V power supply would not drive all
LEDs to their full potential, so I opted for a 28V one.  (Any power supply up to
35V @ 250mA will do, more than that will blow the 7805.  Add a heat sink to the
7805 eventually.)


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

The project was prototyped on an Arduino Nano.  You can alternatively build it
(without USB support) using an ATmega328p with a few external components.
