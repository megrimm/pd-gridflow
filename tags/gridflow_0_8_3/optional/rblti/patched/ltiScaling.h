/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiScaling.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.11.2003
 * revisions ..: $Id$
 */

#ifndef _LTI_SCALING_H_
#define _LTI_SCALING_H_

#include "ltiModifier.h"
#include "ltiInterpolatorType.h"

namespace lti {

  // this is a private class defined in the cpp file for internal use only
  class scalingWorker;

  /**
   * Scaling implements a functor to rescale an image using a real valued
   * scaling factor.
   *
   * This functor represents a faster alternative to lti::geometricTransform,
   * used if you only need to scale the image, without any other kind of 
   * geometric mapping.
   * 
   * It is usually employed to upsample an image, using some interpolation
   * policy indicated in the parameters.  At this point only bilinear
   * interpolation is supported, but this may change as soon as the new
   * family of interpolators for the LTI-Lib are available.
   *
   * You can give also factors smaller than one to downsample an image, but
   * this will just take the interpolated corresponding pixel of the original
   * image, without consideration of its neighborhood.  For a more
   * theoretically founded downsampling functor, you can use
   * lti::downsampling, which allows you to make a low-pass filtering, to
   * avoid artifacts resulting from a violation of the Nyquist sampling
   * theorem.  Nevertheless, if you just one to downsample for some
   * visualization, this functor might be enough (lti::decimation can be even
   * faster if the downsampling factor is integer).
   * 
   *
   * For more complex operations you can still use the lti::geometricTransform 
   * functor.
   *
   * @see lti::geometricTransform, lti::rotation, lti::flipImage
   *
   * @see lti::downsampling, lti::upsampling, lti::filledUpsampling, 
   *      lti::decimation
   *
   * @ingroup gGeometry
   */
  class scaling : public modifier {
  public:
    /**
     * The parameters for the class scaling
     */
    class parameters : public modifier::parameters {
    public:
      /**
       * Default constructor
       */
      parameters();

      /**
       * Copy constructor
       * @param other the parameters object to be copied
       */
      parameters(const parameters& other);

      /**
       * Destructor
       */
      ~parameters();

      /**
       * Returns name of this type
       */
      const char* getTypeName() const;

      /**
       * Copy the contents of a parameters object
       * @param other the parameters object to be copied
       * @return a reference to this parameters object
       */
      parameters& copy(const parameters& other);

      /**
       * Copy the contents of a parameters object
       * @param other the parameters object to be copied
       * @return a reference to this parameters object
       */
      parameters& operator=(const parameters& other);


      /**
       * Returns a pointer to a clone of the parameters
       */
      virtual functor::parameters* clone() const;

      /**
       * Write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool& complete=true) const;

      /**
       * Read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool& complete=true);

#     ifdef _LTI_MSC_VER
      /**
       * This function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool& complete=true);

      /**
       * This function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool& complete=true) const;
#     endif

      // ------------------------------------------------
      // the parameters
      // ------------------------------------------------

      /**
       * Scaling factor.
       *
       * Each axis of the image can be scaled by a different factor.  The
       * \a x component of this point will be the horizontal scaling factor and
       * the \a y component the vertical scaling factor.
       *
       * Only positive values are allowed.  You can flip you image using
       * lti::flipImage if you need to first. 
       *
       * With \a scale between 0 and 1 this functor does a down-sampling and
       * above 1.0 up-sampling.
       *
       * Default value: tpoint<float>(sqrt(2),sqrt(2))
       */
      tpoint<float> scale;

      /**
       * Interpolator type to be used.
       *
       * Default value: BilinearInterpolator
       */
      eInterpolatorType interpolatorType;
    };

    /**
     * Default constructor
     */
    scaling();

    /**
     * Construct a functor using the given parameters
     */
    scaling(const parameters& par);

    /**
     * Copy constructor
     * @param other the object to be copied
     */
    scaling(const scaling& other);

    /**
     * Destructor
     */
    virtual ~scaling();

