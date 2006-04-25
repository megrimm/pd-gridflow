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
 * file .......: ltiGenericMatrix.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 09.04.99
 * revisions ..: $Id$
 */

#ifndef _LTI_GENERIC_MATRIX_H_
#define _LTI_GENERIC_MATRIX_H_

#include "ltiMathObject.h"
#include "ltiGenericVector.h"
#include "ltiPoint.h"
#include "ltiRectangle.h"

namespace lti {
  /**
   * GenericMatrix container class.
   *
   * The lti::genericMatrix class allows the representation of \e n x
   * \e m matrices of any type showing .  The rows will be indexed
   * between 0 and n-1, and the columns between 0 and m-1.
   *
   * All types defined in ltiTypes.h use static members and can be contained
   * by the lti::genericVector and lti::genericMatrix classes.
   *
   * The genericMatrix class is a container class implemented as template.
   *
   * If you need to create a genericMatrix of floats with 20 rows and
   * 15 columns, all elements initialized with an initial value of
   * 4.27 just create it:
   *
   * \code
   * lti::genericMatrix<float> myMat(20,15,4.27f) // creates genericMatrix 
   *                                              // with 300 elements
   *                                              // all initialized with 4.27f
   * \endcode
   *
   * To access the genericMatrix elements use the access operators.
   * There are many possibilities.  With at(const int row, const int
   * col) is possible to access an element directly.  With at(const
   * int row) you can get the row vector.  You cannot for instance
   * resize nor change the memory referenced in this vector (see
   * lti::vector::resize).  For example:
   *
   * \code
   * float accu = 0; // initialize accumulator
   * lti::genericMatrix<float> myMat(20,15,4.27f)
   * lti::vector<float> myVct;
   *
   * for (int j = 0; j < myMat.rows(); j++) {
   *   for (int i = 0; i < myMat.columns(); i++) {
   *   tmp += myMat.at(j,i); // access each element of the genericMatrix:
   *                         // j is the row and i the column
   *   }
   * }
   *
   * myMat.getRowCopy(5,myVct); // copy the sixth row in myVct!
   * myVct.resize(6);           // Valid, the vector has its own memory!
   * myMat.at(5).resize(6)      // ERROR!! the vector is not resizable!
   *
   * \endcode
   *
   * The image representation in the LTILib is based on the lti::genericMatrix
   * class.  It is quite confusing to use first the y-coordinate and then
   * the x-coordinate to access the image elements.  To avoid confusion use
   * the lti::point class to access the elements of the genericMatrix:
   *
   * \code
   *
   * lti::channel8 aChannel(20,15); // creates an 8bit image
   * lti::channel8::value_type tmp; // tmp is of the element type of the
   *                                // channel8!
   * lti::point p;
   *
   *
   * for (p.y = 0; p.y < aChannel.rows(); p.y++) {
   *   for (p.x = 0; p.x < aChannel.columns(); p.x++) {
   *   tmp += aChannel.at(p); // access each element of the genericMatrix:
   *                          // equivalent to: tmp += aChannel.at(p.y,p.x)!
   *   }
   * }
   *
   * \endcode
   *
   * @see lti::image, lti::channel, lti::channel8,
   *      lti::dgenericMatrix, lti::fgenericMatrix, lti::igenericMatrix
   *
   * The genericMatrix has following methods:
   * - Constructors Constructors
   *   -  You can construct an empty genericMatrix with the default constructor
   *      (genericMatrix()).
   *   - If you know the number of rows and columns use
   *     genericMatrix(const int rows,const int columns,const T&
   *     initialValue)
   *   - You can create a copy of another genericMatrix or just copy a
   *     subgenericMatrix with the copy constructor (genericMatrix(const
   *     genericMatrix<T>& otherGenericMatrix))
   * - Access members
   *   - at(), operator[]()
   *   - The rows() member returns the number of rows of the genericMatrix.
   *   - The columns() member returns the number of columns of the
         genericMatrix.
   * - Fill and Copy members
   *   - With the fill() members you can fill the genericMatrix with a given
   *     constant value or with values taken from other matrices.
   *   - With the copy() member you can copy another genericMatrix.
   *   - You can specify, that the genericMatrix should be used just as a
   *     wrapper-object to access external memory regions: useExternData().
   *     To check if a genericMatrix is a wrapper-object you can use 
   *     ownsData().
   * - Mathematical operations
   *   - GenericMatrix multiplication: multiply()
   *   - Elementwise multiplication: emultiply()
   *   - Add another genericMatrix to the actual genericMatrix: add()
   *   - Add another <b>scaled</b> genericMatrix to the actual genericMatrix:
   *     addScaled()
   *   - Add a constant value to all elements of the genericMatrix: 
   *     add(const T& cst)
   *   - Subtract another vector from the actual vector: subtract()
   *   - Multiply the vector with a constant value: multiply()
   * - Iterators
   *   - It is possible to iterate within the genericMatrix by making use of
   *     the genericMatrix iterators.  (see begin() for more information)
   *   - Instead of reverse_iterators as in the STL we use iterators
   *     going backwards, due to faster execution times (see
   *     inverseBegin() for more information)
   *
   * @ingroup gLinearAlgebra
   * @ingroup gAggregate
   */
  template<class T>
  class genericMatrix : public mathObject {
    public:
    /**
     * type of the contained data
     */
    typedef T value_type;

    /**
     * The data can be stored sequentially in the memory or in lines.
     * The "Line" modus will be used when a matrix is created as a
     * submatrix of another "Connected" matrix.
     */
    typedef enum {
      Connected, /**< the data is stored as a single memory block */
      Line       /**< each line has its own memory block.  The iterators
                      do not work on this mode, but you can iterate on each
                      row */
    } eStoreMode;

#   ifdef NDEBUG
    /**
     * Iterator type (allows read and write operations)
     *
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::genericMatrix::begin()
     * for an example .
     *
     * For the debugging version of the iterators, boundary check will be
     * done!  This explains the low speed of the iterators of the debug
     * version.  In the release version, no boundary check will be done,
     * and the iterators are sometimes a factor 10 faster than the
     * debug iterators.
     *
     * The use of the access operator at(.) is faster than the iterators
     * in the debug version only.  If you need to iterate on a genericMatrix use
     * iterators instead (in the release version iterators are approx. a
     * factor 3 faster than "at(.)").
     *
     * Iterators do not work on lined matrices.  This means, if you have
     * a submatrix reference of another one (getMode() == Lined), the
     * iterators won't follow the submatrix but the original data.  This will
     * not change since it would imply reducing the performance for connected
     * matrices, for which the iterators were designed on the first place!
     *
     * CAUTION: Try to use the prefix incremental operator
     * (i.e. ++it) instead of the postfix operator (i.e. it++) to
     * allow efficient code also in debug-modus!
     */
    typedef T* iterator;

