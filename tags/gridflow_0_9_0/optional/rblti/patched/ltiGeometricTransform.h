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
 * file .......: ltiGeometricTransform.h
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 5.2.2001
 * revisions ..: $Id$
 */

#ifndef _LTI_GEOMETRIC_TRANSFORM_H_
#define _LTI_GEOMETRIC_TRANSFORM_H_

#include "ltiMacroSymbols.h"

#include "ltiMath.h"
#include "ltiMatrix.h"
#include "ltiTypeInfo.h"
#include "ltiHTypes.h"

#include "ltiPointList.h"

#include "ltiModifier.h"

#include "ltiInterpolatorType.h"
#include "ltiBilinearInterpolator.h"
#include "ltiBiquadraticInterpolator.h"
#include "ltiGenericInterpolator.h"
#include "ltiNearestNeighborInterpolator.h"

#include <limits>

namespace lti {

  /**
   * This functor implements an generic geometric transform based on a 
   * homogeneous matrix, which is able to represent rotations, translations
   * scalings and all other possibilites of homographies.
   *
   * You can use your own matrix to generate a special geometric
   * transformation, or you can use the member functions of the parameters
   * class to generate the matrix and vectors for a given translation,
   * rotation and scaling.
   *
   * You can use all the three functions in arbitrary order and number to
   * build your transform matrix sequentially.
   *
   * With the <code>invert()</code> member of the parameters class you can
   * invert the matrix for the reverse transformation. The speed in general
   * only depends on the size of the image or the size of the pointList.
   *
   * Please note that even if the transformation can have a 3D nature (4x4
   * homogeneous matrix) and you can turn, shift or scale the image plane 
   * in any of the three coordinates, the input image is always a 2D object
   * with z-coordinate zero.  A typical error is trying to apply the inverse
   * transformation matrix to reconstruct a 3D rotation: this cannot work
   * because of the missing third coordinate for each pixel.
   *
   * There are also a few specialized functors to achieve isolated
   * transformation tasks in a much faster way.  A derived class also
   * restricts the transformation to a similarity transformation (see
   * lti::similarityTransform2D).
   *
   * @see lti::flipImage, lti::rotation, lti::scaling
   *
   * @ingroup gGeometry
   */
  class geometricTransform : public modifier {
  public:
    /**
     * The parameters for the class geometricTransform
     */
    class parameters : public modifier::parameters {
    public:
      /**
       * Invert the homogeneous transformation matrix.
       */
      void invert();

      /**
       * Set the scaling factor of the homogeneous matrix.  This is achieved
       * multiplying the existing matrix with a resize matrix. If you
       * use this function on an unit matrix you get the following matrix:
       * \f[ \begin{bmatrix} thefpoint.x & 0           & 0 & 0 \\
       *                  0           & thefpoint.y & 0 & 0 \\
       *                  0           & 0           & 1 & 0 \\
       *                  0           & 0           & 0 & 1 \end{bmatrix} \f]
       */
      void scale(const tpoint<double>& thefPoint);

      /**
       * Set the scaling factor of the homogeneous matrix.  This is achieved
       * multiplying the existing matrix with a resize matrix. If you
       * use this function on an unit matrix you get the following matrix:
       * \f[ \begin{bmatrix} thefpoint.x & 0           & 0 & 0 \\
       *                  0           & thefpoint.y & 0 & 0 \\
       *                  0           & 0           & thefpoint.z & 0 \\
       *                  0           & 0           & 0 & 1 \end{bmatrix} \f]
       */
      void scale(const tpoint3D<double>& thefPoint);

      /**
       * Special scale function with only one parameter
       * for x, y and z scale
       * \f[ \begin{bmatrix} x & 0 & 0 & 0 \\
       *                     0 & x & 0 & 0 \\
       *                     0 & 0 & x & 0 \\
       *                     0 & 0 & 0 & 1 \end{bmatrix} \f]
       */
      void scale(const double& x);

