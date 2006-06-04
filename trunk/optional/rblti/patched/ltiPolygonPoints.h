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
 *
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiPolygonPoints.h
 * classes ....: lti::PolygonPoints
 * description.: class for description of an objects contour
 *               as a linear polygon
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 06.12.2000
 * revisions ..: $Id$
 */

#ifndef _LTIPOLYGONPOINTS_H
#define _LTIPOLYGONPOINTS_H

#include <list>
#include "ltiMath.h"
#include "ltiTypes.h"
#include "ltiPointList.h"

namespace lti {

  //declared in ltiContour.h
  class borderPoints;
  class ioPoints;

  /**
   * Contour classes: polygonPoints.
   *
   * For the explanation of the contour description in this class, see
   * following image:
   *
   * \code
   *   -- 00000000001111111111222222222233
   *   -- 01234567890123456789012345678901
   *   00 --------------------------------
   *   01 --------------------------------
   *   02 --------------------------------
   *   03 --------Xooo-------Soo----------
   *   04 -------o++++Xoo----E++o---------
   *   05 -------o+++++++X---o+++X--------
   *   06 ------X+++++++o-----o+o---------
   *   07 -------o+++++++XoooX++o---------
   *   08 ---------X++++++++++++X---------
   *   09 --------o+++++++++++++++o-------
   *   10 --------o+++++++++++++++o-------
   *   11 -------X++++++++++++++Xo--------
   *   12 ------o++++++++++++++o----------
   *   13 ------o++++++++++++++oX---------
   *   14 -----X++++++++++++++++++o-------
   *   15 -----o+++++++++++++++++++o------
   *   16 ----o+++++++++++++++++++Xo------
   *   17 ---X++++++++++++++++++oo--------
   *   18 ----ooooooooX++++++++X----------
   *   19 -------------oooooXoo-----------
   *   20 --------------------------------
   *   21 --------------------------------
   *   22 --------------------------------
   *   23 --------------------------------
   * \endcode
   *
   *  "-" means the background, "+" represents the inner part of the object,
   *  "o" indicates a borderPoint,  "X" is a polygonPoint, the
   *  borderPoint "S" is start point of the polygonPointList and "E" the end
   *  point of the list.
   *
   *  This class cast given borderPoints into a polygon.  The polygon
   *  has usually less points than the borderPoints, and thus it saves
   *  memory. The disadvantage is that you loose details of the
   *  object when you do the cast. Two parameters minLength and
   *  maxDistance control the number of points of the resulting
   *  polygon.
   *
   *  Between two points in the polygon there must be more than
   *  minLength points of the borderPoints, and the polygon must be
   *  closer to the borderPoints as maxDistance.
   *
   *  It is also possible to get the convex hull of a set of points
   *  (given in a pointList or in an ioPoints list) calling the respective
   *  castFrom() methods.
   *
   *  @see lti::borderPoints, lti::ioPoints, lti::pointList
   *
   *  @ingroup gAggregate
   *  @ingroup gShape
   */
  template<class T>
  class tpolygonPoints : public tpointList<T> {
  public:
    /**
     * default constructor creates an empty border-point-list
     */
    tpolygonPoints<T>();

    /**
     * create this pointList as a copy of another pointList
     * @param other the pointList to be copied.
     */
    tpolygonPoints(const tpolygonPoints<T>& other);

    /**
     * destructor
     */
    virtual ~tpolygonPoints();

    /**
     * returns the name of this class: "polygonPoints"
     */
    const char* getTypeName() const {return "tpolygonPoints";};

    /**
     * alias for approximate()
     */
    tpolygonPoints<T>& castFrom(const borderPoints& theBorderPoints,
                                const int minLength=-1,
                                const double maxDistance=1.0,
                                const bool closed=true,
                                const bool searchMaxDist=false);

    /**
     * creates the smallest convex polygon that contains all points in
     * the given io-points list.  (see computeConvexHull()).
     *
     * This method eliminates the points present twice in the list before
     * computing the convex hull.
     */
    tpolygonPoints<T>& castFrom(const ioPoints& thePointList);

    /**
     * This is an alias for computeConvexHull()
     */
    tpolygonPoints<T>& castFrom(const tpointList<T>& thePointList);

    /**
     * assigment operator (alias for copy(other)).
     * @param other the pointList to be copied
     * @return a reference to the actual pointList
     */
    inline tpolygonPoints<T>& operator=(const tpolygonPoints<T>& other) {
      copy(other);
      return *this;
    };

    /**
     * Compute the 2 x area  of this polygon.
     * 
     * The reason to return twice the area and not the area itself is very
     * simple:  For integer point types, twice the area is always an integer 
     * value, but the area itself is not. 
     *
     * If the area is positive, the polygon has a clockwise direction.
     * Otherwise it will be negative.
     */
    T areaX2() const;

    /**
     * Compute if this polygon has a clockwise direction or not.
     */
    bool clockwise() const;

    /**
     * Creates the smallest convex polygon that contains all points
     * in the given point list.
     *
     * The list of points \b must be a set, i.e. the same point is not
     * allowed to be twice in the list.  (This usually is not the case
     * in lti::ioPoints lists, see castFrom(const ioPoints&)).
     *
     * For more information on the algorithm used here see:
     *
     * M. de Berg, et.al. Computational Geometry, Algorithms and Applications.
     * 2nd. edition, Springer, 2000, pp. 6ff
     *
     * @param thePointList a set of points (the same point ist not allowed to
     *                     be twice in the list).
     * @return a reference to this object.
     */
    tpolygonPoints<T>& convexHullFrom(const tpointList<T>& thePointList);

