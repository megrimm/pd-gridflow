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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiDilation.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.7.2000
 * revisions ..: $Id$
 */

#ifndef _LTI_DILATION_H_
#define _LTI_DILATION_H_

#include "ltiObject.h"
#include "ltiMath.h"
#include "ltiImage.h"
#include "ltiLinearKernels.h"
#include "ltiMorphology.h"
#include "ltiConvolutionHelper.h"

namespace lti {
  /**
   * Dilation functor.
   *
   * This functor implements the morphological operator "dilation".
   * Through the parameters a "binary" or "gray scale" modus can be choosen,
   * and the structuring element (represented by a linear filter kernel) can
   * be given.
   *
   * For mode Binary the destination image is set to the norm value of
   * the kernel used if there is a source element in the kernel region
   * that is not zero and to zero otherwise.
   *
   * The definition for mode Gray can be found in e.g. Gonzalez,
   * R. and Woods, R.  Digital Image Processing, 2nd Edition,
   * pp. 550--553, Prentice Hall, 2002
   *
   * /code
   * dest(s,t) = max(src(s-x, t-y) + kernel(x,y)) 
   * /endcode
   *
   * where the regions of the kernel and source overlap. Qualitatively
   * the Gray operation results in brightening esp. dark details. For
   * channel8 the resulting values are clipped to be in the allowed
   * range of [0,255]. Note that for channels the kernel values should
   * be much lower than the default 1.f. Also note that when the
   * kernel is separable (sepKernel) the values of all column and row
   * kernels are subtracted. An example is chessBoardKernel.
   *
   *
   * Example:
   *
   * \code
   * lti::dilation dilator;                    // the dilation functor
   * lti::dilation::parameters dilationParam;  // the parameters
   *
   * lti::euclideanKernel<float> theKern(3);   // a circular kernel
   *
   * // binary dilation
   * dilationParam.mode = lti::dilation::parameters::Binary;
   * dilationParam.setKernel(theKern);
   *
   * // set the parameters
   * dilator.setParameters(dilationParam);
   *
   * // apply the dilation to a channel "chnlSrc" and leave the result in
   * // "chnlDest"
   *
   * dilator.apply(chnlSrc,chnlDest);
   * \endcode
   *
   * @ingroup gMorphology
   */
  class dilation : public morphology {
  public:
    /**
     * the parameters for the class dilation
     */
    class parameters : public morphology::parameters {
    public:
      /**
       * type to specify what kind of dilation should be applied. See
       * lti::dilation for more information.
       */
      enum eMode {
        Binary, /*!< Binary dilation */
        Gray    /*!< Gray valued dilation */
      };

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
       * returns a pointer to a clone of the parameters
       */
      virtual functor::parameters* clone() const;

      /**
       * sets the filter kernel to be used.
       * A copy of the given parameter will be made!
       * @param aKernel the filter kernel to be used
       */
      void setKernel(const mathObject& aKernel);

      /**
       * returns the kernel in use.  If it is not set yet, an
       * lti::invalidParameters exception will be thrown
       * @return a const reference to the filter kernel.
       */
      const mathObject& getKernel() const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead!
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead!
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

      // ---------------------------------------------------
      // the parameters
      // ---------------------------------------------------

      /**
       * Dilation mode.
       *
       * The dilation is defined for "Binary" images (only two values per
       * pixel) and for gray scaled images (channels or channel8).
       * In "Binary" modus, the channels and channel8s will be interpreted as
       * binary with 0 as "0" and everything else as "1"
       * The default value is "Binary"
       */
      eMode mode;

    protected:
      /**
       * The kernel to be used.  This parameter can only be set through the
       * setKernel member.
       */
      mathObject* kernel;
    };

    /**
     * default constructor
     */
    dilation();

    /**
     * construct  with the given kernel
     */
    dilation(const mathObject& aKernel);

    /**
     * copy constructor
     * @param other the object to be copied
     */
    dilation(const dilation& other);

    /**
     * destructor
     */
    virtual ~dilation();

    /**
     * returns the name of this type ("dilation")
     */
    virtual const char* getTypeName() const;

