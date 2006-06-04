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
 * file .......: ltiChromaticityHistogram.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.10.2000
 * revisions ..: $Id$
 */

#ifndef _LTI_CHROMATICITY_HISTOGRAM_H_
#define _LTI_CHROMATICITY_HISTOGRAM_H_

#include "ltiContour.h"
#include "ltiObject.h"
#include "ltiMatrix.h"
#include "ltiHistogram.h"
#include "ltiGlobalFeatureExtractor.h"

namespace lti {


  /**
   * Create a chromaticity histrogram feature vector.
   * Due to the fact that chromaticity has no information contents on
   * intensity, this feature is highly illumination invariant.
   * The 2D chromaticity-histogram is a triangular matrix and will be
   * coded in the 1D feature vector to save memory space.
   * @see chromaticityHistogram::parameters
   */
  class chromaticityHistogram : public globalFeatureExtractor {
  public:
    /**
     * the parameters for the class chromaticityHistogram
     */
    class parameters : public globalFeatureExtractor::parameters {
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

      // ------------------------------------------------
      // the parameters
      // ------------------------------------------------

      /**
       * To compensate illumination variations, the norm of a
       * pixel-neighbourhood centered on the analyzed pixel will be
       * calculated.  The following factor will be multiplied by the
       * number of rows of the analyzed image to get the height of the
       * neighbourhood.  */
      float verticalNeighbourhood;

      /**
       * To compensate illumination variations, the norm of a
       * pixel-neighbourhood centered on the analyzed pixel will be
       * calculated.  The following factor will be multiplied by the
       * number of columns of the analyzed image to get the size of the
       * neighbourhood.  */
      float horizontalNeighbourhood;

      /**
       * type used in the norm specification
       */
      enum eNormType {
        NoNorm, /*! Do not normalize */
        L1,     /*!< L1 norm */
        L2      /*!< L2 norm */
      };

      /**
       * Which kind of normalization should be used? (Default L2 Norm)
       */
      eNormType norm;

      /**
       * Dimension of the 1D gaussian kernel which is used to smooth the
       * resulting 1D histogram
       */
      int smoothingKernelSize;

      /**
       * The number of cells in the histogram to store the
       * green-chromaticity values
       */
      int greenCells;

      /**
       * The number of cells in the histogram to store the
       * red-chromaticity values
       */
      int redCells;

      /**
       * The value of the background will be ignored in the histogram
       */
      rgbPixel backgroundColor;
    };

#ifndef SWIG
    /**
     * accumulator class of the squares of the values
     */
    template<class T>
    class l2accumulator {
    public:
      /**
       * default constructor
       */
      l2accumulator() : acc(T()) {};

      /**
       * accumulation operator
       */
      inline const T& operator+=(const T& x) {
        acc+=(x*x); return acc;
      };

      /**
       * deaccumulation operator
       */
      inline const T& operator-=(const T& x) {
        acc-=(x*x); return acc;
      };

      /**
       * division operator
       */
      inline T operator/(const T& y) {
        return ((acc>0) ?  sqrt(acc)/y : 0);
      };

      /**
       * cast operator to type T
       */
      operator T() const {return ((acc>0) ? sqrt(acc) :0) ;};

      /**
       * assignment operator
       */
      const T& operator=(const T& x) {acc=x*x; return (x);};

    protected:
      T acc;
    };
#endif

    /**
     * default constructor
     */
    chromaticityHistogram();

    /**
     * copy constructor
     * @param other the object to be copied
     */
    chromaticityHistogram(const chromaticityHistogram& other);

    /**
     * destructor
     */
    virtual ~chromaticityHistogram();

    /**
     * returns the name of this type ("chromaticityHistogram")
     */
    virtual const char* getTypeName() const;

    /**
     * operates on the given %parameter.
     * @param src image with the source data.
     * @param chrHist the feature vector will be left here
     * @result a reference to the <code>chrHist</code>.
     */
    bool apply(const image& src,dvector& chrHist) const;

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    chromaticityHistogram& copy(const chromaticityHistogram& other);

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;
  };
}

#endif