    /**
     * const iterator type (allows read-only operations)
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::genericMatrix::begin()
     * for an example.
     *
     * For the debugging version of the iterators, boundary check will be
     * done!  This explains the low speed of the iterators of the debug
     * version.  In the release version, no boundary check will be done,
     * and the iterators are sometimes a factor 10 faster than the
     * debug iterators.
     *
     * The use of the access operator at(.) is faster than the iterators in
     * the debug version only.  If you need to iterate on a genericMatrix use
     * iterators instead (in the release version iterators are approx. a
     * factor 3 faster than "at(.)").
     *
     * Iterators do not work on lined matrices.  This means, if you have
     * a submatrix reference of another one (getMode() == Lined), the
     * iterators won't follow the submatrix but the original data.  This will
     * not change since it would imply reducing the performance for connected
     * matrices, for which the iterators were designed on the first place!
     *
     * CAUTION: Try to use the prefix incremental operator
     * (i.e. ++it) instead of the postfix operator (i.e. it++) to
     * allow efficient code also in debug-modus!
     */
    typedef const T* const_iterator;

#   else

    class const_iterator;

    /**
     * iterator type (allows read and write operations).
     *
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::genericMatrix::begin()
     * for an example
     *
     * For the debugging version of the iterators, boundary check will be
     * done!  This explains the low speed of the iterators of the debug
     * version.  In the release version, no boundary check will be done,
     * and the iterators are sometimes a factor 10 faster than the
     * debug iterators.
     *
     * The use of the access operator at(.) is faster than the iterators
     * in the debug version only.  If you need to iterate on a genericMatrix use
     * iterators instead (in the release version iterators are approx. a
     * factor 3 faster than "at(.)").
     *
     * Iterators don't work on lined matrices.
     *
     * CAUTION: Try to use the prefix incremental operator
     * (i.e. ++it) instead of the postfix operator (i.e. it++) to
     * allow efficient code also in debug-modus!
     */
    class iterator {
      friend class genericMatrix<T>;

#     ifdef _LTI_MSC_VER
      friend class const_iterator;
#     else
      friend class genericMatrix<T>::const_iterator;
#     endif

    public:
      /**
       * default constructor
       */
      iterator() : pos(0), theGenericMatrix(0) {};

      /**
       * copy constructor
       */
      iterator(const iterator& other)
        : pos(other.pos),theGenericMatrix(other.theGenericMatrix) {};

      /**
       * advance to next item
       */
      inline iterator& operator++() {++pos; return *this;};   // prefix

      /**
       * advance to next item
       */
      inline iterator operator++(int) {
        iterator tmp(*this); pos++; return tmp;
      }; // postfix

      /**
       * recede to previos item
       */
      inline iterator& operator--() {--pos; return *this;};   // prefix

      /**
       * recede to previos item
       */
      inline iterator operator--(int) {
        iterator tmp(*this); pos--; return tmp;
      }; // postfix

      /**
       * advance (skip) some elements.
       * Use this operator with care! Note that you can skip the end of
       * the genericMatrix, and read (or even worse: write!) out of bounds!
       */
      inline iterator& operator+=(const int n) {pos+=n; return *this;};

      /**
       * recede (skip) some elements.
       * Use this operator with care! Note that you can skip the end of
       * the genericMatrix, and read (or even worse: write!) out of bounds!
       */
      inline iterator& operator-=(const int n) {pos-=n; return *this;};

      /**
       * advance (skip) some elements.
       * Use this operator with care! Note that you can skip the end of
       * the vector, and read (or even worse: write!) out of bounds!
       */
      inline iterator operator+(const int n) {
        return iterator(pos+n,theGenericMatrix);
      };

      /**
       * recede (skip) some elements.
       * Use this operator with care! Note that you can skip the beginning of
       * the vector, and read (or even worse: write!) out of bounds!
       */
      inline iterator operator-(const int n) {
        return iterator(pos-n,theGenericMatrix);
      };

      /**
       * compare
       */
      inline bool operator==(const iterator& other) const {
        return (pos == other.pos);
      };

      /**
       * compare
       */
      inline bool operator!=(const iterator& other) const {
        return (pos != other.pos);
      };

      /**
       * compare if the position of the first iterator is smaller than
       * the position of the second iterator
       */
      inline bool operator<(const iterator& other) const {
        return (pos < other.pos);
      };

      /**
       * compare if the position of the first iterator is greater than
       * the position of the second iterator
       */
      inline bool operator>(const iterator& other) const {
        return (pos > other.pos);
      };

      /**
       * compare if the position of the first iterator is smaller or equal than
       * the position of the second iterator
       */
      inline bool operator<=(const iterator& other) const {
        return (pos <= other.pos);
      };

      /**
       * compare if the position of the first iterator is greater or equal
       * than the position of the second iterator
       */
      inline bool operator>=(const iterator& other) const {
        return (pos >= other.pos);
      };

      /**
       * get pointed data
       */
      inline T& operator*() {
        return theGenericMatrix->at(pos);
      };

      /**
       * copy member
       */
      inline iterator& operator=(const iterator& other) {
        pos = other.pos;
        theGenericMatrix = other.theGenericMatrix;
        return *this;
      };

    protected:
      /**
       * for internal use only!!!
       * This method does not exist in the release version!
       */
      const int getPos() const {return pos;}

      /**
       * for internal use only!!!
       * This method does not exist in the release version!
       */
      const genericMatrix<T>* getGenericMatrix() const {
        return theGenericMatrix;
      }

      /**
       * default constructor (for internal use only)
       * NEVER USE THIS CONSTRUCTOR, OR YOUR CODE WILL NOT
       * COMPILE IN THE RELEASE VERSION!
       */
      explicit iterator(const int startPos,genericMatrix<T>* vct)
        : pos(startPos), theGenericMatrix(vct) {};

    private:
      /**
       * actual genericMatrix index
       */
      int pos;

      /**
       * pointer to the actual genericMatrix
       */
      genericMatrix<T>* theGenericMatrix;
    };

    /**
     * const iterator type (allows read-only operations).
     *
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::genericMatrix::begin()
     * for an example.
     *
     * For the debugging version of the iterators, boundary check will be
     * done!  This explains the low speed of the iterators of the debug
     * version.  In the release version, no boundary check will be done,
     * and the iterators are sometimes a factor 10 faster than the
     * debug iterators.
     *
     * The use of the access operator at(.) is faster than the iterators
     * in the debug version only.  If you need to iterate on a genericMatrix use
     * iterators instead (in the release version iterators are approx. a
     * factor 3 faster than "at(.)").
     *
     * Iterators don't work on lined matrices.
     *
     * CAUTION: Try to use the prefix incremental operator
     * (i.e. ++it) instead of the postfix operator (i.e. it++) to
     * allow efficient code also in debug-modus!
     */
    class const_iterator {
      friend class genericMatrix<T>;
    public:
      /**
       * default constructor
       */
      const_iterator() : pos(0), theGenericMatrix(0) {};

      /**
       * copy constructor
       */
      const_iterator(const const_iterator& other) {(*this)=other;};

