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
 * file .......: ltiEigenSystem.h
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 16.06.99
 * revisions ..: $Id$
 */

#ifndef _LTI_EIGEN_SYSTEM_H_
#define _LTI_EIGEN_SYSTEM_H_

#include "ltiMatrix.h"
#include "ltiVector.h"
#include "ltiLinearAlgebraFunctor.h"
#include "ltiEigenSystemBase.h"

namespace lti {
  /**
   * jacobi functor.
   * This functor computes all eigenvalues and eigenvectors of a
   * real _symmetric and square_ matrix using the Jacobi method.
   *
   * Please note that the eigenvector matrices will contain the
   * eigenvector in the COLUMNS and not in the rows, as could be
   * expected.  This avoids the requirement of transposing matrices in
   * eigenvector-based transformations like PCA!  All eigenvector functors
   * follows this interface.
   */
  template<class T>
  class jacobi : public eigenSystem<T> {
  public:

    /**
     * eigenSystem parameter class
     */
    class parameters : public eigenSystem<T>::parameters {
    public:
      /**
       * default constructor
       */
      parameters()
        : eigenSystem<T>::parameters(),dimensions(0) {
      };

      /**
       * copy constructor
       */
      parameters(const parameters& other) {
        copy(other);
      };

      /**
       * number of dimensions calculated. The default is zero. In this
       * case, all eigenvectors and eigenvalues are calculated.
       *
       * This option is only provided for compatibility with the
       * fastEigenSystem functor based on LAPACK, but it does not make (yet)
       * the computation of the eigensolution any faster.  It just will cut
       * the already computed complete solution to the desired size!
       *
       * Default value: 0 (implying all eigenvalues will be computed)
       */
      int dimensions;

      /**
       * returns the name of this type
       */
      virtual const char* getTypeName() const {
        return "fastEigenSystem::parameters";
      };

      /**
       * copy member.
       */
      parameters& copy(const parameters& other) {
#ifndef _LTI_MSC_6
        // MS Visual C++ 6 is not able to compile this...
        eigenSystem<T>::parameters::copy(other);
#else
        // ...so we have to use this workaround.
        // Conditional on that, copy may not be virtual.
        eigenSystem<T>::parameters&
          (eigenSystem<T>::parameters::* p_copy)(const eigenSystem<T>::parameters&) =
          eigenSystem<T>::parameters::copy;
        (this->*p_copy)(other);
#endif
        dimensions=other.dimensions;

        return (*this);
      };

      /**
       * returns a pointer to a clone of the parameters.
       */
      virtual functor::parameters* clone() const {
        return (new parameters(*this));
      };
    };


    /**
     * default constructor
     */
    jacobi();

    /**
     * default constructor with parameters
     */
    jacobi(const parameters& params);


    /**
     * onCopy version of apply.
     * @param theMatrix  Real symmetric matrix to be transformed.
     * Only the diagonal and above diagonal elements have
     * to be set.
     * @param eigenvalues  Elements will contain the eigenvalues.
     * @param eigenvectors Columns will contain the eigenvectors corresponding
     * to the eigenvalues.
     * returns true if successful, false otherwise.
     */
    virtual bool apply(const matrix<T>& theMatrix,
                       vector<T>& eigenvalues,
                       matrix<T>& eigenvectors) const;

    /**
     * returns a pointer to a clone of the functor.
     */
    virtual functor* clone() const { return (new jacobi<T>(*this));};

    /**
     * returns the name of this type
     */
    virtual const char* getTypeName() const {return "jacobi";};

    /**
     * returns the current parameters.
     */
    const parameters& getParameters() const;

  protected:
    inline void rotateT(double& g,double& h,matrix<T>& a,
                        const int i,const int j,const int k,const int l,
                        const double s,const double tau) const;

    inline void rotate(double& g,double& h,matrix<double>& a,
                       const int i,const int j,const int k,const int l,
                       const double s,const double tau) const;


  };
}


#endif