      /**
       * This function adds the point to the translation
       * vector.  (z component assumed 0)
       */
      void shift(const tpoint<double>& thefPoint);

      /**
       * This function adds the point to the translation
       * vector.
       */
      void shift(const tpoint3D<double>& thefPoint);

      /**
       * Multiply the existing matrix with a rotation matrix.
       * The vectors <code>center</code> and <code>axis</code> have the
       * dimension 3.
       * @param center this vector is the center of the rotation.
       * @param axis this vector is the rotation-axis.
       * @param angle the rotation angle in radians.
       */
      void rotate(const vector<double>& center,
                  const vector<double>& axis,
                  const double& angle);

      /**
       * Multiply the existing matrix with a rotation matrix.
       * The vectors <code>center</code> and <code>axis</code> have the
       * dimension 3.
       * @param center this vector is the center of the rotation.
       * @param axis this vector is the rotation-axis.
       * @param angle the rotation angle in radians.
       */
      void rotate(const tpoint3D<double>& center,
                  const tpoint3D<double>& axis,
                  const double& angle);

      /**
       * Special rotate function with the center set to the origin:
       * The rotation axis is set to the z-axis
       */
      void rotate(const double& angle);

      /**
       * This function erase the tranfomation matrix,translation
       * vector and the rotation center. The matrix is overwriten with
       * the unit matrix.
       */
      void clear(void);

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
      virtual bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * Write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * This function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * This function is required by MSVC only, as a workaround for a
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
       * The transMatrix stores the matrix for the geometric transformation.
       * The functions rotate ,scale, and invert access the matrix.
       *
       * Default value:  unit matrix
       */
      hMatrix3D<float> transMatrix;

      /**
       * If keepDimensions is true, the virtual position and size of
       * the matrix are retained unchanged.  Otherwise the position and size
       * will be changed so that all points in the new image are displayed
       *
       * Default value:  true
       */
      bool keepDimensions;

      /**
       * Interpolator to be used.
       *
       * Default value: BilinearInterpolator
       *
       * @see lti::eInterpolatorType
       */
      eInterpolatorType interpolator;
    };

    // -----------------------------------------------------------------------
    //   Helper classes
    // -----------------------------------------------------------------------
  protected:
    /**
     * Structure to avoid the use of the access operator of matrices
     */
    struct fastMatrix {
      /**
       * Constructor
       */ 
      fastMatrix(const hMatrix3D<float>& m);

      /**
       * Constructor
       */
      fastMatrix(const hMatrix2D<float>& m);
      
      /**
       * The matrix is copied first in these variables to avoid the access
       * operator [y,x] which takes some more time...
       */
      float m00,m01,m02,m03,
            m10,m11,m12,m13,
            m30,m31,m32,m33;
    };

    /**
     * Transform 2D point
     *
     * Uses the given "fastMatrix" to transform the source point into the
     * destination point.
     */
    template<class value_type>
    inline void transf(const tpoint<value_type>& src,
                       tpoint<value_type>& dest,
                       const fastMatrix& m) const {
      // apply the matrix on the point src result is returned in dest
      // regular apply of the function
      const float factor = m.m30*src.x + m.m31*src.y + m.m33;      
      condRoundCastTo(((m.m00*src.x + m.m01*src.y + m.m03)/factor),dest.x);
      condRoundCastTo(((m.m10*src.x + m.m11*src.y + m.m13)/factor),dest.y);
    }

#ifndef SWIG
    /*
     * The template parameter I specifies the interpolator functor to
     * be used.
     * Valid types for I are all classes inherited from
     * scalarValueInterpolation.  The type of the matrices will
     * be typename I::value_type
     */
    template<class I>
    class helper {
    public:
      // type of matrices
      typedef typename I::value_type value_type;

    private:

      /*
       * The interpolator is created only once with the helper
       */
      I inpol;

      /*
       * parameters used by the apply method
       */
      parameters param;