      /**
       * copy constructor
       */
      const_iterator(const iterator& other) {(*this)=other;};

      /**
       * advance to next item  -- prefix
       */
      inline const_iterator& operator++() {++pos; return *this;};

      /**
       * recede to previous item  -- prefix
       */
      inline const_iterator& operator--() {--pos; return *this;};

      /**
       * advance to next item  -- postfix
       */
      inline const_iterator operator++(int) {
        const_iterator tmp(*this); pos++; return tmp;
      };

      /**
       * recede to previous item  -- postfix
       */
      inline const_iterator operator--(int) {
        const_iterator tmp(*this); pos--; return tmp;
      };

      /**
       * advance (skip) some elements.
       * Use this operator with care! Note that you can skip the end of
       * the genericMatrix, and read (or even worse: write!) out of bounds!
       */
      inline const_iterator& operator+=(const int n) {pos+=n; return *this;};

      /**
       * recede (skip) some elements.
       * Use this operator with care! Note that you can skip the beginning of
       * the genericMatrix, and read (or even worse: write!) out of bounds!
       */
      inline const_iterator& operator-=(const int n) {pos-=n; return *this;};

      /**
       * advance (skip) some elements.
       * Use this operator with care! Note that you can skip the end of
       * the vector, and read (or even worse: write!) out of bounds!
       */
      inline const_iterator operator+(const int n) {
        return const_iterator(pos+n,theGenericMatrix);
      };

      /**
       * recede (skip) some elements.
       * Use this operator with care! Note that you can skip the beginning of
       * the vector, and read (or even worse: write!) out of bounds!
       */
      inline const_iterator operator-(const int n) {
        return const_iterator(pos-n,theGenericMatrix);
      };

      /**
       * compare
       */
      inline bool operator==(const const_iterator& other) const {
        return (pos == other.pos);
      };

      /**
       * compare
       */
      inline bool operator!=(const const_iterator& other) const {
        return (pos != other.pos);
      };

      /**
       * compare
       */
      inline bool operator==(const iterator& other) const {
        return (pos == other.getPos());
      };

      /**
       * compare
       */
      inline bool operator!=(const iterator& other) const {
        return (pos != other.getPos());
      };

      /**
       * compare if the position of the first iterator is smaller than
       * the position of the second iterator
       */
      inline bool operator<(const iterator& other) const {
        return (pos < other.pos);
      };

      /**
       * compare if the position of the first iterator is greater than
       * the position of the second iterator
       */
      inline bool operator>(const iterator& other) const {
        return (pos > other.pos);
      };

      /**
       * compare if the position of the first iterator is smaller or equal than
       * the position of the second iterator
       */
      inline bool operator<=(const iterator& other) const {
        return (pos <= other.pos);
      };

      /**
       * compare if the position of the first iterator is greater or equal
       * than the position of the second iterator
       */
      inline bool operator>=(const iterator& other) const {
        return (pos >= other.pos);
      };

      /**
       * compare if the position of the first iterator is smaller than
       * the position of the second iterator
       */
      inline bool operator<(const const_iterator& other) const {
        return (pos < other.pos);
      };

      /**
       * compare if the position of the first iterator is greater than
       * the position of the second iterator
       */
      inline bool operator>(const const_iterator& other) const {
        return (pos > other.pos);
      };

      /**
       * compare if the position of the first iterator is smaller or equal than
       * the position of the second iterator
       */
      inline bool operator<=(const const_iterator& other) const {
        return (pos <= other.pos);
      };

      /**
       * compare if the position of the first iterator is greater or equal
       * than the position of the second iterator
       */
      inline bool operator>=(const const_iterator& other) const {
        return (pos >= other.pos);
      };

      /**
       * get pointed data
       */
      inline const T& operator*() {return theGenericMatrix->at(pos);};

      /**
       * copy member
       */
      inline const_iterator& operator=(const const_iterator& other) {
        pos = other.pos;
        theGenericMatrix = other.theGenericMatrix;
        return *this;
      };

      /**
       * copy member
       */
      inline const_iterator& operator=(const iterator& other) {
        pos = other.getPos();
        theGenericMatrix = other.getGenericMatrix();
        return *this;
      };

    protected:
      /**
       * protected constructor
       * DO NOT EXPLICITLY USE THIS CONSTRUCTOR. OTHERWISE YOUR CODE WILL NOT
       * COMPILE IN THE RELEASE VERSION!
       */
      explicit const_iterator(const int startPos,const genericMatrix<T>* vct)
        : pos(startPos), theGenericMatrix(vct) {};

    private:
      /**
       * actual genericMatrix index
       */
      int pos;

      /**
       * pointer to the actual genericMatrix
       */
      const genericMatrix<T>* theGenericMatrix;
    };

#   endif

    /**
     * Default constructor creates an empty genericMatrix
     */
    genericMatrix();

    /**
     * Create a <code>rows x cols</code> genericMatrix and
     * initializes all elements with \c iniValue.
     *
     * @param rows number of rows of the genericMatrix
     * @param cols number of columns of the genericMatrix
     * @param iniValue all elements will be initialized with this value
     */
    genericMatrix(const int rows,const int cols,const T& iniValue = T());

    /**
     * Create a <code>rows x cols</code> genericMatrix and initialize all 
     * elements with the data pointed by <code>data</code>.
     *
     * The first <code>cols</code>-elements of the data will be copied on the
     * first row, the next ones on the second row and so on.
     *
     * @param rows number of rows of the genericMatrix
     * @param cols number of columns of the genericMatrix
     * @param data pointer to the memory block with the data to be initialized
     *            with.
     */
    genericMatrix(const int rows,const int cols,const T data[]);

    /**
     * This constructor creates a connected <code>size.y x size.x</code>
     * GenericMatrix and initializes all elements with <code>iniValue</code>
     *
     * @param size lti::point with the size of the genericMatrix
     *             (size.x is the number of columns and
     *              size.y the number of rows)
     * @param iniValue all elements will be initialized with this value
     */
    genericMatrix(const ipoint& size,const T& iniValue = T());

    /**
     * Copy constructor.
     *
     * create this genericMatrix as a connected copy of another
     * genericMatrix for this const version, the data will be always
     * copied!  It is also possible to create a copy of a
     * subgenericMatrix of another genericMatrix.
     *
     * @param other the genericMatrix to be copied.
     * @param fromRow initial row of the other genericMatrix to be copied
     * @param toRow last row to be copied of the other genericMatrix
     * @param fromCol initial column of the other genericMatrix to be copied
     * @param toCol last column to be copied of the other genericMatrix
     *
     * Example:
     * \code
     * lti::genericMatrix<int> m(4,6,0); // integer genericMatrix with 25 
     *                                   // elements
     * // ...
     * // initialize GenericMatrix with:
     * //        0  1  2  3  4  5
     * //        2  1  5  4  0  3
     * //        1  2  1  2  3  2
     * //        3  3  2  1  2  3
     *
     * lti::genericMatrix<int> sm(m,0,2,1,3)  // last line will lead to
     * //                                 following contents in sm:
     * //        1  2  3
     * //        1  5  4
     * //        2  1  2
     * \endcode
     */
    genericMatrix(const genericMatrix<T>& other,
                  const int fromRow=0,const int toRow=MaxInt32,
                  const int fromCol=0,const int toCol=MaxInt32);

