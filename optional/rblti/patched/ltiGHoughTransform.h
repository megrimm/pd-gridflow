/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
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
 *
 */

/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiHoughTransform.h
 * authors ....: Jens Rietzschel
 * organization: LTI, RWTH Aachen
 * creation ...: 22.05.2001
 */


#ifndef _LTI_GND_HOUGH_TRANSFORM_H_
#define _LTI_GND_HOUGH_TRANSFORM_H_

#include "ltiImage.h"
#include "ltiModifier.h"
#include "ltiPointList.h"
#include "ltiTensor.h"
#include <list>
#include <map>

namespace lti {
  /**
   * \brief The Generalized-Hough-Transform
   *
   * The Generalized-Hough-Transform detects arbitrary shapes, which
   * have to be specified before (by use()).  It transforms the channel
   * into a four dimensional parameter space (discretisation can be
   * set in parameters).  Therefore it requires a lot of storage and
   * extensive computation.  Peaks in parameter-space are searched and
   * returned as result.  The found object can be drawn back into the
   * image by draw().
   *
   * @see lti::orientedHLTransform
   */
  class gHoughTransform : public modifier {
  public:
    /**
     * the parameters for the class houghTransform
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

      /**
       * Local maxima of parameter-space with values above
       * thresholdLocalMaximum are considered as found objects in
       * image-space.
       *
       * Values depend on number of pixels in image. Higher values
       * fasten the search for maxima.
       *
       * Default:10
       */
      float thresholdLocalMaximum;


      /**
       * kernelSize is size of environment which is considered in
       * search for local maxima at each position in parameter-space
       * for row and column.
       *
       * Default: 11
       */
      int kernelSizeTranslation;

      /**
       * kernelSizeRotation is size of environment which is considered
       * in search for local maxima at each position in parameter-space
       * for rotation.
       *
       * Default: 21
       */
      int kernelSizeRotation;

      /**
       * kernelSizeSize is size of environment which is considered in
       * search for local maxima at each position in parameter-space
       * for size
       *
       * Default: 11
       */
      int kernelSizeScale;

      /**
       * number of Objects to be found (default 1)
       */
      int numberOfObjects;

      /**
       * Values above thresholdEdgePixel are considered as pixel of edges and
       * transformed into parameter space.
       *
       * Best to use previously an edge detector e.g. susanEdges.
       *
       * default 0.9
       */
      float thresholdEdgePixel;

      /**
       * true:  search for local maxima (is very computational intensive)
       * false: search for global maxima (is faster and default)
       */

      bool findLocalMaximum;

      /**
       * size of third dimension of parameter-space
       * discretisation of parameter size
       */
      int disSize;

      /**
       * size of forth dimension of parameter-space
       * discretisation of paramter rotation
       */
      int disRotation;

      /**
       * range of object-extension which is considered
       */
      float extension;

      /**
       * Number of lists regarded for each pixel symmetrical to its
       * orientation angle phi. High values gain better result but more
       * computational intensive.
       */
      int numberOfAngleLists;

    };

    /**
     * default constructor
     */
    gHoughTransform();

    /**
     * copy constructor
     * @param other the object to be copied
     */
    gHoughTransform(const gHoughTransform& other);

    /**
     * destructor
     */
    virtual ~gHoughTransform();

    /**
     * returns the name of this type ("houghTransform")
     */
    virtual const char* getTypeName() const;


    /**
     * @param src channel of object which is to be found later on
     */
    bool use (const channel& src);

    /**
     * @param src tpointList of object which is to be found later on
     */
    bool use (const tpointList<int>& src);



    /**
     * @param src: find object specified by use in channel src
     * @param dest: list containing vectors, that each represent
     *              a found object by five entries: shift_x,
     *              shift_y,size factor,rotation,relevance
     */
    bool apply(const channel& src,std::list<lti::vector<float> >& dest);

    /**
     * @param sample: channel of object which is to be found later on
     * @param src: find object specified by sample in channel src
     * @param dest: list containing vectors, that each represent
     *              a found object by five entries: shift_x,
     *              shift_y,size factor,rotation,relevance
     */
    bool apply(const channel& sample,const channel& src,
               std::list<lti::vector<float> >& dest);

    /**
     * @param contour: tpointList of object which is to be found later on
     * @param src: find object specified by contour in channel src
     * @param dest: list containing vectors, that each represent
     *              a found object by five entries: shift_x,
     *              shift_y,size factor,rotation,relevance
     */
    bool apply(const tpointList<int>& contour,const channel& src,
               std::list<lti::vector<float> >& dest);

    /**
     * @param src: list containing vectors, that each represent
     *             a found object by five entries: shift_x,
     *             shift_y,size factor,rotation,relevance
     *             and is computed by apply-method
     * @param dest: draw into this image
     * @param color: use this color for drawing
     */
    bool draw(const std::list<lti::vector<float> >& src,
                    image& dest,
              const rgbPixel& color=rgbPixel(255,0,0));

    /**
     * copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    gHoughTransform& copy(const gHoughTransform& other);

    /**
     * returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;

  private:

    //used to store maxima of parameterspace
    class listEntry{
    public:
      int row;
      int column;
      int sizeIndex;
      int rotationIndex;
      float value;
      bool operator<(const listEntry& a);
    };


    // used within findMaximum4D to check minimum distances according
    // to kernelsize
    bool distance_remove(std::list<listEntry>& src,int& n);

    //searches for maxima in parametersspace (global/local)
    bool findMaximum4D(tensor<float>& accu,int rows,int columns,int disSize,
                       int disRotation, std::list<listEntry>& dest);

    //data structure used for representation of object to find (map)
    class referenceTable{
    public:
      referenceTable();
      referenceTable(const channel& src,float threshold,int disRTable);
      referenceTable(const tpointList<int>& src,int disRTable);
      bool push (float alpha,float r,double phi);
      bool getAlphaRList(double phi,double rotation,tpointList<float>& dest);
      bool buildReferenceTable(const channel& src,
                                     float threshold,
                                     int disRTable);
      bool buildReferenceTable(const tpointList<int>& src,int disRTable);

      std::map<int,tpointList<float> > refTable;
      tpointList<int> allPoints;
      tpoint<int> anchor;
    private:
      int numberOfAngleLists;
    };

    // member referenceTable
    referenceTable myReferenceTable;

    // true if initialised by use(sample) or apply(sample)
    bool isSet;

  };
}
#endif

