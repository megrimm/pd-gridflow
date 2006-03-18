/*
 * Copyright (C) 2003 Vlad Popovici, EPFL STI-ITS, Switzerland
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
 * file .......: ltiRotation.h
 * authors ....: Vlad Popovici
 * organization: EPFL STI-ITS/LTS1
 * creation ...: 18.6.2003
 * revisions ..: $Id$
 */

#ifndef _LTI_ROTATION_H_
#define _LTI_ROTATION_H_

// For a description and a more pedagogical implementation of the
// rotation algorithm, see http://www.leptonica.com/index.html

#include "ltiModifier.h"

namespace lti {

  /**
   * Rotation implements a rotation functor.
   *
   * The algorithm used is call Rotation by Shear, nicely
   * reviewed at http://www.leptonica.com/rotation.html#ROTATION-BY-SHEAR
   *
   * The size of the computed image will always be enhanced to contain
   * the whole (rotated) image.  In other words, you will get the same
   * result as using lti::geometricTransform using the parameter
   * \c keepDimensions set to false. 
   *
   * This functor does always a bilinear interpolation, but supports only the
   * boundary type "Zero", to avoid some comparisons and increase efficiency.
   *
   * \warning The speed gains of this functor compared with
   * lti::geometricTransform are marginal or non-existent, i.e. this functor
   * algorithm can be even slower.  The functor exists to provide an
   * interface, that can wrap a more efficient algorithm in the future.
   *
   * For more complex operations you can still use the lti::geometricTransform 
   * functor.
   *
   * @see lti::geometricTransform
   *
   * @ingroup gGeometry
   */
  class rotation : public modifier {
  public:
    /**
     * the parameters for the class rotation
     */
    class parameters : public modifier::parameters {
    public:
      /**
       * default constructor
       */
      parameters();

      /**
       * copy constructor
       * @param other the parameters object to be copied
       */
      parameters(const parameters& other);

      /**
       * destructor
       */
      ~parameters();

      /**
       * returns name of this type
       */
      const char* getTypeName() const;

      /**
       * copy the contents of a parameters object
       * @param other the parameters object to be copied
       * @return a reference to this parameters object
       */
      parameters& copy(const parameters& other);

      /**
       * copy the contents of a parameters object
       * @param other the parameters object to be copied
       * @return a reference to this parameters object
       */
      parameters& operator=(const parameters& other);


      /**
       * returns a pointer to a clone of the parameters
       */
      virtual functor::parameters* clone() const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool& complete=true) const;

      /**
       * read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool& complete=true);

#     ifdef _LTI_MSC_VER
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool& complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
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
       * Rotation angle.  
       *
       * In radians.  You can use the global function lti::degToRad() to
       * convert a degrees value to radians.
       *
       * As usual in the LTI-Lib, the coordinate system of image is
       * left-handed, (y=0 means the top of the image, y>0 goes down).  An
       * increasing positive angle denotes in such a coordinate system a
       * clockwise rotation.
       *
       * Default value: 0
       */
      double angle;

    };

    /**
     * default constructor
     */
    rotation();

    /**
     * Construct a functor using the given parameters
     */
    rotation(const parameters& par);

    /**
     * copy constructor
     * @param other the object to be copied
     */
    rotation(const rotation& other);

    /**
     * destructor
     */
    virtual ~rotation();

    /**
     * returns the name of this type ("rotation")
     */
    virtual const char* getTypeName() const;
    
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
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param srcdest image with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,image& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param src image with the source data.
     * @param dest image where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,const image& src,image& dest) const;


    /**
     * Operates on the given %parameter.
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param srcdest matrix<ubyte> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,matrix<ubyte>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param src matrix<ubyte> with the source data.
     * @param dest matrix<ubyte> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,
               const matrix<ubyte>& src,
               matrix<ubyte>& dest) const;

    /**
     * Operates on the given %parameter.
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param srcdest matrix<float> with the source data.  The result
     *                 will be left here too.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,
               matrix<float>& srcdest) const;

    /**
     * Operates on a copy of the given %parameters.
     * @param angle rotation angle used instead of the value in the parameters
     *              object, which is ignored.
     * @param src matrix<float> with the source data.
     * @param dest matrix<float> where the result will be left.
     * @return true if apply successful or false otherwise.
     */
    bool apply(const double& angle,
               const matrix<float>& src,matrix<float>& dest) const;

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    rotation& copy(const rotation& other);

    /**
     * alias for copy member
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    rotation& operator=(const rotation& other);

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * returns used parameters
     */
#ifdef SWIG
    const parameters& getParameters() const;
#else
    const rotation::parameters& getParameters() const;
#endif
  };
}

#endif