    /**
     * If \a init is true this constructor is equivalent to calling
     * genericMatrix(const int rows, const int cols), and thus initializing
     * all elements with T(). However, in some cases the elements need
     * not be initialized during construction, since complex
     * initializion is required. Especially for large matrices, the
     * unnecessary constructor initialization is very time consuming.
     *
     * If \a init is false, memory is allocated but no initialization
     * takes place. Thus the following is equivalent:
     * \code
     * genericMatrix<int> a(false,100,100);
     *
     * genericMatrix<int> a;
     * a.resize(100,100,0,false,false);
     * \endcode
     *
     * @param init initialize genericMatrix or not
     * @param rows number of rows of the genericMatrix
     * @param cols number of columns of the genericMatrix
     */
    genericMatrix(const bool init, const int rows, const int cols);

    /**
     * If \a init is true this constructor is equivalent to calling
     * genericMatrix(const int rows, const int cols), and thus initializing
     * all elements with T(). However, in some cases the elements need
     * not be initialized during construction, since complex
     * initializion is required. Especially for large matrices, the
     * unnecessary constructor initialization is very time consuming.
     *
     * If \a init is false, memory is allocated but no initialization
     * takes place. Thus the following is equivalent:
     * \code
     * genericMatrix<int> a(false,100,100);
     *
     * genericMatrix<int> a;
     * a.resize(100,100,0,false,false);
     * \endcode
     *
     * @param init initialize genericMatrix or not
     * @param size desired size for the genericMatrix
     */
    genericMatrix(const bool init, const ipoint& size);

    /**
     * copy constructor.
     *
     * create this genericMatrix as a connected copy of another genericMatrix
     * taking only the rows indicated by the vector.
     * for this const version, the data will be always copied!
     * Multiple occurence of one row index in <code>rows</code> is allowed.
     *
     * @param other the genericMatrix to be copied.
     * @param rows inidices of the rows to be copied
     *
     * Example:
     * \code
     * lti::vector<int> rows(2);
     * // initialize with
     * // 1 3
     * lti::genericMatrix<int> m(4,6,0); // integer genericMatrix with 25
     *                                   // elements
     * // ...
     * // initialize GenericMatrix with:
     * //        0  1  2  3  4  5
     * //        2  1  5  4  0  3
     * //        1  2  1  2  3  2
     * //        3  3  2  1  2  3
     *
     * lti::genericMatrix<int> sm(m,rows)     // last line will lead to
     * //                                 following contents in sm:
     * //        2  1  5  4  0  3
     * //        3  3  2  1  2  3
     * \endcode
     */
    genericMatrix(const genericMatrix<T>& other,
                  const genericVector<int>& rows);

    /**
     * copy constructor (reference to a submatrix).
     * creates submatrix of another matrix.
     *
     * if <code>copyData == true</code>, the new object has its own data
     * (equivalent to previous copy constructor).
     *
     * if <code>copyData == false</code>, the new object has references to
     * the other matrix, which means that the data is not necessarily
     * consecutive.  (This will not be a connected but a lined matrix)
     *
     * Those algorithms which use direct access to the matrix memory block
     * should check first if the memory lies in a consecutive block!
     * (see getMode())
     *
     * @param copyData should the data of the other matrix be copied or not
     * @param other the matrix with the data to be copied or to be shared
     * @param fromRow initial row of the other matrix to be copied
     * @param toRow last row to be copied of the other matrix
     * @param fromCol initial column of the other matrix to be copied
     * @param toCol last column to be copied of the other matrix
     */
    genericMatrix(const bool copyData, genericMatrix<T>& other,
                  const int fromRow=0,const int toRow=MaxInt32,
                  const int fromCol=0,const int toCol=MaxInt32);

    /**
     * destructor
     */
    virtual ~genericMatrix();

    /**
     * returns the name of this class: "genericMatrix"
     */
    virtual const char* getTypeName() const;

    /**
     * owns this %object the data?
     * returns <em>false</em> if this genericMatrix contains a reference to
     *         extern data.
     */
    inline bool ownsData() const;

    /**
     * If this object does not own its data, this member will create a
     * new memory buffer with the same data and will make this
     * genericMatrix as its owner.  You can also be sure, that the new
     * memory block will be connected (see also getMode() ).  If this
     * genericMatrix already owns its data nothing happens.
     */
    void restoreOwnership();

    /**
     * Reference to extern data.
     *
     * This member allows the use of this %object as an access-functor for
     * the 'data'. An access to the element at (r,c) is equivalent to
     * data[r*columns() + c].
     * The user must take care for memory allocation and deallocation:
     * this object will never delete the data!.
     * If <em>rows</em> and <em>cols</em> are invalid dimensions, the
     * behaviour will be unpredictible.
     * @param rows number of rows
     * @param cols number of columns
     * @param data a pointer to the memory block to be used
     *
     * For an example see attach()
     */
    void useExternData(const int rows,const int cols,T* data);

    /**
     * Attach extern data to the genericMatrix.
     *
     * This member allows the use of this %object as an access-functor for
     * the 'data'. An access to the element at (r,c) is equivalent to
     * data[r*columns() + c].
     * If <em>rows</em> and <em>cols</em> are invalid dimensions, the
     * behaviour will be unpredictible.
     *
     * The memory will be administrated by this genericMatrix object,
     * and may be deleted if required (genericMatrix deleted or
     * resized!).  The user should not try to manipulate the memory
     * allocation of the data after the attachment!  See also
     * useExternData().
     *
     * @param rows number of rows
     * @param cols number of columns
     * @param data a pointer to the memory block to be used
     *
     * Example:
     * \code
     * lti::genericMatrix<int> myMat;
     * int block1[25];
     * int* block2;
     * block2 = new int[25];
     *
     * myMat.useExternData(5,5,block1); // ok
     * myMat.attach(5,5,block1); // wrong!!! genericMatrix will try 
     *                           // to manipulate stack memory: 
     *                           // DO NOT DO THIS!!!!!
     * myMat.attach(5,5,block2); // ok!  but do not try to delete the memory
     *                           //      block2!!
     * \endcode
     */
    void attach(const int rows,const int cols,T* data);

    /**
     * Free the data of this object and hand it over to the
     * "receiver". The value of ownsData is also transfered to the
     * receiver. (see Note).
     *
     * This function makes a "memory block transfusion" to another
     * genericMatrix.  It is a very efficient way to make a copy of
     * this genericMatrix, if you don't need the source data anymore!
     *
     * \b Note: This method will fail if this genericMatrix is not
     * connected. Also take care that if the attach() or
     * useExternData() methods of this genericMatrix have been called before
     * detachment, the same rules for memory management apply now for
     * the receiver.
     *
     * At the end of the detachment, this genericMatrix will be empty.
     *
     * @param receiver the genericMatrix which will receive the memory
     *        block.  All data of that genericMatrix will be first deleted!
     */
    void detach(genericMatrix<T>& receiver);

