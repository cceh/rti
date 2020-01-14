=================
 Shooting Manual
=================

Before taking an RTI image you must decide on :ref:`which lens to use <lenses>`
and if you will be shooting :ref:`RAW images or JPEGs <raw_or_jpeg>`.  Also look
into :ref:`vibration reduction <vibration_reduction>` techniques.

Always refer to the online `camera manuals`_ for more details.

.. note::

   This guide refers to Nikon brand cameras.  The RTI dome works with other
   brand cameras too.  We will add those cameras as soon as we can get our hands
   around some.


Camera Settings
===============

#. **Turn the camera on.**  Turn the power switch to :guilabel:`ON`.

#. **Check the clock of the camera.** Press :guilabel:`MENU`, then select
   :menuselection:`SETUP MENU`, :menuselection:`Time zone and date`,
   :menuselection:`Date and time`.

#. **Choose RAW or JPEG.** Press :guilabel:`QUAL` and turn the :guilabel:`rear
   dial` until either :menuselection:`RAW` or :menuselection:`FINE★` appears in
   the display.

#. **Set the white balance** to incandescent light if you are shooting JPEG.
   Press :guilabel:`WB` and turn the :guilabel:`rear dial` until a light bulb
   symbol appears on the camera display.  This setting has no effect on RAW
   images.

#. **Set the camera to manual exposure.** Press :guilabel:`MODE` and turn the
   :guilabel:`rear dial` until an :menuselection:`M` appears in the display.

#. **Set the ISO sensitivity.** Use the ISO sensitivity with least noise.  Press
   :guilabel:`ISO` and turn the :guilabel:`rear dial` until the chosen value appears
   on the camera display.  All cameras have one ISO sensitivity where the sensor
   noise is minimal, usually the lowest, but you should check with your camera
   manual or the internet.

#. **Set the camera to Continuous High Speed Mode** press the safety
   button near the dial and turn the :guilabel:`left dial` to the
   :guilabel:`CH` position.

#. **Set the camera and lens to manual focus.** Set the focus-mode
   selector on the camera to :guilabel:`M`.  Also set the focus mode switch on
   the lens (if present) to :guilabel:`M`.

#. **D700 only: Turn on exposure delay mode.** Press :guilabel:`MENU` and select
   :menuselection:`CUSTOM SETTINGS MENU`, :menuselection:`d9`.

#. **Turn the camera off.**  Turn the power switch to :guilabel:`OFF`.


Dome Setup
==========

1. **Connect the RTI dome to the CRM 114** (Camera to RTI Module) using two
   Cat.5 or better Ethernet cables.  Don't swap the cables: make sure to connect
   socket 1 on the CRM 114 with socket 1 on the dome.

2. **Connect the camera to the CRM** using two interface cables.  Connect the
   PC-Sync output on the camera with the flash input on the CRM.  Connect the
   trigger input on the camera with the trigger output on the CRM.

3. **Fit the lens and camera to the dome.** Align the camera's vertical axis
   with the 0° axis of dome.  The tripod socket on the camera should be near to
   the 0° marking on the dome while the bottom plate of the camera should be at
   exactly 90° angle to that mark.  Note: A polarizing filter between dome and
   lens will allow you to align the camera with more ease.

   .. warning::

      A correct alignment of dome and camera is essential to RTI.

4. **Optionally connect an external monitor to the camera** using a cable with a
   mini-HDMI plug at one end.

5. **Connect the AC/DC adaptor to the CRM** and plug it into a wall socket.  The
   green light on the CRM should come on.

6. **Turn the camera on.**  Turn the power switch to :guilabel:`ON`.


Shooting
========

#. **Put the object under the dome** in the approximate center of the dome. You
   will find the exact position later.

#. **Turn the pilot light on.** Short press the green button to turn the pilot
   lights on.  You can also use this button to turn the lights off again.

#. **Focus and center the object.** Look through the viewfinder and use the
   focus ring on the lens to put the object into focus.  Move the dome or the
   object around until the object is in the exact center of the picture.  Be
   careful not to rotate the camera from the 0° position.

   If you want to use live view, press :guilabel:`Lv` to enter (and exit) live
   view mode.  This will show the picture on the camera monitor.  If you have an
   external monitor attached to the camera you will also see the picture there.

#. **Take a test exposure.** Press the shutter-release button to make a test
   exposure.

   Review the picture and check the histogram.  Press :guilabel:`▶` and then
   press what Nikon calls the multi selector in the down direction until you get
   to the histogram view.  Adjust the lens aperture (:guilabel:`front dial`)
   and/or the shutter speed (:guilabel:`rear dial`) (and maybe also the ISO
   sensitivity).  Repeat this step until you are satisfied with the picture
   histogram.

#. **Close the viewfinder.** This avoids stray light falling into the camera
   from above.

#. **Turn on live view**.  Nikon D800 and later models only.  Press
   :guilabel:`Lv`.  The object should now appear on the camera screen.

#. **Long press the green button** to start the shooting run.  The camera will
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

Choosing the lens aperture: the right lens aperture depends on the depth of
field (:term:`DOF`) you need to bring the whole object in focus.  A flat object
requires only a small DOF, an object with more relief requires a larger DOF.
The smallest aperture gives you the largest DOF.  Note that higher numbers mean
smaller apertures: f/8 is a smaller aperture than f/4.  OTOH at apertures
smaller than f/11 lens diffraction will become noticeable.

If you look through the viewfinder you see the DOF of the largest aperture,
which is much less than the DOF you will get on the actual picture.  Now focus
somewhere on the middle ground of the object.  If you press the DOF preview
:guilabel:`Pv` button, the lens will stop down and you will see in the
viewfinder the actual DOF you'll get in the picture.

If you want color, place the object onto a :term:`grey card` or put a
:term:`color reference chart` near the object.  This will make it easier to find
the correct white balance in the raw converter software.


Camera Manuals
==============

  - `Nikon D700 <https://downloadcenter.nikonimglib.com/en/products/15/D700.html>`_

  - `Nikon D800 <https://downloadcenter.nikonimglib.com/en/products/16/D800.html>`_

  - `Nikon D810 <https://downloadcenter.nikonimglib.com/en/products/176/D810.html>`_

  - `Nikon D850 <https://downloadcenter.nikonimglib.com/en/products/359/D850.html>`_


Footnotes
=========

.. [#] The pilot light (current shared by N LEDs) has not the same intensity as
       the individual LED (current used by one LED) because LED emission is not
       linear with current.  Also specular lights will be N times brighter if
       the object is lighted by one LED only.
