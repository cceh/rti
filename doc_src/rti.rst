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

The file size of a PTM is roughly 3--6 times the file size of the corresponding
JPEG file.


Mathematical Background
=======================

The original PTM method has been developed Tom Malzbender, Dan Gelb, and Hans
Wolters at Hewlett-Packard Laboratories.  [Malzbender2001]_ Improvements have
been suggested: [Drew2012]_, [Zhang2014]_.  We here describe the original
method.

The PTM consist in fitting the :math:`N` samples obtained of each pixel under
varying lighting conditions to a biquadratic polynomial.  Then, for each pixel,
we store the coefficients of the biquadratic polynomial.  Using the polynomial
we can interpolate an arbitrary light direction.

The polynomial used for curve fitting is:

.. math::
   :label: poly

   b = x_0u^2 + x_1v^2 + x_2uv + x_3u + x_4v + x_5

where :math:`(u,v,w)^T` is the normalized light vector and :math:`b` is the
sampled surface luminance. [Malzbender2001]_

To fit the curve to the samples we use a Least Squares approximation and solve
it using Singular Value Decomposition (SVD). "In the case of an overdetermined
system, SVD produces a solution that is the best approximation in the
least-squares sense." [Press2007]_, Chapter 15.4.2.

Our design matrix:

.. math::

   \mathbf{A}^{N\times6} =
   \begin{bmatrix}
     u_0^2 & v_0^2 & u_0v_0 & u_0 & v_0 & 1 \\
     u_1^2 & v_1^2 & u_1v_1 & u_1 & v_1 & 1 \\
     u_2^2 & v_2^2 & u_2v_2 & u_2 & v_2 & 1 \\
     \vdots & \vdots & \vdots & \vdots & \vdots & \vdots \\
     u_{N-1}^2 & v_{N-1}^2 & u_{N-1}v_{N-1} & u_{N-1} & v_{N-1} & 1
   \end{bmatrix}

The vector of samples:

.. math::

   \mathbf{b}^N = b_0,\dots,b_{N-1}

The sought least-squares solution vector:

.. math::

   \mathbf{x}^6 = x_0,\dots,x_5

For each pixel we have to solve:

.. math::
   :label: axb

   \mathbf{Ax} \approx \mathbf{b}

We do a SVD of :math:`\mathbf{A}^{N\times6}` (See [Golub2013]_, Chapter 2.4):

.. math::

   \mathbf{U}^T\mathbf{AV} = \boldsymbol{\Sigma} = \mathrm{diag}(\sigma_1,\dots,\sigma_6) \in \Bbb{R}^{N\times6}

where :math:`\sigma_1\geq\sigma_2\geq\dots\geq\sigma_6\geq0.`

:math:`\mathbf{U}^{N\times N}` and :math:`\mathbf{V}^{6\times6}` are orthogonal,
so their inverses are equal to their transposes, and we can rearrange:

.. math::

   \mathbf{A} = \mathbf{U} \boldsymbol{\Sigma} \mathbf{V}^T

We replace into :eq:`axb`:

.. math::

   \mathbf{U} \boldsymbol{\Sigma} \mathbf{V}^T \mathbf{x} \approx \mathbf{b}

:math:`\boldsymbol{\Sigma}` is diagonal, so its inverse is the diagonal matrix
whose elements are the reciprocals of the elements
:math:`\sigma_1,\dots,\sigma_6` and we can rearrange for the final least-squares
solution of :eq:`axb`:

.. math::

   \mathbf{x}=\mathbf{V}\begin{bmatrix}\mathrm{diag}(1/\sigma_1,\dots,1/\sigma_6)\end{bmatrix}\mathbf{U}^T\mathbf{b}

See: [Press2007]_, Chapter 2.6.4.

A naive GNU Octave implementation of the PTM is:

.. code-block:: octave

   function X = ptm(A,b)

   [U S V] = svd(A,0)   % 0 requests thin SVD
   M       = V * diag (1 ./ diag (S)) * U'
   X       = M * b

   end

Note that the SVD needs to be computed only once for every different arrangement
of light sources.  The only part of the above formula that varies for each pixel
is :math:`\mathbf{b}`.


.. [Drew2012] Drew, M.S. [et al.] 2012, *Robust Estimation of Surface Properties
              and Interpolation of Shadow/Specularity Components*
              http://www.cs.sfu.ca/~mark/ftp/Ivc2012/ivc2012.pdf

.. [Golub2013] Golub, G.H., and Van Loan, C.F. 2013, *Matrix Computations,* 4th
               edition, (John Hopkins University Press, Baltimore)

.. [Malzbender2001] Malzbender, T., Gelb, D., and Wolters, H. 2001, *Polynomial
                    Texture Maps,*
                    http://www.hpl.hp.com/research/ptm/papers/ptm.pdf

.. [Malzbender2001a] Malzbender, T., and Gelb, D. 2001, *Polynomial Texture Map
                     (.ptm) File Format,* Version 1.2,
                     http://www.hpl.hp.com/research/ptm/downloads/PtmFormat12.pdf

.. [Motta2001] Motta, G., Winberger, M.J. 2001, *Compression of Polynomial
               Texture Maps,* (HP Laboratories, Palo Alto)
               http://www.hpl.hp.com/techreports/2000/HPL-2000-143R2.pdf

.. [Press2007] Press, W.H. [et al.] 2007, *Numerical recipes: the art of
               scientifical computing,* 3rd edition, (Cambridge University
               Press, Cambridge)

.. [Wikipedia] https://en.wikipedia.org/wiki/Polynomial_texture_mapping

.. [Zhang2014] Zhang, M., and Drew, M.S. 2014, *Efficient robust image
               interpolation and surface properties using polynomial texture
               mapping,* (EURASIP Journal on Image and Video Processing)
               https://doi.org/10.1186/1687-5281-2014-25