    /**
     * Free the data of this object and hand it over to the
     * "receiver". The value of ownsData is also transfered to the
     * receiver. (see Note).
     *
     * This function makes a "memory block transfusion" to a vector by
     * concatenating the rows of the genericMatrix.  It is a very efficient
     * way to move the data of this genericMatrix into a vector, if you don't
     * need the source data anymore!
     *
     * \b Note: This method will fail if this genericMatrix is not
     * connected. Also take care that if the attach() or
     * useExternData() methods of this genericMatrix have been called before
     * detachment, the same rules for memory management apply now for
     * the receiver.
     *
     * At the end of the detachment, this genericMatrix will be empty.
     * @param receiver the genericMatrix which will receive the memory block.
     *                 All data of that genericMatrix will be first deleted!
     */
    void detach(genericVector<T>& receiver);

    /**
     * \deprecated Please use swap instead
     */
    void exchange(genericMatrix<T>& other);

    /**
     * Exchange (in a fast way) the data between this and the other 
     * genericMatrix.
     *
     * Similar to detach(), this method will exchange the complete memory
     * blocks, avoiding an element-wise copy.
     * @param other the genericMatrix with which the data will be exchanged.
     */
    void swap(genericMatrix<T>& other);

    /**
     * Data storage mode.
     *
     * Returns the data storage mode, which can be:
     * - <code>Connected</code> if the memory is a single block, or
     * - <code>Line</code>      if the memory of each line is allocated
     *                          in different places.
     *
     * For the lined-matrices the interators will not work.  You can however
     * iterate on each individual row, which are always connected.
     *
     * The only possible way to get a lined-matrix is as a submatrix of
     * another one, using the appropriate constructor:
     * genericMatrix(const bool, genericMatrix<T>&,const int,const int,
     * const int,const int)
     */
    inline eStoreMode getMode() const;

    /**
     * number of rows of the genericMatrix
     */
    inline const int rows() const;

    /**
     * number of columns of the genericMatrix
     */
    inline const int columns() const;

    /**
     * index of the last row (rows()-1)
     */
    inline const int lastRow() const;

    /**
     * Index of the last columns (columns()-1)
     */
    inline const int lastColumn() const;

    /**
     * Number of "physical" rows of the matrix.
     *
     * @return If this is a <code>Connected</code> Matrix, (see getMode()),
     * this member returns the same value as rows().
     *
     * If this is a <code>Line</code> Matrix, this value is bigger or equal
     * than rows().  If this was created with the copy constructor for a
     * submatrix with "no copy data", this value will return the size of the
     * original matrix.
     */
    inline const int metaRows() const;

    /**
     * Number of "physical" columns of the matrix.
     *
     * @return If this is a <code>Connected</code> Matrix, (see getMode()),
     * this member returns the same value as columns().
     *
     * If this is a <code>Line</code> Matrix, this value is bigger or equal
     * than columns().  If this was created with the copy constructor for a
     * submatrix with "no copy data", this value will return the number of
     * columns of the original matrix.
     */
    inline const int metaColumns() const;

    /**
     * return type of the size() member
     */
    typedef ipoint size_type;

    /**
     * returns the size of the %genericMatrix in a lti::point structure.
     * @return lti::point with the number of columns in its
     *         <code>x</code> coordinate and the number of rows in its
     *         <code>y</code> coordinate.
     */
    inline const size_type& size() const;

#ifndef SWIG
    /**
     * returns iterator to the begin of the genericMatrix
     * The use of the interators is similar to the iterators of the
     * Standard Template Library (STL).
     * If you need to iterate on all elements of the genericMatrix, you can
     * use following code:
     * \code
     * int tmp,accu;                  // a temporal variable
     * lti::genericMatrix<int> myMat(10,8,1); // a vector with 10 elements
     * lti::genericMatrix<int>::iterator it;  // an iterator
     *
     * for (it=myMat.begin();it!=myMat.end();it++) {
     *   tmp = *it;                   // tmp has value of element pointed
     *                                // by the iterator.
     *   accu += tmp;
     *   (*it) = accu;                // change the value in the genericMatrix.
     * }
     * \endcode
     * Please note that if you define <code>it</code> as a const_iterator,
     * you can not make something like <code>*it=accu</code>.
     */
    inline iterator begin();

    /**
     * returns first element of the genericMatrix as a const_iterator.
     * Note that you can not change the values of the genericMatrix
     * elements when you use a const_iterator. See also begin() for an example
     */
    inline const_iterator begin() const;

    /**
     * returns iterator to the end of the genericMatrix
     */
    inline iterator end();

    /**
     * returns iterator to the end of the genericMatrix
     */
    inline const_iterator end() const;


    /**
     * This method returns an iterator that points to the \b last
     * valid element of the genericMatrix. It is used for inverse order
     * iteration through the genericMatrix using normal iterators (as opposed
     * to reverse iterators). This has the advantage that iterators
     * going from front to end and in the inverse direction are the
     * same and can thus be compared, copied etc. Further the
     * implementation of reverse_iterators is not as fast as that of
     * iterators and thus not desired in the LTI-Lib.
     *
     * See lti::genericVector<T>::inverseBegin() for an example.
     */
    inline iterator inverseBegin();

    /**
     * This method returns an iterator that points to the \b last
     * valid element of the genericMatrix. See inverseBegin() for more details.
     */
    inline const_iterator inverseBegin() const;

    /**
     * This method returns an iterator that points to the element \b
     * before the \b first valid element of the genericMatrix. It is used to
     * mark the end for inverse order iteration through the genericMatrix
     * using normal iterators (as opposed to reverse iterators). This
     * has the advantage that iterators going from front to end and in
     * the inverse direction are the same and can thus be compared,
     * copied etc.
     */
    inline iterator inverseEnd();

    /**
     * This method returns an iterator that points to the element \b
     * before the \b first valid element of the genericMatrix.
     */
    inline const_iterator inverseEnd() const;

