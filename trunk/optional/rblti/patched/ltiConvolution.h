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
 * file .......: ltiConvolution.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id$
 */

#ifndef _LTI_CONVOLUTION_H_
#define _LTI_CONVOLUTION_H_

#include <vector>
#include "ltiFilter.h"
#include "ltiArray.h"
#include "ltiMatrix.h"
#include "ltiTypes.h"
#include "ltiLinearKernels.h"

namespace lti {
  /**
   * Convolution %functor.
   *
   * This functor convolves a %filter kernel (given in the
   * convolution::parameters) with a vector or matrix.
   *
   * The kernel must be have of the kernel types provided by the library:
   * - lti::kernel1D<T> if you want to filter vectors
   * - lti::kernel2D<T> for non-separable kernels (to filter channels)
   * - lti::sepKernel<T> for separable kernels (to filter channels)
   *
   * @see lti::gaussKernel1D, lti::gaussKernel2D, lti::gaborKernel
   *
   * Even if the setKernel() method accepts lti::mathObject objects, only
   * the previous ones are accepted.
   *
   * In the normal case, the type of the filter kernel has to be the
   * same as the type of the channel (or matrix) to be filter.  For
   * example, if you want to filter a channel (of floats) you will
   * require a kernel of floats.
   *
   * If you try to use different types for the kernel and the matrix, this
   * functor will try to cast the kernel to the proper type first (and this
   * will take some time).
   *
   * For the convolution of kernels and matrices (or channels) of
   * fixed point types (e.g. channel8), you must make use of the norm-term in
   * the kernel. (see kernel1D<T>::norm).
   *
   * @ingroup gLinearFilters
   *
   * Example using a gaussian kernel
   *
   * \code
   * // the channel to be filtered:
   * lti::channel data,result;
   *
   * // ... initialize channel here ...
   *
   * // gauss filter kernel with dimensions 5x5, and a variance of 1.3
   * lti::gaussKernel2D<lti::channel::value_type> kernel(5,1.3);
   *
   * lti::convolution filter;                        // convolution operator
   * lti::convolution::parameters param;             // parameters
   * param.setKernel(kernel);                        // use the gauss kernel
   * filter.setParameters(param);                    // use given parameters
   *
   * // filter the channel and leave the result there too
   * filter.apply(data);
   * \endcode
   *
   * You can also create the functor with a given filter kernel:
   *
   * \code
   * lti::convolution filter(lti::gaussKernel2D<lti::channel::value_type>(5,1.3);
   * filter.apply(data); // parameters already set in the constructor!
   * \endcode
   *
   * The filter kernel can also be change, changing the parameters or with
   * the shortcut setKernel():
   *
   * \code
   * param.setKernel(anotherKernel);
   * filter.setParamters(param);
   *
   * // this is equivalent to:
   *
   * filter.setKernel(anotherKernel);
   *
   * \endcode 
   */
  class convolution : public filter {
  public:
    /**
     * parameters of the lti::convolution functor
     */
    class parameters : public filter::parameters {
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
      virtual ~parameters();

      /**
       * returns name of this type
       */
      const char* getTypeName() const;

      /**
       * copy the contents of other parameter object
       * @param other the parameters object to be copied
       * @return a reference to this object
       */
      parameters& copy(const parameters& other);

      /**
       * returns a pointer to a clone of this parameters
       */
      virtual functor::parameters* clone() const;

      /**
       * returns the kernel in use.  If it is not set yet, an
       * lti::invalidParameters exception will be thrown
       * @return a const reference to the filter kernel.
       */
      const mathObject& getKernel() const;

      /**
       * sets the filter kernel to be used.
       * A copy of the given %parameter will be made!
       * @param aKernel the filter kernel to be used
       */
      void setKernel(const mathObject& aKernel);

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool& complete=true) const;

      /**
       * write the parameters in the given ioHandler
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
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead!
       */
      bool readMS(ioHandler& handler,const bool& complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead!
       */
      bool writeMS(ioHandler& handler,const bool& complete=true) const;
#     endif

    protected:
      /**
       * pointer to the filter kernel copy
       */
      mathObject* kernel;
    };

    /**
     * default constructor
     */
    convolution();