      /*
       * transform 3D point
       */
      inline void transf(const tpoint3D<float>& src,
                         tpoint<float>& dest,
                         const fastMatrix& m) const {
        // apply the matrix on the point src result is returned in dest
        // regular apply of the function
        const float factor = (m.m30*src.x + m.m31*src.y + m.m32*src.z + m.m33);
        dest.x = ((m.m00*src.x + m.m01*src.y + m.m02*src.z + m.m03))/factor;
        dest.y = ((m.m10*src.x + m.m11*src.y + m.m12*src.z + m.m13))/factor;
      }

      // requirements of the helper class
    public:
      // default constructor
      helper() {};
      
      // constructor
      helper(const parameters& par) : param(par) {
        inpol.setBoundaryType(par.boundaryType);
      };

      // set parameters
      bool setParameters(const parameters& par) {
        param.copy(par);
        inpol.setBoundaryType(par.boundaryType);
        return true;
      }
      
      // apply method
      bool tApply(const matrix<value_type>& src,
                        matrix<value_type>& dest,
                        point& offset) const {
        if (src.empty()) {
          return false;
        }

        // Get parameters
        hMatrix3D<float> mat;
        mat.invert(param.transMatrix);
        
        if (param.keepDimensions) {
          dest.resize(src.size(),value_type(),false,false);
          offset.set(0,0);
        } else {
          // calculate the position of the four corners and use them to
          // define the size and offset of the new matrix
          tpoint<float> tr,tl,br,bl;
          param.transMatrix.multiply(tpoint<float>(0,0),tl);
          param.transMatrix.multiply(tpoint<float>((float)src.lastColumn(),0),
                                     tr);
          param.transMatrix.multiply(tpoint<float>(0,(float)src.lastRow()),bl);
          param.transMatrix.multiply(tpoint<float>((float)src.lastColumn(),
                                                   (float)src.lastRow()),br);
          
          point theMin,theMax;
          theMin.x = iround(min(min(tr.x,tl.x),min(br.x,bl.x)));
          theMax.x = iround(max(max(tr.x,tl.x),max(br.x,bl.x)));
          theMin.y = iround(min(min(tr.y,tl.y),min(br.y,bl.y)));
          theMax.y = iround(max(max(tr.y,tl.y),max(br.y,bl.y)));
          
          dest.resize(theMax.y-theMin.y+1,theMax.x-theMin.x+1,
                      value_type(),false,false);
          
          // now translate the transformation matrix to avoid using the
          // offset for every pixel:
          // note that the matrix must be
          hMatrix3D<float> matT;
          matT.translate(hPoint3D<float>((float)theMin.x,(float)theMin.y,0));
          mat.multiply(matT);
          // give the offset to the outside world
          offset = theMin;
        }
        
        tpoint<float> q;
        tpoint3D<float> p;
        const float sizex(static_cast<float>(dest.columns()));
        const float sizey(static_cast<float>(dest.rows()));
        
        float t2,a,b,c,d,r22,tmp;
        t2 = mat.at(2,3);
        r22 = mat.at(2,2);
        
        a = t2*mat.at(3,0)-mat.at(3,3)*mat.at(2,0);
        b = t2*mat.at(3,1)-mat.at(3,3)*mat.at(2,1);
        
        c = mat.at(3,2)*mat.at(2,0) - mat.at(3,0)*r22;
        d = mat.at(3,2)*mat.at(2,1) - mat.at(3,1)*r22;
        
        const fastMatrix fmat(mat);
        
        for (p.y=0;p.y<sizey;++p.y) {
          for (p.x=0;p.x<sizex;++p.x) {
            tmp = (c*p.x + d*p.y +r22);
            if (tmp == 0) {
              dest.at(static_cast<int>(p.y),
                      static_cast<int>(p.x)) = value_type(0);
            } else {
              p.z = (a*p.x+b*p.y-t2)/tmp;
              transf(p,q,fmat);
              dest.at(static_cast<int>(p.y),
                      static_cast<int>(p.x)) = inpol.interpolate(src,q.y,q.x);
            }
          }
        }
        
        return true;
      }