    /**
     * change the dimensions of the genericMatrix.
     * @param newRows new number of rows
     * @param newCols new number of columns
     * @param iniValue the initialization value.
     * @param copyData if this parameter is true, the old data of the
     *                 genericMatrix will be copied.  If it is false, the old 
     *                 data will be lost.
     * @param initNew  if this parameter is true, then all new elements (if
     *                 they exist) will be initialized with
     *                 <code>iniValue</code>.
     *                 if <code>initNew</code> is false, then the new
     *                 elements are left uninitialized.
     *
     * For example:
     * \code
     *   lti::genericMatrix<int> myMat;  // creates empty genericMatrix
     *   myMat.resize(5,5,0);     // genericMatrix with 5x5 elements 
     *                            // initialized with 0
     *   myMat.resize(10,7,2);    // genericMatrix has now 10x7 elements: the
     *                            // subgenericMatrix 5x5 at (0,0) has still 0s
     *                            // and the rest have a 2
     *   myMat.resize(20,10,0,false,false); // now the genericMatrix has 20
     *                                      // elements but their values
     *                                      // are unknown.
     *   myMat.resize(5,5,1,false,true); // the genericMatrix has now 5x5
     *                                   // elements all initialized with 1
     *
     *   // note that the last line could also be written:
     *
     *   myMat.resize(5,5,1,false);
     *
     * \endcode
     *
     * If the resize is possible (see useExternData()), this %object
     * will always owns the data!
     */
    void resize(const int newRows,const int newCols,
                const T& iniValue = T(),
                const bool copyData=true,
                const bool initNew=true);
#endif // SWIG

    /**
     * clears the genericMatrix (at the end this will be an empty
     * genericMatrix)
     */
    void clear();

    /**
     * returns true if the genericMatrix is empty
     */
    bool empty() const;

    /**
     * change the dimensions of the genericMatrix.
     * @param newDim   new dimensions of the genericMatrix
     * @param iniValue the initialization value.
     * @param copyData if this parameter is true, the old data of the
     *                 genericMatrix will be copied.  If it is false, the old
     *                  data will be lost.
     * @param initNew  if this parameter is true, then all new elements (if
     *                 they exist) will be initialized with
     *                 <code>iniValue</code>.
     *                 if <code>initNew</code> is false, then the new
     *                 elements are left uninitialized.
     *
     * If the resize is possible (see useExternData()), this %object
     * will always owns the data!
     *
     * This is equivalte to call
     *   resize(newDim.y,newDim.x,iniValue,copyData,initNew)
     */
    inline void resize(const ipoint& newDim,
                       const T& iniValue = T(),
                       const bool copyData=true,
                       const bool initNew=true);

    /**
     * fills genericMatrix elements with <code>iniValue</code>.
     * The fill "area" is limited by <code>fromCol</code>,<code>toCol</code>,
     * <code>fromRow</code> and <code>toRow</code>.
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     *
     * WARNING: Syntax changed for the parameters:  the old version was
     *          fromRow,toRow, fromCol,toCol.
     *          Now the order is:
     *          fromRow,FROMCOL,TOROW,toCol.  This allow to give the
     *          first coordinates only!
     *
     * @param iniValue the elements will be initialized with this value
     * @param fromRow  first row of the subgenericMatrix to be filled
     * @param fromCol  first column of the subgenericMatrix to be filled
     * @param toRow    last row of the subgenericMatrix to be filled
     * @param toCol    last column of the subgenericMatrix to be filled
    */
    void fill(const T& iniValue,
              const int fromRow=0,
              const int fromCol=0,
              const int toRow=MaxInt32,
              const int toCol=MaxInt32);

    /**
     * fills genericMatrix elements with <code>iniValue</code>.
     * The fill "area" is limited by <code>from</code> and <code>to</code>
     * points
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     *
     * @param iniValue the elements will be initialized with this value
     * @param from first position of the subgenericMatrix to be filled
     * @param to   last row of the subgenericMatrix to be filled
    */
    inline void fill(const T& iniValue,
                     const ipoint& from,
                     const ipoint& to=point(MaxInt32,MaxInt32));

    /**
     * fills genericMatrix elements with <code>iniValue</code>.
     * The fill "area" is limited by <code>window</code>.
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     * @param iniValue the elements will be initialized with this value
     * @param window   the window to be filled
     */
    inline void fill(const T& iniValue,const irectangle& window);

    
#ifndef SWIG
    /**
     * fills genericMatrix elements with the data pointed by <code>data</code>.
     * The fill "area" is limited by <code>fromCol</code>,<code>toCol</code>,
     * <code>fromRow</code> and <code>toRow</code>.
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     *
     * WARNING: Syntax changed for the parameters:  the old version was
     *          fromRow,toRow, fromCol,toCol.
     *          Now the order is:
     *          fromRow,FROMCOL,TOROW,toCol.  This allow to give the
     *          first coordinates only!
     *
     * @param data     pointer to the data to be copied.
     * @param fromRow  first row of the subgenericMatrix to be filled
     * @param fromCol  first column of the subgenericMatrix to be filled
     * @param toRow    last row of the subgenericMatrix to be filled
     * @param toCol    last column of the subgenericMatrix to be filled
     */
    void fill(const T data[],
              const int fromRow=0,
              const int fromCol=0,
              const int toRow=MaxInt32,
              const int toCol=MaxInt32);

    /**
     * fills genericMatrix elements with the data pointed by <code>data</code>.
     * The fill "area" is limited by <code>fromCol</code>,<code>toCol</code>,
     * <code>fromRow</code> and <code>toRow</code>.
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     *
     * @param data  pointer to the data to be copied.
     * @param from  first position of the subgenericMatrix to be filled
     * @param to    last position of the subgenericMatrix to be filled
     */
    inline void fill(const T data[],
                     const ipoint& from,
                     const ipoint& to=point(MaxInt32,MaxInt32));

    /**
     * fills genericMatrix elements with <code>iniValue</code>.
     * The fill "area" is limited by <code>window</code>.
     * If these values are out of bounds, they will be (internally)
     * adjusted to correct values.
     * @param data   pointer to the data to be copied
     * @param window the window to be filled
     */
    inline void fill(const T data[],const irectangle& window);

#endif // SWIG

    /**
     * fills this genericMatrix between the "from's" and "to's" with
     * the contents of the genericMatrix <code>mat</code> starting at
     * <code>startAtRow</code> and <code>startAtCol</code>
     *
     * WARNING: Syntax changed for the parameters:  the old version was
     *          fromRow,toRow, fromCol,toCol.
     *          Now the order is:
     *          fromRow,FROMCOL,TOROW,toCol.  This allow to give the
     *          from coordinates only!
     *
     * @param mat      the genericMatrix with the data to be copied
     * @param fromRow  first row of the subgenericMatrix to be filled
     * @param fromCol  first column of the subgenericMatrix to be filled
     * @param toRow    last row of the subgenericMatrix to be filled
     * @param toCol    last column of the subgenericMatrix to be filled
     * @param startAtRow starting row of <code>mat</code> where the data is
     *                   located.
     * @param startAtCol starting column of <code>mat</code> where the data
     *                   is located.
     */
    void fill(const genericMatrix<T>& mat,
              const int fromRow=0,
              const int fromCol=0,
              const int toRow=MaxInt32,
              const int toCol=MaxInt32,
              const int startAtRow=0,
              const int startAtCol=0);

