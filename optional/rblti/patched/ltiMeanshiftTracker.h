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
 * file .......: ltiMeanshiftTracker.h
 * authors ....: Torsten K�per
 * organization: LTI, RWTH Aachen
 * creation ...: 24.9.2001
 * revisions ..: $Id$
 */

#ifndef _LTI_MEANSHIFT_TRACKER_H_
#define _LTI_MEANSHIFT_TRACKER_H_

// include files which are needed in this header
#include "ltiHistogram.h"

// include parent class:

#include "ltiModifier.h"
namespace lti {
  /**
   * This is the implementation of the MEANSHIFT
   * Tracking algorithm as described in:
   * Comaniciu, Ramesh and Meer, "Real-Time Tracking of
   * Non-Rigid Objects using the Mean Shift", IEEE Workshop
   * on Applic.Comp.Vis.,2000.
   *
   * It tracks a rectangular search window (the target)
   * in an image by its color distribution. Therefore
   * it uses an iterative gradient ascent algorithm, known as
   * mean shift, that finds a similarity peak between target
   * and candidate positions. The similarity is measured with
   * a distance metric, which can be obtained after each apply().
   *
   * Use this by calling an apply on subsequent images and thereby
   * passing the last returned search rectangle. Initialization is
   * done with first use or after a call of the reset() method.
   */
  class meanshiftTracker : public modifier {
  
  #ifdef SWIG
  #ifdef SWIGRUBY
  %rename(init) initialize (const image &, trectangle< int > &);
  %rename(init) initialize (const image &, trectangle< int > &, const channel8 &);
  %rename(init) initialize (const image &, trectangle< int > &, const channel &);  
  #endif
  #endif

  public:
    /**
     * the parameters for the class meanshiftTracker
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
	     * Kernel Type.
       */
      enum eKernelType {
        Normal,           /**< Normal Kernel (default)*/
        Epanechnikov      /**< Epanechnikov Kernel */
      };


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
      virtual bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * read the parameters from the given ioHandler
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
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

      // ------------------------------------------------
      // the parameters
      // ------------------------------------------------

      // If you add more parameters manually, do not forget to do following:
      // 1. indicate in the default constructor the default values
      // 2. make sure that the copy member also copy your new parameters
      // 3. make sure that the read and write members also read and
      //    write your parameters

      /**
       * precision for iterations:
       * 0.1 for deep iteration , 10.0 for fast runtime
       * default is 2. allowed range is [0.1 ... ]
       */
      float precision;

      /**
       * Threshold for tracker validity. Range is from 0.0 to 1.0.
       * If color distribution distance after apply is below this
       * threshold, then the tracker simply doesn't react. Default is 0.8
       */
      float threshold;

      /**
       * Kerneltypes are Normal or Epanechnikov-Kernel
       * Default is Normal
       */
      eKernelType kernelType;

      /**
       * The size adaption ratio tells how strong the size adaptation
       * may be with one call of an apply() method.
       * Range is 0.0 to 1.0 (reasonable values lie between 0.0 and 0.5).
       * Default is 0.1
       */
      float sizeAdaptRatio;
    };

    /**
     * default constructor
     */
    meanshiftTracker();

    /**
     * copy constructor
     * @param other the object to be copied
     */
    meanshiftTracker(const meanshiftTracker& other);

    /**
     * destructor
     */
    virtual ~meanshiftTracker();

    /**
     * returns the name of this type ("meanshiftTracker")
     */
    virtual const char* getTypeName() const;

    /**
     * Tracks a target color distribution. The initial target is
     * specified by "window" on first usage or after a reset()
     * (i.e. tracker is not initialized).
     * Subsequent calls of this method perform tracking by translating
     * and scaling the "window" to optimally fit the target color
     * distribution.
     */
    bool apply(const image& src,trectangle<int>& window);

    /**
     * Tells, if the tracker has already been initialized with a
     * target color distribution.
     */
    bool isInitialized() const;

    /**
     * Explicitly initialize tracker with given region in image.
     * This is called automatically when an apply is used while
     * tracker is not initialised. I.e. calling initialize() is
     * the same as subsequently calling reset() and apply().
     */
    void initialize(const image& src,trectangle<int>& window);

    /**
     * Explicitly initialize tracker with given region in image.
     * Additionally weight all pixels with the value in mask.
     * You must call this explicitly, if you want to initialize
     * with a weighting mask.
     */
    void initialize(const image& src,trectangle<int>& window, const channel8& mask);