    // end of helper class
    };

    /*
     * Interpolator collection
     *
     * This is a class to get at compile time an appropriate helper class
     * for a given image type
     */
    template<class T>
    class interpolatorCollection {
    public:

#ifdef _LTI_GNUC_2
      geometricTransform::helper< nearestNeighborInterpolator<T> > nearest;
      geometricTransform::helper< bilinearInterpolator<T> >        bilinear;
      geometricTransform::helper< biquadraticInterpolator<T> >     biquadratic;
      geometricTransform::helper< genericInterpolator<T> >         bicubic;
#else
      helper< nearestNeighborInterpolator<T> > nearest;
      helper< bilinearInterpolator<T> >        bilinear;
      helper< biquadraticInterpolator<T> >     biquadratic;
      helper< genericInterpolator<T> >         bicubic;
#endif
      interpolatorCollection() {};

      interpolatorCollection(const parameters& par)
        : nearest(par),bilinear(par),biquadratic(par),bicubic(par) {};

      bool setParameters(const parameters& par) {
        return (nearest.setParameters(par) &&
                bilinear.setParameters(par) &&
                biquadratic.setParameters(par) &&
                bicubic.setParameters(par));
      }
    };

    /**
     * Some attributes with the interpolators to save the creation time
     * in the apply methods
     */
    //@{
    interpolatorCollection<double>   dcollect;
    interpolatorCollection<float>    fcollect;
    interpolatorCollection<int>      icollect;
    interpolatorCollection<ubyte>    bcollect;
    interpolatorCollection<rgbPixel> ccollect;
    //@}

    /**
     * Methods that return the proper collection type
     *
     * The dummy argument \a a is only there to allow the compiler to chose
     * the proper method.
     */
    //@{
    inline const interpolatorCollection<double>& 
    getCollection(double& a) const {
      a = 1.0;
      return dcollect;
    }
    inline const interpolatorCollection<float>& getCollection(float& a) const {
      a = 2.0;
      return fcollect;
    }
    inline const interpolatorCollection<int>& getCollection(int& a) const {
      a = 3;
      return icollect;
    }
    inline const interpolatorCollection<ubyte>& getCollection(ubyte& a) const {
      a = 4;
      return bcollect;
    }
    inline const interpolatorCollection<rgbPixel>&
    getCollection(rgbPixel& a) const {
      a = rgbPixel(5,5,5);
      return ccollect;
    }
#endif // SWIG   

    //@}

    // -----------------------------------------------------------------------
    //   Geometric Transform Functor
    // -----------------------------------------------------------------------
  public:
    /**
     * Default constructor
     */
    geometricTransform();

    /**
     * Default constructor with parameters object
     */
    geometricTransform(const parameters& par);

    /**
     * Copy constructor
     * @param other the object to be copied
     */
    geometricTransform(const geometricTransform& other);

    /**
     * Destructor
     */
    virtual ~geometricTransform();

    /**
     * Returns the name of this type ("geometricTransform")
     */
    virtual const char* getTypeName() const;

    /**
     * @name Apply methods for points and point lists
     */
    //@{

    // implementation of template member must be here (in header file) due
    // to MS VC++ bug.

    /**
     * Apply the geometric transformation to the given point
     * @param srcdest point with the source data.  The result
     *                 will be left here too.
     * @return true if successful, false otherwise
     */
    template<class T>
    bool apply(tpoint<T>& srcdest) const {
      tpoint<T> src(srcdest);
      return apply(src,srcdest);
    }

    /**
     * Apply the geometric transformation on src point and leave the result on
     * dest.
     * @param src point with the source data.
     * @param dest point where the result will be left.
     * @return true if successful, false otherwise
     */
    template<class T>
    bool apply(const tpoint<T>& src,tpoint<T>& dest) const {
      //transform one Point
      const fastMatrix mat(getParameters().transMatrix);
      transf(src,dest,mat);
      return true;
    }