    /**
     * fills this genericMatrix between the "from's" and "to's" with
     * the contents of the genericMatrix <code>mat</code> starting at
     * <code>startAtRow</code> and <code>startAtCol</code>
     *
     * @param mat     the genericMatrix with the data to be copied
     * @param from    first position of the subgenericMatrix to be filled
     * @param to      last position of the subgenericMatrix to be filled
     * @param startAt starting position of <code>mat</code> where the data is
     *                located.
     */
    inline void fill(const genericMatrix<T>& mat,
                     const ipoint& from,
                     const ipoint& to=point(MaxInt32,MaxInt32),
                     const ipoint& startAt=point(0,0));

    /**
     * fills the region of this genericMatrix specified by
     * <code>window</code> with the contents of the genericMatrix
     * <code>mat</code> starting at <code>start</code>.  If these
     * values are out of bounds, they will be (internally) adjusted to
     * correct values.
     *
     * @param mat    pointer to the data to be copied
     * @param window the window to be filled
     * @param start  the start position of the region to be copied of the
     *               genericMatrix <code>mat</code>
     */
    inline void fill(const genericMatrix<T>& mat,
                     const irectangle& window,
                     const ipoint& start=point(0,0));


//#ifndef SWIG
    /**
     * access element at the given row and column
     * @param row the row of the element
     * @param col the column of the element
     * @return a reference to the genericMatrix element
     */
    inline T& at(const int row, const int col);

    /**
     * read-only access at the given row and column
     * @param row the row of the element
     * @param col the column of the element
     * @return a reference to the genericMatrix element
     */
    inline const T& at(const int row, const int col) const;

    /**
     * access element at the given position.
     * (can be used only in connected matrices.  See constructors for
     *  more information on this.)
     *
     * With this operator the genericMatrix can be accessed as a
     * vector, where the rows of the genericMatrix are concatenated.
     * The access to the genericMatrix with at(row,col) is equivalent
     * to at(row*columns()+col).
     *
     * @param pos the index of the element of the genericMatrix
     * @return a reference to the genericMatrix element
     */
    inline T& at(const int pos);

    /**
     * access element at the given position.
     * (can be used only in connected matrices.  See constructors for
     *  more information on this.)
     *
     * With this operator the genericMatrix can be accessed as a
     * vector, where the rows of the genericMatrix are concatenated.
     * The access to the genericMatrix with at(row,col) is equivalent
     * to at(row*columns()+col).
     *
     * @param pos the index of the element of the genericMatrix
     * @return a reference to the genericMatrix element
     */
    inline const T& at(const int pos) const;

    /**
     * access operator of genericMatrix element as a point in a 2D-Map
     *
     * @param p position of the element (this is equivalent to at(p.y,p.x))
     * @return a reference to the genericMatrix element
     */
    inline T& at(const ipoint& p);

    /**
     * const access operator of genericMatrix element as a point in a 2D-Map
     *
     * @param p position of the element (this is equivalent to at(p.y,p.x))
     * @return a const reference to the vector element
     */
    inline const T& at(const ipoint& p) const;
//#endif // SWIG

    /**
     * return genericMatrix-row as a vector.
     *
     * This method works fast, since it returns a reference to the row vector.
     * The data will NOT be copied.
     *
     * @param row the row to be accessed
     * @return a reference to the vector row
     */
    inline genericVector<T>& getRow(const int row);

    /**
     * return genericMatrix-row as a const vector.
     * This method works fast, since it returns a reference to the row vector.
     * The data will NOT be copied.
     *
     * @param row the row to be accessed
     * @return a const reference to the vector row
     */
    inline const genericVector<T>& getRow(const int row) const;

    /**
     * alias for getRow()
     */
    inline genericVector<T>& operator[](const int row);

    /**
     * alias for getRow()
     */
    inline const genericVector<T>& operator[](const int row) const;

    /**
     * Copy a row vector in the given parameter.
     *
     * This method copies the data of a given row of the genericMatrix
     * in the given vector.
     *
     * @param row the number of the row to be copied
     * @param theRow the vector, where the data will be copied
     * @see getRow()
     */
    inline void getRowCopy(const int row,genericVector<T>& theRow) const;

    /**
     * return genericMatrix-row as a vector.
     * This method copies the data of the genericMatrix, therefore is not as
     * fast as getRow()
     * @param row the number of tthe row to be copied
     * @return a vector with the contents of the row of the genericMatrix
     */
    inline genericVector<T> getRowCopy(const int row) const;

    /**
     * return genericMatrix-column as a vector.
     * This method copies the data of the genericMatrix, therefore is not as
     * fast as getRow()
     * @param col the number of the column to be copied
     * @param theCol a vector, where the column vector of the genericMatrix
     *               should be copied.
     */
    void getColumnCopy(const int col,genericVector<T>& theCol) const;

    /**
     * return genericMatrix-column as a vector.
     * This method copies the data of the genericMatrix, therefore is not as
     * fast as getRow()
     * @param col the number of the column to be copied
     * @return a vector with the contents of the column of the genericMatrix
     */
    inline genericVector<T> getColumnCopy(const int col) const;

    /**
     * Return the diagonal elements of the genericMatrix as vector.
     * This method copies the diagonal elements of the genericMatrix into
     * the vector. If the genericMatrix is non-symmetrical, the vector will
     * be of dimension min(rows(),columns()).
     * @param diag a vector, where the diagonal of the genericMatrix
     *             should be copied.
     */
    void getDiagonal(genericVector<T>& diag) const;

    /**
     * Return the diagonal elements of the genericMatrix as vector.
     * This method copies the diagonal elements of the genericMatrix into
     * the vector. If the genericMatrix is non-symmetrical, the vector will
     * be of dimension min(rows(),columns()).
     * @return a vector with the diagonal elements of the genericMatrix.
     */
    inline genericVector<T> getDiagonal() const;

    /**
     * copy the data of a vector in a given row
     * @param row the row that receives the data.
     * @param theRow the vector with the data to be copied
     */
    inline void setRow(const int row,const genericVector<T>& theRow);

    /**
     * copy the data of a vector in a given column
     * @param col the column that receives the data.
     * @param theCol the vector with the data to be copied
     */
    void setColumn(const int col,const genericVector<T>& theCol);

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected genericMatrix.
     * I.e. you cannot copy the sub-genericMatrix property of another
     * genericMatrix.
     *
     * @param other the other genericMatrix to be copied
     */
    genericMatrix<T>& copy(const genericMatrix<T>& other);

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected genericMatrix.
     * I.e. you cannot copy the sub-genericMatrix property of another
     * genericMatrix.
     *
     * @param other the other genericMatrix to be copied
     * @param fromRow initial row of the other genericMatrix to be copied
     * @param toRow last row to be copied of the other genericMatrix
     * @param fromCol initial column of the other genericMatrix to be copied
     * @param toCol last column to be copied of the other genericMatrix
     */
    genericMatrix<T>& copy(const genericMatrix<T>& other,
                           const int fromRow,
                           const int toRow=MaxInt32,
                           const int fromCol=0,
                           const int toCol=MaxInt32);

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected genericMatrix.
     * I.e. you cannot copy the sub-genericMatrix property of another
     * genericMatrix.
     *
     * @param other the other genericMatrix to be copied
     * @param window rectangle define the copy area
     */
    inline genericMatrix<T>& copy(const genericMatrix<T>& other,
                                  const irectangle& window);

