=================
 Shooting Manual
=================

Before taking an RTI image you must decide on :ref:`which lens to use <lenses>`
and if you will be shooting :ref:`RAW images or JPEGs <raw_or_jpeg>`.  Also look
into :ref:`vibration reduction <vibration_reduction>` techniques.


Camera Settings
===============

1. **Choose RAW or JPEG.** Nikon: :guilabel:`QUAL` + :guilabel:`rear dial`.

2. **Set the white balance** to incandescent light if you are shooting JPEG.
   Nikon: :guilabel:`WB` + :guilabel:`rear dial`.  This setting has no effect on
   RAW images.

3. **Set the ISO sensitivity.** Use the ISO setting with least noise.  Nikon:
   :guilabel:`ISO` + :guilabel:`rear dial`.  All cameras have one ISO setting
   where the sensor noise is minimal, usually the lowest, but you should check
   with your camera manual or the internet.

4. **Set the lens to manual focus.** Nikon: Push the switch on the lens from
   :guilabel:`M/A` to :guilabel:`M`.

5. **Set the camera to manual exposure.** Nikon: :guilabel:`MODE` +
   :guilabel:`rear dial`.  Use the smallest aperture for maximum depth of field (:term:`DOF`).
   Nikon: :guilabel:`front dial`.  Set the exposure time.  Nikon: :guilabel:`rear dial`.

6. Either:

   a) **Turn on live view**.  Nikon D850: :kbd:`Lv`  Or

   b) **Turn on exposure delay mode.** Nikon D700: menu :menuselection:`d9`.


Dome Setup
==========

1. **Connect the RTI dome to the CRM 114** (Camera to RTI Module) using two
   Cat.5 or better Ethernet cables.

2. **Fit lens and camera to the dome.** Align the camera's vertical axis with
   the 0° axis of dome.  A polarizing filter between dome and lens will allow
   you to turn the camera with more ease.

   .. warning::

      You will not get working RTI pictures without correct alignment of dome
      and camera.

3. **Connect the camera to the CRM** using two interface cables.  Connect the
   PC-Flash output on the camera with the flash input on the CRM.  Connect the
   trigger input on the camera with the trigger output on the CRM.

4. **Connect the AC/DC adaptor to the CRM** and plug it into a wall socket.  The
   green light on the CRM should come on.

5. **Turn the camera on.**


Shooting
========

1. **Put the object under the dome** in the exact center.

2. **Short press the green button** for focus light.  You can short press again
   to turn the focus light off.

3. **Focus the object.**  Close the viewfinder.  If you are using live view,
   press :kbd:`Lv` to exit and re-enter live view mode.

4. **Long press the green button** to start the picture run.  The red button
   resets the CRM.  Use it to abort a shooting run.

   .. warning::

      Check your pictures for over- (specular lights) or under-exposure.  Adjust
      the aperture, time and maybe ISO settings and repeat the run.  The focus
      light alone does not give enough guidance to set exposure parameters [#]_.


Various Notes
=============

Your mileage will depend on how fast the camera can save the pictures to the
memory card.  Many cameras have a small internal buffer, which fills up after a
short burst, and after that the speed of the memory card limits the frame rate.

For greatest depth of field (:term:`DOF`) use the smallest aperture (that is the
*highest* aperture number) that you can manage.  Most cameras use the largest
aperture for viewing, the DOF you see in the viewfinder is much less than the
DOF you will get on the picture.  In case of a 3D object focus on the middle
ground.

If you want color, place the object onto a :term:`grey card`.  This will make it
a lot easier to find the correct white balance in the raw converter software.


Footnotes
=========

.. [#] The focus light (current shared by N LEDs) has not the same intensity as
       the individual LED (current used by one LED) because LED emission is not
       linear with current.  Also specular lights will be N times brighter if
       the object is lighted by one LED only.