    /**
     * Returns the name of this type ("scaling")
     */
    virtual const char* getTypeName() const;
    
    /**
     * @name Standard apply methods.
     *
     * The scaling factor and interpolation types are taken from the parameters
     */
    //@{
    /**
     * Operates on the given %parameter.
     * @param srcdest image with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(image& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param src image with the source data.
     * @param dest image where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const image& src,image& dest) const;


    /**
     * Operates on the given %parameter.
     * @param srcdest matrix<ubyte> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(matrix<ubyte>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param src matrix<ubyte> with the source data.
     * @param dest matrix<ubyte> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const matrix<ubyte>& src,matrix<ubyte>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param srcdest matrix<float> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(matrix<float>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param src matrix<float> with the source data.
     * @param dest matrix<float> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const matrix<float>& src,matrix<float>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param srcdest matrix<int> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(matrix<int>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param src matrix<int> with the source data.
     * @param dest matrix<int> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const matrix<int>& src,matrix<int>& dest) const;
    //@}

    /**
     * @name Symmetric scaling apply methods.
     *
     * The scaling factor for both vertical and horizontal axes is given
     * directly.
     */
    //@{
    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used for both, the horizontal and vertical
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param srcdest image with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,image& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used for both, the horizontal and vertical 
     *            axes.  The scaling factor in the parameters will be ignored.
     * @param src image with the source data.
     * @param dest image where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,const image& src,image& dest) const;


    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used for both, the horizontal and vertical
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param srcdest matrix<ubyte> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,matrix<ubyte>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used for both, the horizontal and vertical
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param src matrix<ubyte> with the source data.
     * @param dest matrix<ubyte> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,
               const matrix<ubyte>& src,
               matrix<ubyte>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used for both, the horizontal and vertical 
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param srcdest matrix<float> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,
               matrix<float>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used for both, the horizontal and vertical
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param src matrix<float> with the source data.
     * @param dest matrix<float> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,
               const matrix<float>& src,matrix<float>& dest) const;

   /**
     * Operates on the given %parameter.
     * @param scale scaling factor used for both, the horizontal and vertical 
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param srcdest matrix<int> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,
               matrix<int>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used for both, the horizontal and vertical
     *             axes.  The scaling factor in the parameters will be ignored.
     * @param src matrix<int> with the source data.
     * @param dest matrix<int> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const float scale,
               const matrix<int>& src,matrix<int>& dest) const;

    /**
     * @name Asymmetric scaling apply methods.
     *
     * The scaling factor for horizontal and vertical axes are given through
     * the x and y components of the point respectivelly.
     */
    //@{
    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param srcdest image with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,image& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param src image with the source data.
     * @param dest image where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,const image& src,image& dest) const;


    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param srcdest matrix<ubyte> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,matrix<ubyte>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param src matrix<ubyte> with the source data.
     * @param dest matrix<ubyte> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,
               const matrix<ubyte>& src,
               matrix<ubyte>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param srcdest matrix<float> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,
               matrix<float>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used instead of the value given in the 
     *              parameters object.
     * @param src matrix<float> with the source data.
     * @param dest matrix<float> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,
               const matrix<float>& src,matrix<float>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param scale scaling factor used instead of the value given in the
     *              parameters object.
     * @param srcdest matrix<int> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,
               matrix<int>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param scale scaling factor used instead of the value given in the 
     *              parameters object.
     * @param src matrix<int> with the source data.
     * @param dest matrix<int> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const tpoint<float>& scale,
               const matrix<int>& src,matrix<int>& dest) const;

    //@}

    /**
     * Copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    scaling& copy(const scaling& other);

    /**
     * Alias for copy member
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    scaling& operator=(const scaling& other);

    /**
     * Returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * Returns used parameters
     */
#ifdef SWIG
    const parameters& getParameters() const;
#else
    const scaling::parameters& getParameters() const;
#endif
    /**
     * returns used parameters
     */
    virtual bool updateParameters();

  protected:
    /**
     * Class containing the real algorithms.
     * This is set in setParameters();
     */
    scalingWorker* worker;
  };
}

#endif