    /**
     * assigment operator.
     *
     * copy the contents of the specified rows/columns of
     * <code>other</code> into this object. Multiple occurence of one
     * row/column index in <code>idx</code> is allowed. If the
     * argument \b rows is true, \b idx specifies rows, if false \b
     * idx specifies columns.
     *
     * The result of the copy is always a connected genericMatrix.
     * I.e. you cannot copy the sub-genericMatrix property of another
     * genericMatrix.
     *
     * @param other the other genericMatrix to be copied
     * @param idx indices of the rows to be copied.
     * @param rows if true works on rows, else on columns
     */
    genericMatrix<T>& copy(const genericMatrix<T>& other,
                           const genericVector<int>& idx,
                           bool rows=true);

    /**
     * copy the <code>other</code> genericMatrix by casting each of
     * its elements
     *
     * @param other The genericMatrix to be casted
     *
     * Example:
     * \code
     *   lti::genericMatrix<int> matA(10,10,1);// a genericMatrix of integers
     *   lti::genericMatrix<double> matB;      // a genericMatrix of doubles
     *
     *   matB.castFrom(matA);          // this will copy matA in matB!!
     * \endcode
     */
    template<class U>
    genericMatrix<T>& castFrom(const genericMatrix<U>& other) {
      resize(other.rows(),other.columns(),T(),false,false);
      int y;
      for (y=0;y<rows();y++) {
        getRow(y).castFrom(other.getRow(y));
      }

      return (*this);
    };

#ifdef SWIG
    %template(castFromInt) 	castFrom<int>;
    %template(castFromFloat) 	castFrom<float>;
    %template(castFromDouble) 	castFrom<double>;
    %template(castFromUbyte) 	castFrom<ubyte>;
    //%template(CastFromRgbPixel)	castFrom<rgbPixel>;
#endif    

    /**
     * create a clone of this genericMatrix
     * @return a pointer to a copy of this genericMatrix
     */
    virtual mathObject* clone() const;

    /**
     * Compare this genericMatrix with other.
     *
     * @param other the other genericMatrix to be compared with
     * @return true if both matrices have the same elements and same size
     */
    bool equals(const genericMatrix<T>& other) const;

    /**
     * Compare the contents of each element of this genericMatrix with the
     * other one.  It assumes the type T can be compared using the
     * operator==.
     *
     * @param other the other genericMatrix to be compared with
     * @return true if both matrices have the same elements and same size
     */
    inline bool operator==(const genericMatrix<T>& other) const;

    /**
     * Assigment operator (alias for copy(other)).
     *
     * @param other the genericMatrix to be copied
     * @return a reference to the actual genericMatrix
     */
    inline genericMatrix<T>& operator=(const genericMatrix<T>& other);

#ifndef SWIG
    /**
     * @name Apply Methods
     * Following methods are used to apply simple functions to each element
     * of the vector.
     */
    //@{

    /**
     * applies a C-function to each element of the genericMatrix.
     * @param function a pointer to a C-function
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(T (*function)(T));

    /**
     * applies a C-function to each element of the other genericMatrix
     * @param other the genericMatrix which elements will go through the given
     *              function.
     * @param function a pointer to a C-function
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& other,T (*function)(T));

    /**
     * applies a C-function to each element of the genericMatrix.
     * @param function a pointer to a C-function
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(T (*function)(const T&));

    /**
     * applies a C-function to each element of the other genericMatrix
     * @param other the genericMatrix which elements will go through the given
     *              function.
     * @param function a pointer to a C-function
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& other,
                            T (*function)(const T&));

    /**
     * a two-parameter C-function receives the i-th elements of this
     * and the given genericMatrix and the result will be left in this
     * genericMatrix.  Note that both matrices must have the same size!
     * @param other the second genericMatrix to be considered (the first
     *              genericMatrix will be this object!)
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& other,
                            T (*function)(const T&,const T&));

    /**
     * a two-parameter C-function receives the i-th elements of this
     * and the given genericMatrix and the result will be left in this
     * genericMatrix.  Note that both matrices must have the same size!
     * @param other the second genericMatrix to be considered (the first
     *              genericMatrix will be this object!)
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& other,
                            T (*function)(T,T));

    /**
     * a two-parameter C-function receives the i-th elements of both given
     * matrices and leaves the result here.
     * Note that both matrices must have the same size!
     * @param a the first genericMatrix
     * @param b the second genericMatrix
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& a,
                     const genericMatrix<T>& b,
                     T (*function)(const T&,const T&));

    /**
     * a two-parameter C-function receives the i-th elements of both given
     * matrices and leaves the result here.
     * Note that both matrices must have the same size!
     * @param a the first genericMatrix
     * @param b the second genericMatrix
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual genericMatrix
     */
    genericMatrix<T>& apply(const genericMatrix<T>& a,
                     const genericMatrix<T>& b,
                     T (*function)(T,T));

    //@}
#endif // SWIG

    /**
     * @name Input and Output
     */
    //@{

    /**
     * write the object in the given ioHandler
     */
    virtual bool write(ioHandler& handler,const bool& complete = true) const;

    /**
     * read the object from the given ioHandler
     */
    virtual bool read(ioHandler& handler,const bool& complete = true);
    //@}

  protected:

    /**
     * number of rows of the genericMatrix
     */
    int& numRows;

    /**
     * number of columns of the genericMatrix
     */
    int& numColumns;

    /**
     * theSize.x is equal numColumns and theSize.y is equal numRows
     */
    ipoint theSize;

    /**
     * index of the last row
     */
    int lastRowIdx;

    /**
     * index of the last column
     */
    int lastColIdx;

    /**
     * number of rows the "physical" matrix
     * (always bigger than the dimensions of the "active" matrix).
     */
    int metaNumRows;

    /**
     * number of columns of the "physical" matrix
     * (always bigger than the dimensions of the "active" matrix).
     */
    int metaNumColumns;

    /**
     * size of theElements
     */
    int totalSize;

    /**
     * indicates if <code>theElements</code> points to own data or to
     * external data.
     */
    bool ownData;

    /**
     * indicate if <code>theElements</code> points to consecutive memory
     * or to "sparse" memory
     */
    eStoreMode mode;

    /**
     * pointer to the elements of the genericMatrix
     */
    T* theElements;

    /**
     * table of pointers to the rows
     */
    genericVector<T>* rowAddressTable;

    /**
     * Allocate \a n number of rows or the appropriate type.
     */
    inline virtual genericVector<T>* allocRows(const int n);

  };
}

/**
 * outputs the elements of the vector on a stream
 */
namespace std {
  template <class T>
  ostream& operator<<(ostream& s,const lti::genericMatrix<T>& amat);
}

#include "ltiGenericMatrix_inline.h"
#include "ltiGenericMatrix_template.h"

#else
#include "ltiGenericMatrix_template.h"
#endif

