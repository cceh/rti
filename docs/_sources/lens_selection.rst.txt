.. _lenses:

================
 Lens Selection
================

You must use a lens that has a minimum focal distance equal to or less than the
distance between camera sensor plane and object.

You must also select a lens with the right focal length.  Too small a focal
length will make the pictured object tiny and too large a focal length will
image only a part of the object.

With the Ø 50cm dome at CCeH, as a rule of thumb we use a 105mm macro lens for
coin-sized objects and a 60mm macro for objects up to 10cm diagonal.


Buying Lenses
=============

Before you buy a lens make sure that it will focus down to the object distance.
For domes smaller than 1 m in diameter you will probably have to buy macro
lenses.  The minimum focal distance is stated on the datasheet of the lens and
is measured from the focal plane.  A Plimsoll Mark (a circle with a horizontal
line through it) on your camera shows where the focal plane is.

If the front thread of the lens turns while focusing you will not be able to
mount the lens on the dome by the front thread, instead you'll have to use a
tripod to suspend the camera above the dome.  We advise buying a lens whose
front does not turn when focusing.

Some lenses will extend when focusing, this is unpractical when mounted on the
dome by the front thread.  We advise buying lens that does not extend.  Nikon
calls this "internal focusing".

With some lenses the focus will wander if the lens is pointed up or down,
depending on the quality and the weight of the internal mechanics.  Avoid those
lenses as they are useless for RTI.


Object Sizes
============

The following tables shows the maximum size of the objects you can reasonably
expect to image using different lenses and dome sizes.  A full frame (eg. 36 ×
24 mm) sensor is assumed.  Values are shown only if a lens exists that will
focus if mounted on a dome of that size.


.. csv-table:: Maximum angle of view
   :file: Angles of View.csv
   :header: "Focal Length (mm)", "Horizontal (deg)", "Vertical (deg)"


.. csv-table:: Maximum object sizes in a dome of 50cm Ø
   :file: Sizes Dome 50.csv
   :header: "Focal Length (mm)", "Width (cm)", "Height (cm)"


.. csv-table:: Maximum object sizes in a dome of 100cm Ø
   :file: Sizes Dome 100.csv
   :header: "Focal Length (mm)", "Width (cm)", "Height (cm)"


The viewing angle was computed using the formula:

.. math::

   \alpha = 2 \arctan \left( \frac{d}{2f} \right)