    /**
     * Applies the geometric transformation on the given %parameter.
     * The transformation is applied on each point of the list.
     * @param srcdest pointList with the source data.  The result
     *                 will be left here too.
     * @return true if successful, false otherwise
     */
    template<class T>
    bool apply(tpointList<T>& srcdest) const {

      typename tpointList<T>::iterator it,eit;
      tpoint<T> tmp;

      it = srcdest.begin();
      eit = srcdest.end();

      const fastMatrix mat(getParameters().transMatrix);

      while(it!=eit) {
        transf(*it,tmp,mat);
        (*it).copy(tmp);
        ++it;
      }
      return true;
    }

    /**
     * Applies the geometric transformation on the given %parameter.
     * The transformation is applied on each point of the list.
     * @param src pointList with the source data.
     * @param dest pointList where the result will be left.
     * @return true if successful, false otherwise
     */
    template<class T>
    bool apply(const tpointList<T>& src,
                     tpointList<T>& dest) const {
      typename tpointList<T>::const_iterator it,eit;
      tpoint<T> tmp;

      it = src.begin();
      eit = src.end();
      dest.clear();

      const fastMatrix mat(getParameters().transMatrix);

      while(it!=eit) {
        transf(*it,tmp,mat);
        dest.push_back(tmp);
        ++it;
      }

      return true;
    }
    //@}

    /**
     * @name Apply methods for matrices
     */
    //@{

    /**
     * Applies the geometric transformation on the given %parameter.
     * The transformation is applied on each point of the matrix.
     *
     * @param srcdest matix with the source data.  The result
     *                 will be left here too.
     * @return true if successful or false otherwise.
     */
    template<class T>
    inline bool apply(matrix<T>& srcdest) const {
      matrix<T> dest;
      if (apply(srcdest,dest)) {
        dest.detach(srcdest);
        return true;
      }
      return false;
    }

    /**
     * Applies the geometric transformation on the given %parameter.
     * The transformation is applied on each point of the matrix.
     *
     * @param src matrix with the source data.
     * @param dest the result will be left here.
     * @return true if successful or false otherwise.
     */
    template<class T>
    inline bool apply(const matrix<T>& src,matrix<T>& dest) const {
      point offset;
      return apply(src,dest,offset);
    }


    /**
     * Applies the geometric transformation on the given %parameter.
     * The transformation is applied on each point of the matrix.
     *
     * @param src matrix with the source data.
     * @param dest the result will be left here.
     * @param offset if you specified parameters::keepDimensions as false,
     *        then this is the offset added to each pixel transformation to
     *        obtain the results in a positive range.  It is usefull if you
     *        also transform some points and want to obtain their value
     *        in the destination image.
     * @return true if successful or false otherwise.
     */
    template<class T>
    inline bool apply(const matrix<T>& src,
                            matrix<T>& dest,
                            point& offset) const {
      const parameters& param = getParameters();
      T tmp; // a dummy value used to select the proper collection
      switch(param.interpolator) {
        case NearestNeighborInterpolator: {
          return getCollection(tmp).nearest.tApply(src,dest,offset);
        } break;   
        case BiquadraticInterpolator: {
          return getCollection(tmp).biquadratic.tApply(src,dest,offset);
        } break;
        case BicubicInterpolator: {
          return getCollection(tmp).bicubic.tApply(src,dest,offset);
        } break;
        default: {
          return getCollection(tmp).bilinear.tApply(src,dest,offset);
        }
      }
    }
    //@}

