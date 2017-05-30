Electrical design
=================

The main goals of this design are:

- bright and affordable LEDs,

- user-solderable off-the-shelf components.

High performance LEDs are designed for surface mounting, which makes them
difficult to solder by traditional means.  Most of them are also very small.
After research I settled on this LED make:

*Osram Duris S8 GW P9LMS1.EM-NSNU-57S5-0*

which is a 20V, 200mA, 359 → 450lm LED, available at prices of €0.91 in 50+
quantities from https://www.rsonline-privat.de/ (or https://de.rs-online.com/
for corporate clients).  It comes in a 5*5 mm package, which you should be able
to hand-solder onto a standard 2.54" grid PCB.

(Alternatively there is a cheaper and newer version of this LED, the *GW
P9LMS2.EM-NQNS-57S5-0*, 304 → 390 lm, which comes at €0,60 in 50+ quantities,
but in a different package which may not be solderable as easily.)


Led Driver
----------

The LED driver section is very flexible.  Adjusting component values you can
drive almost any LED up to 1.5A / 80V, (but you'll have to use a separate 5V
power supply if your LED power supply goes over 35V, which is the max. the 7805
can handle).  The components you have to adjust is the resistor of the constant
current source and the base resistors of the high-side BD140's.

The chosen LEDs have a forward voltage of:

========= =====
V_f (min) 18.6V
V_f (typ) 19.8V
V_f (max) 22.2V
========= =====

@ a forward current of 200mA.

::

   LM 317 Drop-Out Voltage                     =  1.65V (@ I_O = 200mA, T_j = 25°C)
   LM 317 V_adjust                             =  1.25V
   BD 139 Collector-Emitter Saturation Voltage =  0.5V  (@ I_C =  0.5A, I_B =  50mA)
   BD 140 Collector-Emitter Saturation Voltage = -0.5V  (@ I_C = -0.5A, I_B = -50mA)

This gives us a total drop of 3.9V.  A 24V power supply would be marginal, so I
opted for a 28V one.  (Any power supply up to 35V @ 250mA will do, more will blow
the 7805.  Add a heat sink to the 7805 eventually.)


Constant Current Source
-----------------------

To get even luminosity I use an LM 317 as constant current source.  The resistor
value is given by::

  R_adj = 1.25V / I_O
  R_adj = 1.25V / 200mA = 6.25ohm, 0.25W

The nearest standard value is 6.2ohm E24 (or 4.7ohm + 1.5ohm E12).

N\.B. the constant current source also drives the bases of the high-side BD140
transistors, which sink 10mA with the chosen resistors (but would need 40mA of
base current to switch 1.5A LEDs).


Microcontroller
---------------

The project was prototyped on an Arduino Nano.  You can alternatively build it
using an ATmega328p with a few external components.