    /**
     * operates on the given parameter.
     * @param srcdest channel with the source data.  The result
     *                 will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(channel& srcdest) const;

    /**
     * operates on the given parameter.
     * @param srcdest channel8 with the source data.  The result
     *                 will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(channel8& srcdest) const;

    /**
     * operates on the given parameter.
     * @param srcdest fvector with the source data.  The result
     *                 will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(fvector& srcdest) const;

    /**
     * operates on the given parameter.
     * @param srcdest vector<channel8::value_type> with the source data.
     *                The result will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(vector<channel8::value_type>& srcdest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src channel with the source data.
     * @param dest channel where the result will be left.
     * @result true if ok, false otherwise.
     */
    bool apply(const channel& src,channel& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src channel8 with the source data.
     * @param dest channel8 where the result will be left.
     * @result true if ok, false otherwise.
     */
    bool apply(const channel8& src,channel8& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src fvector with the source data.
     * @param dest fvector where the result will be left.
     * @result true if ok, false otherwise.
     */
    bool apply(const fvector& src,fvector& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src vector<channel8::value_type> with the source data.
     * @param dest vector<channel8::value_type> where the result will be left.
     * @result true if ok, false otherwise.
     */
    bool apply(const vector<channel8::value_type>& src,
                     vector<channel8::value_type>& dest) const;

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    dilation& copy(const dilation& other);

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;
    /**
     * shortcut to set the filter kernel in the functor parameters.
     * The other parameters remain unchanged.
     */
    void setKernel(const mathObject& aKernel);

  private:
    /**
     * This is the accumulator class needed by the convolution helper to
     * act as a dilation operator for gray valued images.
     *
     * The type T is the type of the elements of the object to be filtered
     * The (optional) type U is the type of the accumulator variable for
     * the filter.
     */
#ifndef SWIG
    template<class T,class U=T>
    class accumulatorGray {
    public:
      /**
       * Default constructor
       */
      accumulatorGray();

      /**
       * Accumulate the values of filter and src
       */
      inline void accumulate(const T& filter,const T& src);

      /**
       * Accumulate the values of T(0) and src
       */
      inline void accumulateZero(const T& src);

      /**
       * Accumulate the values of filter and srcL and srcR
	   * for symmetric filter kernel
	   * src:				srcL  *  middle  *  srcR
	   * filter:			*  *  *  middle  *  *  *
	   * used filter part:	*  *  *  middle
       */
      inline void accumulateSym(const T& filter,const T& srcL,const T& srcR);

      /**
       * Accumulate the values of filter and src
	   * for asymmetric filter kernel
	   * src:				srcL  *  middle  *  srcR
	   * filter:			*  *  *  middle  *  *  *
	   * used filter part:	*  *  *  middle
       */
      inline void accumulateASym(const T& filter,const T& srcL,const T& srcR);


      /**
       * Get the state of the accumulator
       */
      inline T getResult() const;

      /**
       * Reset the state of the accumulator
       */
      inline void reset();

      /**
       * set norm (unused)
       */
      inline void setNorm(const T& n);

    protected:
      /**
       * the accumulated value
       */
      U state;

      /**
       * the norm
       */
      T norm;
    };

    /**
     * This is the accumulator class needed by the convolution helper to
     * act as a dilation operator for binary valued images.
     *
     * The type T is the type of the elements of the object to be filtered
     * The (optional) type U is the type of the accumulator variable for
     * the filter.
     */
    template<class T,class U=T>
    class accumulatorBin {
    public:
      /**
       * Default constructor
       */
      accumulatorBin();

      /**
       * Accumulate the values of filter and src
       */
      inline void accumulate(const T& filter,const T& src);

      /**
       * Accumulate the values of T(0) and src
       */
      inline void accumulateZero(const T& src);

      /**
       * Accumulate the values of filter and srcL and srcR
	   * for symmetric filter kernel
	   * src:				srcL  *  middle  *  srcR
	   * filter:			*  *  *  middle  *  *  *
	   * used filter part:	*  *  *  middle
       */
      inline void accumulateSym(const T& filter,const T& srcL,const T& srcR);

      /**
       * Accumulate the values of filter and src
	   * for asymmetric filter kernel
	   * src:				srcL  *  middle  *  srcR
	   * filter:			*  *  *  middle  *  *  *
	   * used filter part:	*  *  *  middle
       */
      inline void accumulateASym(const T& filter,const T& srcL,const T& srcR);


      /**
       * Get the state of the accumulator
       */
      inline T getResult() const;

      /**
       * Reset the state of the accumulator
       */
      inline void reset();

      /**
       * set norm (unused)
       */
      inline void setNorm(const T& nrm);

    protected:
      /**
       * the accumulated value
       */
      U state;

      /**
       * the norm
       */
      T norm;
    };
#endif // SWIG
  };
}

#include "ltiDilation_template.h"

#endif