    /**
     * @name Shortcuts for usual transformations.
     *
     * Example:
     * \code
     * // Usual parameters
     * lti::geometricTransform::parameters geoPar;
     * geoPar.keepDimensions = true;
     * geoPar.boundaryType = lti::Mirror;
     *
     * // Create geometric transformation object
     * lti::geometricTransform geoTrans(geoPar);
     *
     * // images 
     * lti::image img(256,256,lti::Black),res;
     *
     * // initialize your image with something
     * (img.getRow(img.rows()/2)).fill(lti::Red);
     *
     * // Rotate an image using shortcuts and default interpolator
     * geoTrans.rotate(img,res,45*Pi/180);  // rotate 45 degrees
     * \endcode
     */
    //@{
    /**
     * Scale the src image on copy.
     *
     * You usually want to set parameters::keepDimensions to false when using
     * this method.
     *
     * @see lti::scaling
     *
     * @param src source image
     * @param dest matrix where the result will be left
     * @param factor point containing the scaling factor for x and y
     *               coordinates
     * @return true if successful, false otherwise.
     */
    template<class T>
    bool scale(const matrix<T>& src,
                     matrix<T>& dest,
               const tpoint<float>& factor) {
      parameters par(getParameters());
      par.clear();
      par.scale(factor);
      setParameters(par);
      return apply(src,dest);
    }

    /**
     * Scale the srcdest image on-place
     *
     * You usually want to set parameters::keepDimensions to false when using
     * this method.
     *
     * @see lti::scaling
     *
     * @param srcdest source image.  The result will be left here too.
     * @param factor point containing the scaling factor for x and y
     *               coordinates
     * @return true if successful, false otherwise.
     */
    template<class T>
    bool scale(matrix<T>& srcdest,
               const tpoint<float>& factor) {
      matrix<T> tmp;
      if (scale(srcdest,tmp,factor)) {
        tmp.detach(srcdest);
        return true;
      }
      return false;
    }

    /**
     * Rotate the src image on copy
     *
     * @see lti::rotation
     *
     * @param src source image
     * @param dest matrix where the result will be left
     * @param angle angle in radians to rotate the image
     * @param center rotation point (takes effect only if \c keepDimensions
     *               in the parameters is set to true).  The default value
     *               ensures rotation on the image center.
     * @return true if successful, false otherwise.
     */
    template<class T>
    bool rotate(const matrix<T>& src,
                      matrix<T>& dest,
                const double& angle,
                const point& center=point(std::numeric_limits<int>::max(),
                                          std::numeric_limits<int>::max())) {
      
      tpoint3D<double> c;
      if ((center.x == std::numeric_limits<int>::max()) &&
          (center.y == std::numeric_limits<int>::max())) {
        c.x=src.columns()/2;
        c.y=src.rows()/2;
      } else {
        c.x=center.x;
        c.y=center.y;
      }
      c.z=0;
      tpoint3D<double> ax(0,0,1.0);

      parameters par(getParameters());
      par.clear();
      par.rotate(c,ax,angle);
      setParameters(par);

      return apply(src,dest);
    }

    /**
     * Rotate the image on place 
     *
     * @see lti::rotation
     *
     * @param srcdest matrix with source data and where the result will be left
     * @param angle angle in radians to rotate the image
     * @param center rotation point (takes effect only if \c keepDimensions
     *               in the parameters is set to true).  The default value
     *               ensures rotation on the image center.
     * @return true if successful, false otherwise.
     */
    template<class T>
    bool rotate(matrix<T>& srcdest,
                const double& angle,
                const point& center=point(std::numeric_limits<int>::max(),
                                          std::numeric_limits<int>::max())) {
      matrix<T> tmp;
      if (rotate(srcdest,tmp,angle,center)) {
        tmp.detach(srcdest);
        return true;
      }
      return false;
    }
    //@}

    /**
     * Copy data of "other" functor.
     * @param other the functor to be copied
     * @return a reference to this functor object
     */
    geometricTransform& copy(const geometricTransform& other);

    /**
     * Returns a pointer to a clone of this functor.
     */
    virtual functor* clone() const;

    /**
     * Returns used parameters
     */
    const parameters& getParameters() const;

    /**
     * Set parameters
     */
    virtual bool updateParameters();

  };
}

#endif