    /**
     * Approximate the given border points as a polygon.
     *
     * At the end of the approximation process this polygon-points-list
     * contains the coordinates of the vertices.
     *
     * @param theBorderPoints list with the border points to be approximated
     *                        with a polygon.  The elements of this list must
     *                        be adjacent.
     * @param minStep minimal "distance" between vertices of the polygon.
     *                  (if 0 or negative, only the maxDistance parameter
     *                   will be considered).
     *                  The "distance" used here is \b NOT the
     *                  euclidean distance between the vertices but
     *                  the number of elements between both vertices
     *                  in the border list.  These is usually
     *                  therefore a city-block distance or a
     *                  8-neighborhood distance depending on the
     *                  border points description used.
     * @param maxDistance specify the maximal allowed \b euclidean distance
     *                  between the border points and the approximated polygon.
     *                  (if negative, each "minLength" pixel of the
     *                    border points will be taken).
     * @param closed if true (default), only the found vertices will be
     *                    in the list.  If false,  the last point of the list
     *                    will be adjacent to the first point in the list.
     * @param searchMaxDist if true, the first two vertices will be computed
     *                    as the two points in the border with the maximal
     *                    distance.  If false, the first point in the
     *                    list will be a vertex and the border point with
     *                    the maximal distance to it too.  This faster
     *                    method is the default.  It provides usually
     *                    good results.
     *
     * @return a reference to this polygon-points instance.
     */
    tpolygonPoints<T>& approximate(const borderPoints& theBorderPoints,
                                   const int minStep=-1,
                                   const double maxDistance=1,
                                   const bool closed=false,
                                   const bool searchMaxDist=false);

    /**
     * Approximate the given border points as a polygon, but ensure
     * that the given points are vertices.  The given list of points
     * \b must fulfill following two conditions:
     * -# It must have the same sequence than theBorderPoints.
     * -# The points must be contained by the theBorderPoints
     *
     * If one of these condition is not met, the algorithm will take too much
     * time trying to figure out what happend and to recover.  
     *
     * The first condition ensures an efficient search for the
     * vertices in the border points list.
     *
     * At the end of the approximation process this polygon-points-list
     * contains the coordinates of the vertices.
     *
     * @param theBorderPoints list with the border points to be approximated
     *                        with a polygon.  The elements of this list must
     *                        be adjacent.
     * @param forcedVertices list of vertices that must be included in
     *                        the polygon representation.  This list must be a 
     *                        subset and respect  the same ordering as in
     *                        theBorderPoints
     * @param minStep minimal "distance" between vertices of the polygon.
     *                  (if 0 or negative, only the maxDistance parameter
     *                   will be considered).
     *                  The "distance" used here is \b NOT the
     *                  euclidean distance between the vertices but
     *                  the number of elements between both vertices
     *                  in the border list.  These is usually
     *                  therefore a city-block distance or a
     *                  8-neighborhood distance depending on the
     *                  border points description used.
     * @param maxDistance specify the maximal allowed \b euclidean distance
     *                  between the border points and the approximated polygon.
     *                  (if negative, each "minLength" pixel of the
     *                    border points will be taken).
     * @param closed if true (default), only the found vertices will be
     *                    in the list.  If false,  the last point of the list
     *                    will be adjacent to the first point in the list.
     * @param searchMaxDist if true, the first two vertices will be computed
     *                    as the two points in the border with the maximal
     *                    distance.  If false, the first point in the
     *                    list will be a vertex and the border point with
     *                    the maximal distance to it too.  This faster
     *                    method is the default.  It provides usually
     *                    good results.
     * @return a reference to this polygon-points instance.
     */
    tpolygonPoints<T>& approximate(const borderPoints& theBorderPoints,
                                   const pointList& forcedVertices,
                                   const int minStep=-1,
                                   const double maxDistance=1,
                                   const bool closed=false,
                                   const bool searchMaxDist=false);

    /**
     * invert the direction of the polygon points
     */
    void invert();

    /**
     * Combine two polygons.  The result is the union of the points of
     * both polygons.  If both given polygons do not overlap, the result
     * will be the first polygon.
     *
     * @param polygon1 first polygon
     * @param polygon2 second polygon
     * @param minLength minimal distance between vertices of the polygon.
     * @param maxDistance specify the maximal allowed distance between the
     *                    border points and the approximated polygon.
     *                    If set to 0, every minLength points will be
     *                    taken as vertice of the polygon.
     * @return a reference to this object
     *
     */
    tpolygonPoints<T>& clipPoly(const tpolygonPoints<T>& polygon1,
                                const tpolygonPoints<T>& polygon2,
                                const int& minLength = 4,
                                const double& maxDistance = 0.8);

  protected:
    /**
     * used in the approximation of a border points with a polygon.
     * Implements the Ramer (or Duda Hart) method.  See also
     * Haralick and Shapiro.  Computer and Robot Vision vol. 1 Addison Wesley,
     * 1992.  pp. 563ff
     */
    void fitAndSplit(const vector< point >& vct,
                     const int idx1,
                     const int idx2,
                     const double maxDistance,
                     vector<ubyte>& flags);
  };

#ifdef SWIG
%template(ipolygonPoints) lti::tpolygonPoints<int>;
#endif
  
  /**
   * Polygon Points of Integer Points
   */
  class polygonPoints : public tpolygonPoints<int> {
  public:
    polygonPoints() : tpolygonPoints<int>() {};
    polygonPoints(const polygonPoints& other)
      : tpolygonPoints<int>(other) {};
  };

}//end namespace lti
#endif

