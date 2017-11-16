==========================================
 Reflectance Transformation Imaging (RTI)
==========================================

*Reflectance Transformation Imaging,* also known as *Polynomial Texture Mapping*
(PTM), is a technique of imaging and interactively displaying objects under
varying lighting conditions to reveal surface phenomena.

N light sources of equal intensity are positioned on a half sphere (the dome).
The object to image is positioned at the center of the half sphere.  The camera
is positioned outside the dome and sees the object through a hole in the dome.

The object is now illuminated once from every light source, yielding N pictures.

Since the object and the camera are not moved, this procedure yields N samples
for each pixel of the image.

For each pixel of the image we fit the N samples to a model.  This reduces the
volume of data and at the same time allows us to interpolate an arbitrary light
direction at the cost of some small error in the pixel luminance.

.. math::

   b = x_0u^2 + x_1v^2 + x_2uv + x_3u + x_4v + x_5

(u,v) are projections of the normalized light vector onto the object plane.  b
is the resultant surface luminance. [Malzbender2001]_

To fit the N samples to this model we use a General Linear Least Squares method
and solve it using Singular Value Decomposition (SVD). "In the case of an
overdetermined system, SVD produces a solution that is the best approximation in
the least-squares sense." [Press2007]_ p. 793

Our design matrix:

.. math::

   \mathbf{A} = \begin{bmatrix}
                      u_0^2 & v_0^2 & u_0v_0 & u_0 & v_0 & 1 \\
                      u_1^2 & v_1^2 & u_1v_1 & u_1 & v_1 & 1 \\
                      u_2^2 & v_2^2 & u_2v_2 & u_2 & v_2 & 1 \\
                      \vdots & \vdots & \vdots & \vdots & \vdots & \vdots \\
                      u_{N-1}^2 & v_{N-1}^2 & u_{N-1}v_{N-1} & u_{N-1} & v_{N-1} & 1
                \end{bmatrix}

The vector of samples:

.. math::

   \mathbf{b} = b_0,\cdots,b_{N-1}

The sought least-squares solution vector:

.. math::

   \mathbf{x} = x_0,\cdots,x_5

For each pixel we have to solve:

.. math::

   \mathbf{A x} \approx \mathbf{b}

We do a SVD of :math:`\mathbf{A}`:

.. math::

   \mathbf{A} = \mathbf{U} \boldsymbol{\Sigma} \mathbf{V}^T

Then we approximate our linear system:

.. math::

   \mathbf{x} = \mathbf{V} \begin{bmatrix} diag (1/w_j) \end{bmatrix} \mathbf{U}^T \mathbf{b}

See: [Press2007]_ p. 73.

Note that the SVD needs to be computed only once for every different arrangement
of light sources.  The only part of the above formula that varies for each pixel
is :math:`\mathbf{b}`.

.. [Malzbender2001] Malzbender, Tom [et al.]. Polynomial Texture Maps.
                    http://www.hpl.hp.com/research/ptm/papers/ptm.pdf

.. [Press2007] Press, William H. [et al.]. Numerical recipes: the art of
               scientifical computing. 3rd edition 2007. University Printing
               House, Cambridge.
