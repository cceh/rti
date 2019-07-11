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

3. **Set the ISO sensitivity.** Use the ISO sensitivity with least noise.
   Nikon: :guilabel:`ISO` + :guilabel:`rear dial`.  All cameras have one ISO
   sensitivity where the sensor noise is minimal, usually the lowest, but you
   should check with your camera manual or the internet.

4. **Set the lens to manual focus.** Nikon: Push the switch on the lens from
   :guilabel:`M/A` to :guilabel:`M`.

5. **Set the camera to manual exposure.** Nikon: :guilabel:`MODE` +
   :guilabel:`rear dial`.  Use the smallest aperture for maximum depth of field (:term:`DOF`).
   Nikon: :guilabel:`front dial`.  Set the shutter speed.  Nikon: :guilabel:`rear dial`.

6. **Set the camera to Continuous High Speed Mode** Nikon: :guilabel:`left dial`.

7. Either:

   a) **Turn on live view**.  Nikon D850: :guilabel:`Lv`.  Or

   b) **Turn on exposure delay mode.** Nikon D700: menu :menuselection:`d9`.

8. Check the clock of the camera. Nikon: :guilabel:`info`.


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

4. **Connect an external monitor to the camera** using a cable with a mini-HDMI
   plug at one end.  You may skip this step if you don't want to use an external
   monitor.

5. **Connect the AC/DC adaptor to the CRM** and plug it into a wall socket.  The
   green light on the CRM should come on.

6. **Turn the camera on.**


Shooting
========

1. **Put the object under the dome** in the approximate center of the dome. You
   will find the exact position later.

2. **Turn the pilot light on.** Short press the green button to turn the pilot
   lights on.  You can also use this button to turn the lights off again.

3. **Focus and center the object.** Look through the viewfinder and use the
   focus ring on the lens to put the object into focus.  Move the dome or the
   object around until the object is in the exact center of the picture.  Be
   careful not to rotate the camera from the 0° position.

   If you want to use live view, press :guilabel:`Lv` to enter (and exit) live
   view mode.  This will show the picture on the camera monitor.  If you have an
   external monitor attached to the camera you will also see the picture there.

4. **Press the shutter-release button** to make a test exposure.

   Review the picture and check the histogram.  Press :guilabel:`▶` and then
   press what Nikon calls the multi selector in the down direction until you get
   to the histogram view.  Adjust the lens aperture (:guilabel:`front dial`)
   and/or the shutter speed (:guilabel:`rear dial`) (and maybe also the ISO
   sensitivity).  Repeat this step until you are satisfied with the picture
   histogram.

5. **Close the viewfinder.** This avoids stray light falling into the camera
   from above.

6. **Press** :guilabel:`Lv` **to enter live view mode.**

7. **Long press the green button** to start the shooting run.  The camera will
   start taking exposures.

   During a shooting run don't touch the camera or dome and avoid anything that
   may cause vibration.

   The red button resets the CRM.  Use it to abort a shooting run.

   The shooting run is done when the green light comes on again.

   Remove the object from under the dome, open the camera viewfinder and repeat
   from step 1.

   .. warning::

      After every shooting run check all your pictures' histograms for over-
      (specular lights) or under-exposure.  If unsatisfied adjust the aperture,
      shutter speed and maybe ISO sensitivity and repeat the shooting run.  The
      pilot light alone does not give you complete guidance to set exposure
      parameters [#]_.


Various Notes
=============

Working speed: Your mileage may vary depending on the shutter speed you have to
select and on the speed of the memory card in the camera.  Cameras have an
internal buffer, which fills up after a short burst, after that the speed of the
memory card may limit the frame rate.  If your exposure time is longer than the
writing time the speed of the memory card may not matter.

For greatest depth of field (:term:`DOF`) use the smallest aperture (that is the
*highest* aperture number).  When you look through the viewfinder you see the
DOF of the largest aperture, which is much less than the DOF you will get on the
picture.  For best results focus on the middle ground.

If you want color, place the object onto a :term:`grey card`.  This will make it
easier to find the correct white balance in the raw converter software.


Footnotes
=========

.. [#] The pilot light (current shared by N LEDs) has not the same intensity as
       the individual LED (current used by one LED) because LED emission is not
       linear with current.  Also specular lights will be N times brighter if
       the object is lighted by one LED only.
