.. _programming:

=====================================
 Programming the AVR-Microcontroller
=====================================

How to install a new :ref:`firmware` into the CRM.  To do this you need to
purchase an IPS programmer device.  We recommend the `AVR Dragon
<https://en.wikipedia.org/wiki/AVR_microcontrollers#AVR_Dragon>`_ device, which
will set you back ~ €60 in Germany.

To program a microcontroller chip that is mounted in the CRM module you need a
6-way ribbon cable.  To program microcontroller chips when they are not yet
mounted, you must solder a socket to the AVR-Dragon and use jumper cables.
Refer to the AVR-Dragon instruction manual.


Prerequisites
=============

Install avrdude:

.. code-block:: console

   apt-get install avrdude

If you are not using an AVR-Dragon: Edit the :code:`MY_AVRDUDE =` line in the
file :file:`firmware/Makefile` to suit your ISP programmer.


Program the Microcontroller
===========================


To program or reprogram a microcontroller chip, when it is already mounted in
the CRM Module:

0. Open the case.

1. Connect the AVR-Dragon programmer with the CRM Module using a 6-way ribbon
   cable.  Make sure to connect pin 1 on the Dragon to pin 1 on the CRM!

2. Connect the AVR-Dragon with the PC using an USB cable.

3. Connect the power source to the CRM.

4. Change into the :file:`firmware` directory.

   .. code-block:: console

      cd firmware

5. If this is the first time you prgram this microcontroller :ref:`initialize
   it<µC-init>`:

   .. code-block:: console

      make my_initfuses

6. Program the microcontroller:

   .. code-block:: console

      make my_ispload

7. Disconnect the 6-way cable from the CRM, remove the power cord and close the
   case.


.. _µC-init:

Initialize a New Microcontroller
================================

To initialize a factory-new microcontroller run the following command:

.. code-block:: console

   make my_init_fuses

This command only needs to be run once on every microcontroller chip (but it
doesn't hurt doing it more often).  It tells the microcontroller to use the
external crystal oscillator instead of the internal one.