    /**
     * default constructor with parameters
     */
    convolution(const parameters& par);

    /**
     * construct a convolution functor with a parameters set
     * which includes the given filter kernel.
     *
     * @param aKernel kernel object with which you want to convolve.
     * @param boundary Boundary assumption (Zero, Mirror, Periodic, Constant or
     *                 NoBoundary).  @see eBoundaryType
     */
    convolution(const mathObject& aKernel,
                const eBoundaryType& boundary = Zero);

    /**
     * copy constructor
     * @param other the other functor to be copied
     */
    convolution(const convolution& other);

    /**
     * destructor
     */
    virtual ~convolution();

    /**
     * returns the name of this type
     */
    virtual const char* getTypeName() const;

    /**
     * operates on the given %parameter.
     * @param srcdest channel8 with the source data.  The result
     *                will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(channel8& srcdest) const;

    /**
     * operates on the given %parameter.
     * @param srcdest channel with the source data.  The result
     *                will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(channel& srcdest) const;

    /**
     * operates on the given %parameter.
     * @param srcdest dmatrix with the source data.  The result
     *                will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(dmatrix& srcdest) const;


    /**
     * operates on the given %parameter.
     * @param srcdest vector<channel8::value_type> with the source data.
     *                The result will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(vector<channel8::value_type>& srcdest) const;

    /**
     * operates on the given %parameter.
     * @param srcdest vector<channel::value_type> with the source data.
     *                The result will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(vector<channel::value_type>& srcdest) const;

    /**
     * operates on the given %parameter.
     * @param srcdest dvector with the source data.
     *                The result will be left here too.
     * @return true if successful, false otherwise.
     */
    bool apply(dvector& srcdest) const;


    /**
     * operates on a copy of the given parameters.
     * @param src channel8 with the source data.
     * @param dest channel8 where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const channel8& src,channel8& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src channel with the source data.
     * @param dest channel where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const channel& src,channel& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src dmatrix with the source data.
     * @param dest dmatrix where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const dmatrix& src,dmatrix& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src vector<channel8::value_type> with the source data.
     * @param dest vector<channel8::value_type> where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const vector<channel8::value_type>& src,
                     vector<channel8::value_type>& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src vector<channel::value_type> with the source data.
     * @param dest vector<channel::value_type> where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const vector<channel::value_type>& src,
                     vector<channel::value_type>& dest) const;

    /**
     * operates on a copy of the given parameters.
     * @param src dvector with the source data.
     * @param dest dvector where the result will be left.
     * @return true if successful, false otherwise.
     */
    bool apply(const dvector& src,
                     dvector& dest) const;

    /**
     * Split the given color image in its RGB components, filter each of them
     * (as lti::channel) and merge the results.
     * @param src image to be filtered.
     * @param dest resulting filtered image.
     * @return true if successful, false otherwise
     */
    bool apply(const image& src,
                     image& dest) const;

    /**
     * Split the given color image in its RGB components, filter each of them
     * (as lti::channel) and merge the results.
     * @param srcdest image to be filtered.  The result is left here too.
     * @return true if successful, false otherwise
     */
    bool apply(image& srcdest) const;

    /**
     * copy data of "other" functor.
     */
    convolution& copy(const convolution& other);

    /**
     * returns a pointer to a clone of the functor.
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
#ifndef SWIG
    /**
     * This is the accumulator class needed by the convolution helper to
     * act as a linear convolution operator for gray valued images.
     *
     * The type T is the type of the elements of the object to be filtered
     * The (optional) type U is the type of the accumulator variable for
     * the filter.
     */
    template<class T,class U=T>
    class accumulator {
    public:
      /**
       * Default constructor
       */
      accumulator();

      /**
       * Accumulate the values of filter and src
       */
      inline void accumulate(const T& filter,const T& src);

      /**
       * Get the state of the accumulator
       */
      inline T getResult() const;

      /**
       * Reset the state of the accumulator
       */
      inline void reset();

      /**
       * set norm
       */
      inline void setNorm(const T& norm);

    protected:
      /**
       * the accumulated value
       */
      U state;

      /**
       * norm
       */
      T norm;
    };
#endif


  };
}

#include "ltiConvolution_template.h"

#endif

