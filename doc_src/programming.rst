.. _programming:

=====================================
 Programming the AVR-Microcontroller
=====================================

How to install a new :ref:`firmware` into the CRM.  To do this you need to
purchase an IPS programmer device.  We recommend the `AVR Dragon
<https://en.wikipedia.org/wiki/AVR_microcontrollers#AVR_Dragon>`_ device, which
will set you back ~ €60 in Germany.


Prerequisites
=============

Install avrdude: :command:`apt-get install avrdude`.

If you are not using an AVR-Dragon: Change into the :file:`firmware` directory
and edit the :code:`MY_AVRDUDE =` line in the :file:`Makefile` to suit your ISP
programmer.


Initialize a New Microcontroller
================================

To initialize a new microcontroller.  This command only needs to be run once on
every microcontroller chip.

.. code-block: shell

   make my_init_fuses


Program
=======

0. Open the case.

1. Connect the AVR-Dragon programmer with the CRM Module using a 6-way ribbon
   cable.  Make sure to connect pin 1 on the Dragon with pin 1 on the CRM!

2. Connect the AVR-Dragon with the PC using an USB cable.

3. Connect the power source to the CRM.

4. Change into the :file:`firmware` directory and say: :command:`make
   myispload`.

5. Disconnect the 6-way cable from the CRM.
