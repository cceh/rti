==========================================
 Reflectance Transformation Imaging (RTI)
==========================================

*Reflectance Transformation Imaging,* formerly known as *Polynomial Texture
Mapping* (PTM), is a technique of imaging and interactively displaying objects
under varying lighting conditions to reveal surface phenomena.

N light sources of equal intensity are positioned on a hemisphere (the dome).
The object to image is positioned at the center of the hemisphere.  The camera
is positioned outside the dome and sees the object through a hole in the dome.

The object is now illuminated once from every light source, and N exposures are
taken.  The object and the camera must not move between exposures.

These N pictures are then combined into one PTM image with the RTI Builder
software.  The PTM image can be viewed with the standalone RTI Viewer or a
javascript web viewer.


Mathematical Background
=======================

The original PTM method has been developed Tom Malzbender, Dan Gelb, and Hans
Wolters at Hewlett-Packard Laboratories.  [Malzbender2001]_ Improvements have
been suggested.  [Zhang2014]_ We here describe the original method.

The PTM consist in fitting the :math:`N` samples obtained of each pixel under
varying lighting conditions to a biquadratic polynomial.  Then, for each pixel,
we store the coefficients of the biquadratic polynomial.  Using the polynomial
we can interpolate an arbitrary light direction.

The polynomial used for curve fitting is:

.. math::

   b = x_0u^2 + x_1v^2 + x_2uv + x_3u + x_4v + x_5

:math:`(u,v)` are projections of the normalized light vector onto the object
plane.  :math:`b` is the sampled surface luminance. [Malzbender2001]_

To fit the curve to the samples we use a Least Squares approximation and solve
it using Singular Value Decomposition (SVD). "In the case of an overdetermined
system, SVD produces a solution that is the best approximation in the
least-squares sense." [Press2007]_ p. 793

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

Replacing:

.. math::

   \mathbf{U} \boldsymbol{\Sigma} \mathbf{V}^T \mathbf{x} \approx \mathbf{b}

the solution of which is:

.. math::

   \mathbf{x} = \mathbf{V} \begin{bmatrix} diag (1/w_j) \end{bmatrix} \mathbf{U}^T \mathbf{b}

Since :math:`U,V` are orthogonal matrices, their inverse is equal to their
transpose.  The center matrix :math:`\boldsymbol{\Sigma}` is a diagonal matrix,
whose inverse, denoted by :math:`diag (1/w_j)`, consists of the reciprocals of
the (non-zero) diagonal elements of :math:`\boldsymbol{\Sigma}`.

See: [Press2007]_ p. 73.

A naive GNU Octave implementation of this is:

.. code-block:: octave

   function X = ptm(A,b)

   [U S V] = svd(A,0)   % 'economy size' SVD
   M       = V * diag (1 ./ diag (S)) * U'
   X       = M * b

   end

Note that the SVD needs to be computed only once for every different arrangement
of light sources.  The only part of the above formula that varies for each pixel
is :math:`\mathbf{b}`.

.. [Malzbender2001] Malzbender, Tom [et al.]. Polynomial Texture Maps.
                    http://www.hpl.hp.com/research/ptm/papers/ptm.pdf

.. [Malzbender2001a] Malzbender, T., Gelb, D.  Polynomial Texture Map (.ptm) File Format.
                     http://www.hpl.hp.com/research/ptm/downloads/PtmFormat12.pdf

.. [Press2007] Press, William H. [et al.]. Numerical recipes: the art of
               scientifical computing. 3rd edition 2007. University Printing
               House, Cambridge.

.. [Wikipedia] https://en.wikipedia.org/wiki/Polynomial_texture_mapping

.. [Zhang2014] Zhang and Drew.  Efficient robust image interpolation and surface
               properties using polynomial texture mapping.  EURASIP Journal on
               Image and Video Processing.  2014.
               https://doi.org/10.1186/1687-5281-2014-25