    /**
     * Explicitly initialize tracker with given region in image.
     * Additionally weight all pixels with the value in mask.
     * You must call this explicitly, if you want to initialize
     * with a weighting mask.
     */
    void initialize(const image& src,trectangle<int>& window, const channel& mask);

    /**
     * Resets the tracker. Next call of apply initializes it again.
     */
    void reset();

    /**
     * Tells if returned window is valid. This depends on the threshold
     * parameter. If the color distribution distance between target and
     * candidate window is above threshold, then this method returns
     * false.
     */
    bool isValid() const;

    /**
     * returns the distance between the color distribution of the
     * initial target and the last returned window.
     */
    float getDistance() const;

    /**
     * returns the center of the tracked candidate distribution.
     * In general, this is the returned search windows center,
     * except that the winwod is at the fringe of the image.
     */
    tpoint<float> getCenter() const;

    /**
     * returns a const reference to the histogram of the target.
     * The histogram only changes on initialization.
     */
    const thistogram<float>& getTargetHist() const;

    /**
     * returns a const reference to the histogram of the tracked candidate.
     * The histogram changes after each call of an apply method.
     */
    const thistogram<float>& getCandidateHist() const;

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    meanshiftTracker& copy(const meanshiftTracker& other);

    /**
     * alias for copy member
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    meanshiftTracker& operator=(const meanshiftTracker& other);

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

   private:
    /**
     * Struct for storing the tracker data after call of any apply method
     */
    class trackerState {
      public:
        trackerState();
        ~trackerState();

        // methods
        void clear();
        trackerState& copy(const trackerState& other);
        trackerState& operator=(const trackerState& other);

        // final center position y1 (after iterations)
        tpoint<float> y1;
        // final Bhattacharyya coefficient for position y1
        float bhat1;
        // target color probability
        thistogram<float> targetProb;
        // candidate color probability for pos y1
        thistogram<float> candProb;
        // distance and height to width ratio of tracking box
        float distance, hwRatio;
    };

    /**
     * calculates the bhattacharyya metric for measuring
     * similarity of two color distributions
     */
    float calcBhatCoef(const thistogram<float>& targetProb,
                       const thistogram<float>& candProb) const;

    /**
     * fills the histogram "prob" with the colors inside the
     * rectangle "canvas" in the image. Each color is weighted
     * with a kernel function. Note: the histogramm is not
     * normalized to 1, thus use prob.getProbability() to obtain
     * probabilities.
     */
    void calcProb(thistogram<float>& prob,
                  const trectangle<int>& canvas,
                  const image& src) const;

    /**
     * fills the histogram "prob" with the colors inside the
     * rectangle "canvas" in the image. Each color is weighted
     * with a kernel function AND the weight given in mask.
     * Note: the histogramm is not normalized to 1, thus use
     * prob.getProbability() to obtain probabilities.
     */
    void calcProb(thistogram<float>& prob,
                  const trectangle<int>& canvas,
                  const image& src,
                  const channel8& mask) const;

     /**
     * fills the histogram "prob" with the colors inside the
     * rectangle "canvas" in the image. Each color is weighted
     * with a kernel function AND the weight given in mask.
     * Note: the histogramm is not normalized to 1, thus use
     * prob.getProbability() to obtain probabilities.
     */
    void calcProb(thistogram<float>& prob,
                  const trectangle<int>& canvas,
                  const image& src,
                  const channel& mask) const;


    /**
     * returns the Normal or Epanechikov kernel for the given position
     */
    float kernel(const tpoint<int>& cen,
                 const tpoint<int>& pos,
                 const tpoint<int>& h) const;

    /**
     * calculates the color distribution distance between two histogramms
     */
    float calcDistance(const thistogram<float>& targetProb,
                       const thistogram<float>& candProb) const;

    /**
     * shifts and clips rect to fit into given canvas
     */
    void correctRect(trectangle<int>& rect,
                     const trectangle<int>& canvas) const;

    /**
     * returns the derivate of the Epanechikov kernel
     */
    float derivateKernel(const tpoint<int>& cen,
                         const tpoint<int>& pos,
                         const tpoint<int>& h) const;

    /**
     * stores results of last call of one apply method (the state of the tracker)
     */
    trackerState td;

    /**
     * indicates if the tracker is initialized
     */
    bool initialized;

    /**
     * indicates if the last apply was a valid tracking step
     */
    bool valid;

  };
}

#endif

