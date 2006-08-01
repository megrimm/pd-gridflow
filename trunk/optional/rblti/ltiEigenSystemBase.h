#ifndef _LTI_EIGEN_SYSTEM_BASE_H_
#define _LTI_EIGEN_SYSTEM_BASE_H_

#include "ltiMatrix.h"
#include "ltiVector.h"
#include "ltiLinearAlgebraFunctor.h"

namespace lti {
  /**
   * eigenSystem functor.
   *
   * Base class for Eigenvalue and Eigenvector computation functors.
   *
   * Please note that the eigenvector matrices will contain the
   * eigenvector in the COLUMNS and not in the rows, as could be
   * expected.  This avoids the requirement of transposing matrices in
   * eigenvector-based transformations like PCA!
   */
  template<class T>
  class eigenSystem : public linearAlgebraFunctor {
  public:
    /**
     * eigenSystem parameter class
     */
    class parameters : public linearAlgebraFunctor::parameters {
    public:
      /**
       * default constructor
       */
      parameters() : linearAlgebraFunctor::parameters(),sort(false) {
      };

      /**
       * copy constructor
       */
      parameters(const parameters& other)
        : linearAlgebraFunctor::parameters() {copy(other);};

      /**
       * returns the name of this type
       */
      virtual const char* getTypeName() const {
        return "eigenSystem::parameters";
      };

      /**
       * copy member.
       */
      parameters& copy(const parameters& other) {
#ifndef _LTI_MSC_6
        // MS Visual C++ 6 is not able to compile this...
        linearAlgebraFunctor::parameters::copy(other);
#else
        // ...so we have to use this workaround.
        // Conditional on that, copy may not be virtual.
        functor::parameters&
          (functor::parameters::* p_copy)(const functor::parameters&) =
          functor::parameters::copy;
        (this->*p_copy)(other);
#endif
        sort = other.sort;

        return (*this);
      };

      /**
       * returns a pointer to a clone of the parameters.
       */
      virtual functor::parameters* clone() const {
        return (new parameters(*this));
      };

      // ---------------------------------------------------
      // the parameters
      // ---------------------------------------------------

      /**
       * If true, the eigenvalues and corresponding eigenvectors will be
       * sorted in decreasing order of the eigenvalues.
       *
       * Default value: false
       */
      bool sort;

    };

    /**
     * default constructor
     */
    eigenSystem()
      : linearAlgebraFunctor() {};

    /**
     * constructor, sets the parameters
     */
    eigenSystem(const parameters& theParams);

    /**
     * constructor, sets the parameters
     */
    eigenSystem(const bool theSort);

    /**
     * destructor
     */
    virtual ~eigenSystem() {};

    /**
     * returns the current parameters.
     */
    const parameters& getParameters() const;

    /**
     * onCopy version of apply.
     * @param theMatrix matrix to be transformed
     * @param eigenvalues elements will contain the eigenvalues
     * @param eigenvectors columns will contain the eigenvectors
     *        corresponding to the eigenvalues
     * @return bool true if successful, false otherwise.
     */
    virtual bool apply(const matrix<T>& theMatrix,
                       vector<T>& eigenvalues,
                       matrix<T>& eigenvectors) const=0;

    /**
     * returns the name of this type
     */
    virtual const char* getTypeName() const {
      return "eigenSystem";
    };
  };

}


#endif

