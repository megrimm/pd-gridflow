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
 * file .......: ltiMatrix.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 09.04.99
 * revisions ..: $Id$
 */

#ifndef _LTI_MATRIX_H_
#define _LTI_MATRIX_H_

#include "ltiMathObject.h"
#include "ltiVector.h"
#include "ltiPoint.h"
#include "ltiRectangle.h"

#define _LTI_GENERIC_MATRIX_DONT_INSTANTIATE_REQUEST 1
#include "ltiGenericMatrix.h"
#undef _LTI_GENERIC_MATRIX_DONT_INSTANTIATE_REQUEST

namespace lti {
  /**
   * Mathematical matrix container class.
   *
   * The lti::matrix class allows the representation of \e n x \e m matrices.
   * The rows will be indexed between 0 and n-1, and the columns between 0
   * and m-1.
   *
   * All types defined in ltiTypes.h use static members and can be contained
   * by the lti::vector and lti::matrix classes.
   *
   * The matrix class is a container class implemented as template.
   *
   * If you need to create a matrix of floats with 20 rows and 15 columns,
   * all elements initialized with an initial value of 4.27 just create it:
   *
   * \code
   * lti::matrix<float> myMat(20,15,4.27f) // creates matrix with 300 elements
   *                                       // all initialized with 4.27f
   * \endcode
   *
   * To access the matrix elements use the access operators.  There are many
   * possibilities.  With at(const int row, const int col) is possible to
   * access an element directly.  With at(const int row) you can get the row
   * vector.  You cannot for instance resize nor change the memory referenced
   * in this vector (see lti::vector::resize).  For example:
   *
   * \code
   * float accu = 0; // initialize accumulator
   * lti::matrix<float> myMat(20,15,4.27f)
   * lti::vector<float> myVct;
   *
   * for (int j = 0; j < myMat.rows(); j++) {
   *   for (int i = 0; i < myMat.columns(); i++) {
   *   tmp += myMat.at(j,i); // access each element of the matrix:
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
   * The image representation in the LTILib is based on the lti::matrix
   * class.  It is quite confusing to use first the y-coordinate and then
   * the x-coordinate to access the image elements.  To avoid confusion use
   * the lti::point class to access the elements of the matrix:
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
   *   tmp += aChannel.at(p); // access each element of the matrix:
   *                          // equivalent to: tmp += aChannel.at(p.y,p.x)!
   *   }
   * }
   *
   * \endcode
   *
   * @see lti::image, lti::channel, lti::channel8,
   *      lti::dmatrix, lti::fmatrix, lti::imatrix
   *
   * The parent class is lti::genericMatrix and provides many memory
   * management options.
   *
   * The matrix has following methods:
   * - Constructors Constructors
   *   -  You can construct an empty matrix with the default constructor
   *      (matrix()).
   *   -  If you know the number of rows and columns use
   *      matrix(const int rows,const int columns,const T& initialValue)
   *   -  You can create a copy of another matrix or just copy a submatrix
   *      with the copy constructor (matrix(const matrix<T>& otherMatrix))
   * - Access members
   *   - at(), operator[]()
   *   - The rows() member returns the number of rows of the matrix.
   *   - The columns() member returns the number of columns of the matrix.
   * - Fill and Copy members
   *   -  With the fill() members you can fill the matrix with a given
   *      constant value or with values taken from other matrices.
   *   -  With the copy() member you can copy another matrix.
   *   -  You can specify, that the matrix should be used just as a
   *      wrapper-object to access external memory regions: useExternData().
   *      To check if a matrix is a wrapper-object you can use ownsData().
   * - Mathematical operations
   *   - Matrix multiplication: multiply()
   *   - Elementwise multiplication: emultiply()
   *   - Add another matrix to the actual matrix: add()
   *   - Add another <b>scaled</b> matrix to the actual matrix: addScaled()
   *   - Add a constant value to all elements of the matrix: add(const T& cst)
   *   - Subtract another vector from the actual vector: subtract()
   *   - Multiply the vector with a constant value: multiply()
   *   - Transpose the matrix: transpose()
   * - Iterators
   *   - It is possible to iterate within the matrix by making use of
   *     the matrix iterators.  (see begin() for more information)
   *   - Instead of reverse_iterators as in the STL we use iterators
   *     going backwards, due to faster execution times (see
   *     inverseBegin() for more information)
   *
   * @ingroup gLinearAlgebra
   * @ingroup gAggregate
   */
  template<class T>
  class matrix : public genericMatrix<T> {
    public:
    /**
     * type of the contained data
     */
    typedef T value_type;

    /**
     * return type of the size() member
     */
    typedef ipoint size_type;

    // iterator types of the parent class
    typedef typename genericMatrix<T>::iterator iterator;
    typedef typename genericMatrix<T>::const_iterator const_iterator;


    /**
     * default constructor creates an empty matrix
     */
    matrix();

    /**
     * this constructor creates a connected <code>rows x cols</code> Matrix
     * and initializes all elements with <code>iniValue</code>
     * @param rows number of rows of the matrix
     * @param cols number of columns of the matrix
     * @param iniValue all elements will be initialized with this value
     */
    matrix(const int rows,const int cols,const T& iniValue = T());

    /**
     * this constructor creates a connected <code>rows x cols</code> Matrix
     * and initializes all elements with the data pointed by
     * <code>data</code>.  The first <code>cols</code>-elements of the data
     * will be copied on the first row, the next ones on the second row and
     * so on.
     * @param rows number of rows of the matrix
     * @param cols number of columns of the matrix
     * @param data pointer to the memory block with the data to be initialized
     *            with.
     */
    matrix(const int rows,const int cols,const T data[]);

    /**
     * this constructor creates a connected <code>size.y x size.x</code>
     * Matrix and initializes all elements with <code>iniValue</code>
     * @param size lti::point with the size of the matrix
     *             (size.x is the number of columns and
     *              size.y the number of rows)
     * @param iniValue all elements will be initialized with this value
     */
    matrix(const ipoint& size,const T& iniValue = T());

    /**
     * copy constructor.
     *
     * create this matrix as a connected copy of another matrix
     * for this const version, the data will be always copied!
     * It is also possible to create a copy of a submatrix of another matrix.
     *
     * @param other the matrix to be copied.
     * @param fromRow initial row of the other matrix to be copied
     * @param toRow last row to be copied of the other matrix
     * @param fromCol initial column of the other matrix to be copied
     * @param toCol last column to be copied of the other matrix
     *
     * Example:
     * \code
     * lti::matrix<int> m(4,6,0); // integer matrix with 25 elements
     * // ...
     * // initialize Matrix with:
     * //        0  1  2  3  4  5
     * //        2  1  5  4  0  3
     * //        1  2  1  2  3  2
     * //        3  3  2  1  2  3
     *
     * lti::matrix<int> sm(m,0,2,1,3)  // last line will lead to
     * //                                 following contents in sm:
     * //        1  2  3
     * //        1  5  4
     * //        2  1  2
     * \endcode
     */
    matrix(const matrix<T>& other,
           const int fromRow=0,const int toRow=MaxInt32,
           const int fromCol=0,const int toCol=MaxInt32);

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
    matrix(const bool copyData, matrix<T>& other,
           const int fromRow=0,const int toRow=MaxInt32,
           const int fromCol=0,const int toCol=MaxInt32);

    /**
     * If \a init is true this constructor is equivalent to calling
     * matrix(const int rows, const int cols), and thus initializing
     * all elements with T(). However, in some cases the elements need
     * not be initialized during construction, since complex
     * initializion is required. Especially for large matrices, the
     * unnecessary constructor initialization is very time consuming.
     *
     * If \a init is false, memory is allocated but no initialization
     * takes place. Thus the following is equivalent:
     * \code
     * matrix<int> a(false,100,100);
     *
     * matrix<int> a;
     * a.resize(100,100,0,false,false);
     * \endcode
     *
     * @param init initialize matrix or not
     * @param rows number of rows of the matrix
     * @param cols number of columns of the matrix
     */
    matrix(const bool init, const int rows, const int cols);

    /**
     * If \a init is true this constructor is equivalent to calling
     * matrix(const int rows, const int cols), and thus initializing
     * all elements with T(). However, in some cases the elements need
     * not be initialized during construction, since complex
     * initializion is required. Especially for large matrices, the
     * unnecessary constructor initialization is very time consuming.
     *
     * If \a init is false, memory is allocated but no initialization
     * takes place. Thus the following is equivalent:
     * \code
     * matrix<int> a(false,100,100);
     *
     * matrix<int> a;
     * a.resize(100,100,0,false,false);
     * \endcode
     *
     * @param init initialize matrix or not
     * @param size desired size for the matrix
     */
    matrix(const bool init, const ipoint& size);

    /**
     * copy constructor.
     *
     * create this matrix as a connected copy of another matrix
     * taking only the rows indicated by the vector.
     * for this const version, the data will be always copied!
     * Multiple occurence of one row index in <code>rows</code> is allowed.
     *
     * @param other the matrix to be copied.
     * @param rows inidices of the rows to be copied
     *
     * Example:
     * \code
     * lti::vector<int> rows(2);
     * // initialize with
     * // 1 3
     * lti::matrix<int> m(4,6,0); // integer matrix with 25 elements
     * // ...
     * // initialize Matrix with:
     * //        0  1  2  3  4  5
     * //        2  1  5  4  0  3
     * //        1  2  1  2  3  2
     * //        3  3  2  1  2  3
     *
     * lti::matrix<int> sm(m,rows)     // last line will lead to
     * //                                 following contents in sm:
     * //        2  1  5  4  0  3
     * //        3  3  2  1  2  3
     * \endcode
     */
    matrix(const matrix<T>& other,
           const genericVector<int>& rows);

    /**
     * destructor
     */
    virtual ~matrix();

    /**
     * returns the name of this class: "matrix"
     */
    virtual const char* getTypeName() const;

    /**
     * return matrix-row as a vector.
     * This method works fast, since it returns a reference to the row vector.
     * The data will NOT be copied.
     * @param row the row to be accessed
     * @return a reference to the vector row
     */
    inline vector<T>& getRow(const int row);

    /**
     * return matrix-row as a const vector.
     * This method works fast, since it returns a reference to the row vector.
     * The data will NOT be copied.
     * @param row the row to be accessed
     * @return a const reference to the vector row
     */
    inline const vector<T>& getRow(const int row) const;

    /**
     * alias for getRow()
     */
    inline vector<T>& operator[](const int row);

    /**
     * alias for getRow()
     */
    inline const vector<T>& operator[](const int row) const;

    /**
     * return matrix-row as a vector.
     * This method copies the data of the matrix, therefore is not as
     * fast as getRow()
     * @param row the number of tthe row to be copied
     * @return a vector with the contents of the row of the matrix
     */
    inline vector<T> getRowCopy(const int row) const;

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
    inline void getRowCopy(const int row,vector<T>& theRow) const;

    /**
     * return matrix-column as a vector.
     * This method copies the data of the matrix, therefore is not as
     * fast as getRow()
     * @param col the number of the column to be copied
     * @return a vector with the contents of the column of the matrix
     */
    inline vector<T> getColumnCopy(const int col) const;

    /**
     * return genericMatrix-column as a vector.
     * This method copies the data of the genericMatrix, therefore is not as
     * fast as getRow()
     * @param col the number of the column to be copied
     * @param theCol a vector, where the column vector of the genericMatrix
     *               should be copied.
     */
    inline void getColumnCopy(const int col,vector<T>& theCol) const;

    /**
     * Return the diagonal elements of the matrix as vector.
     * This method copies the diagonal elements of the matrix into
     * the vector. If the matrix is non-symmetrical, the vector will
     * be of dimension min(rows(),columns()).
     * @return a vector with the diagonal elements of the matrix.
     */
    inline vector<T> getDiagonal() const;

    /**
     * Return the diagonal elements of the genericMatrix as vector.
     * This method copies the diagonal elements of the genericMatrix into
     * the vector. If the genericMatrix is non-symmetrical, the vector will
     * be of dimension min(rows(),columns()).
     * @param diag a vector, where the diagonal of the genericMatrix
     *             should be copied.
     */
    inline void getDiagonal(vector<T>& diag) const;

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected matrix.  I.e. you cannot
     * copy the sub-matrix property of another matrix.
     *
     * @param other the other matrix to be copied
     */
    inline matrix<T>& copy(const matrix<T>& other);

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected matrix.  I.e. you cannot
     * copy the sub-matrix property of another matrix.
     *
     * @param other the other matrix to be copied
     * @param fromRow initial row of the other matrix to be copied
     * @param toRow last row to be copied of the other matrix
     * @param fromCol initial column of the other matrix to be copied
     * @param toCol last column to be copied of the other matrix
     */
    inline matrix<T>& copy(const matrix<T>& other,
                           const int fromRow,
                           const int toRow=MaxInt32,
                           const int fromCol=0,
                           const int toCol=MaxInt32);

    /**
     * assigment operator.
     *
     * copy the contents of <code>other</code> in this object.
     *
     * The result of the copy is always a connected matrix.  I.e. you cannot
     * copy the sub-matrix property of another matrix.
     *
     * @param other the other matrix to be copied
     * @param window rectangle define the copy area
     */
    inline matrix<T>& copy(const matrix<T>& other,const irectangle& window);

    /**
     * assigment operator.
     *
     * copy the contents of the specified rows/columns of
     * <code>other</code> into this object. Multiple occurence of one
     * row/column index in <code>idx</code> is allowed. If the
     * argument \b rows is true, \b idx specifies rows, if false \b
     * idx specifies columns.
     *
     * The result of the copy is always a connected matrix.  I.e. you cannot
     * copy the sub-matrix property of another matrix.
     *
     * @param other the other matrix to be copied
     * @param idx indices of the rows to be copied.
     * @param rows if true works on rows, else on columns
     */
    inline matrix<T>& copy(const matrix<T>& other, const vector<int>& idx,
                           bool rows=true);

    /**
     * copy the <code>other</code> matrix by casting each of its elements
     * @param other The matrix to be casted
     * Example:
     * \code
     *   lti::matrix<int> matA(10,10,1);// a matrix of integers
     *   lti::matrix<double> matB;      // a matrix of doubles
     *
     *   matB.castFrom(matA);          // this will copy matA in matB!!
     * \endcode
     */
    template<class U>
    matrix<T>& castFrom(const matrix<U>& other) {
      resize(other.rows(),other.columns(),T(),false,false);
      int y;
      for (y=0;y<this->rows();y++) {
        this->getRow(y).castFrom(other.getRow(y));
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
     * create a clone of this matrix
     * @return a pointer to a copy of this matrix
     */
    virtual mathObject* clone() const;

    /**
     * compare this matrix with other, and use the given tolerance to
     * determine if the value of each element of the other matrix
     * approximately equals the values of the actual matrix elements.
     *
     * An element <i>x</i> is approximately equal to another element <i>y</i>
     * with a tolerance <i>t</i>, if following equation holds:
     * <i>x</i>-t < <i>y</i> < <i>x</i>+t
     *
     * @param other the other matrix to be compared with
     * @param tolerance the tolerance to be used
     *
     * @return true if both matrices are approximatly equal
     */
    bool prettyCloseTo(const matrix<T>& other,const T& tolerance) const;

    /**
     * assigment operator (alias for copy(other)).
     * @param other the matrix to be copied
     * @return a reference to the actual matrix
     */
    inline matrix<T>& operator=(const matrix<T>& other);

    /**
     * @name Apply Methods
     * Following methods are used to apply simple functions to each element
     * of the vector.
     */
    //@{

#ifndef SWIG
    /**
     * applies a C-function to each element of the matrix.
     * @param function a pointer to a C-function
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(T (*function)(T));

    /**
     * applies a C-function to each element of the other matrix
     * @param other the matrix which elements will go through the given
     *              function.
     * @param function a pointer to a C-function
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& other,T (*function)(T));

    /**
     * applies a C-function to each element of the matrix.
     * @param function a pointer to a C-function
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(T (*function)(const T&));

    /**
     * applies a C-function to each element of the other matrix
     * @param other the matrix which elements will go through the given
     *              function.
     * @param function a pointer to a C-function
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& other,T (*function)(const T&));

    /**
     * a two-parameter C-function receives the i-th elements of this
     * and the given matrix and the result will be left in this
     * matrix.  Note that both matrices must have the same size!
     * @param other the second matrix to be considered (the first
     *              matrix will be this object!)
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& other,T (*function)(const T&,const T&));

    /**
     * a two-parameter C-function receives the i-th elements of this
     * and the given matrix and the result will be left in this
     * matrix.  Note that both matrices must have the same size!
     * @param other the second matrix to be considered (the first
     *              matrix will be this object!)
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& other,T (*function)(T,T));

    /**
     * a two-parameter C-function receives the i-th elements of both given
     * matrices and leaves the result here.
     * Note that both matrices must have the same size!
     * @param a the first matrix
     * @param b the second matrix
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& a,
                     const matrix<T>& b,
                     T (*function)(const T&,const T&));

    /**
     * a two-parameter C-function receives the i-th elements of both given
     * matrices and leaves the result here.
     * Note that both matrices must have the same size!
     * @param a the first matrix
     * @param b the second matrix
     * @param function a pointer to C-function with two parameters
     * @return a reference to the actual matrix
     */
    matrix<T>& apply(const matrix<T>& a,
                     const matrix<T>& b,
                     T (*function)(T,T));

    //@}
#endif // SWIG

    /**
     * @name Arithmetical Operations
     */
    //@{

    /**
     * add <code>other</code> matrix to this matrix, and leave result here
     * @param other the matrix to be added with
     * @return a reference to the actual matrix
     */
    matrix<T>& add(const matrix<T>& other);

    /**
     * add  matrices <code>a</code> and <code>b</code> and write the result
     * in this matrix
     * @param a the first matrix
     * @param b the second matrix
     * @return a reference to the actual matrix
     */
    matrix<T>& add(const matrix<T>& a, const matrix<T>& b);

    /**
     * add constant value to this matrix, and leave result here
     * @param value a constant value to be added to all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& add(const T value);

    /**
     * add constant value to the other matrix and leave result here.
     * @param other the other matrix
     * @param value a constant value to be added to all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& add(const matrix<T>& other,const T value);

    /**
     * alias for add(const T value)
     * @param value a constant value to be added to all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& operator+=(const T value) {return add(value);};

    /**
     * alias for add(const matrix)
     * @param other the matrix to be added with
     * @return a reference to the actual matrix
     */
    matrix<T>& operator+=(const matrix<T>& other) {return add(other);};

    /**
     * add <code> other</code> matrix to this matrix and leave the result in
     * a new matrix.  This object is not changed.
     * @param other the other matrix to be added with.
     * @return a matrix with the sum of this matrix and <code>other</code>
     *
     * Note that the use of this operator is not as efficient as the use of
     * the add() methods, in which the programmer can decide when to use
     * temporal and when not...
     */
    matrix<T> operator+(const matrix<T>& other) const;

    /**
     * add constant value to this matrix, and leave result in a new
     * matrix. This object is not changed.
     * @param value a constant value to be added to all matrix elements
     * @return a new matrix with the result
     */
    matrix<T> operator+(const T value) const;

    /**
     * Add another matrix scaled by \f$b\f$ to this matrix. The matrices
     * must be of the same types and dimensions. Let \f$A\f$ be this matrix
     * and \f$B\f$ the other matrix, then this method performs:<p>
     * \f$A=A+b\cdot B\f$
     * @param b scaling factor for <b>other</b>
     * @param other the matrix to be added after scaling
     * @return a reference to this matrix
     */
    matrix<T>& addScaled(const T b, const matrix<T>& other);

    /**
     * Add a matrix A with a matrix B scaled by \f$b\f$, and leave the
     * result in this matrix. The matrices must be of the same types
     * and dimensions. This method performs:<p> \f$A+b\cdot B\f$
     * @param matA the first matrix
     * @param b scaling factor for <b>matB</b>
     * @param matB the matrix to be added after scaling
     * @return a reference to this matrix
     */
    matrix<T>& addScaled(const matrix<T>& matA,
                         const T b,
                         const matrix<T>& matB);

    /**
     * Leave the scaled %sum of two matrices in this matrix. The matrices
     * must be of the same types and dimensions. Let \f$A\f$ be the
     * first matrix and \f$B\f$ the second matrix with corresponding
     * scaling factors \f$a\f$ and \f$b\f$, further \f$C\f$ this
     * matrix, then this method performs:<p>
     * \f$C=a\cdot A+b\cdot B\f$
     * @param a scaling factor for <b>first</b>
     * @param first the first matrix to be added after scaling
     * @param b scaling factor for <b>second</b>
     * @param second the second matrix to be added after scaling
     * @return a reference to this matrix
     */
    matrix<T>& addScaled(const T a, const matrix<T>& first,
                         const T b, const matrix<T>& second);


    /**
     * subtract <code>other</code> matrix from this matrix, and leave result
     * here.
     * @param other the matrix to be subtracted
     * @return a reference to this matrix.
     */
    matrix<T>& subtract(const matrix<T>& other);

    /**
     * subtract matrices <code>b</code> from <code>a</code> and write result
     * in this matrix.
     * @param a the first matrix
     * @param b the second matrix
     * @return a reference to this matrix, which is now a-b.
     */
    matrix<T>& subtract(const matrix<T>& a, const matrix<T>& b);

    /**
     * subtract constant value from this matrix, and leave result here
     * @param value a constant value to be subtracted from all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& subtract(const T value);

    /**
     * subtract constant value from the other matrix and leave result here.
     * @param other the other matrix
     * @param value a constant value to be subtracted from all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& subtract(const matrix<T>& other,const T value);

    /**
     * alias for subtract(const T value)
     * @param value a constant value to be subtracted from all matrix elements
     * @return a reference to the actual matrix
     */
    matrix<T>& operator-=(const T value) {return subtract(value);};

    /**
     * alias for subtract(const matrix)
     * @param other the matrix to be subtracted 
     * @return a reference to the actual matrix
     */
    matrix<T>& operator-=(const matrix<T>& other) {return subtract(other);};

    /**
     * multiply this matrix with <code>other</code> matrix, and leave result
     * here. The dimensions of this matrix will change if needed!
     * @param other the other matrix to be multiplied with.
     * @return a reference to this matrix.
     */
    matrix<T>& multiply(const matrix<T>& other);

    /**
     * multiply <code>first</code> with <code>second</code> and leave the
     * result in this object.
     * @param first the first matrix
     * @param second this second matrix will be multiplied with the first one.
     * @return a reference to this matrix, which has now the multiplication
     *         result.
     */
    matrix<T>& multiply(const matrix<T>& first,
                        const matrix<T>& second);

    /**
     * multiply this matrix with a vector and leave the result in
     * <code>result</code>.
     * A reference to <code>result</code> is returned.
     * @param other the vector to be multiplied with.  Its dimension must be
     *              equal to the number of columns of the matrix.
     * @param result the resulting vector.  It will have a number of
     *              dimensions equal to the number of rows of the matrix.
     * @return a reference to the result
     */
    vector<T>& multiply(const vector<T>& other,
            vector<T>& result) const;

    /**
     * multiply this matrix with a vector and leave the result in
     * same vector (In-Place Method)
     * A reference to the vector is returned.
     * @param srcdest the vector to be multiplied with.  Its dimension must be
     *              equal to the number of columns of the matrix.  The result
     *              vector will be left here too, and the resulting size
     *              will be the number of rows of the matrix.
     * @return a reference to the result
     */
    vector<T>& multiply(vector<T>& srcdest) const;

    /**
     * multiply constant value with this matrix, and leave result here
     * @param value the constant value to be multiplied with
     * @return a reference to the actual matrix
     */
    matrix<T>& multiply(const T value);

    /**
     * multiply constant value with the other matrix and leave result here.
     * @param other the other matrix
     * @param value the constant value to be multiplied with
     * @return a reference to the actual matrix
     */
    matrix<T>& multiply(const matrix<T>& other,const T value);

    /**
     * alias for multiply(const matrix)
     * @param other the other matrix to be multiplied with.
     * @return a reference to this matrix.
     */
    matrix<T>& operator*=(const matrix<T>& other) {
      return multiply(other);
    };

    /**
     * alias for multiply(const T& value)
     * @param value the constant value to be multiplied with
     * @return a reference to this matrix.
     */
    matrix<T>& operator*=(const T value) {return multiply(value);};

    /**
     * multiply the vector <code>vct</code> with this matrix, and save
     * the result in the vector <code>result</code>.
     *
     * The given vector will be interpreted as a row-vector (or a transposed
     * column vector).
     *
     * @param vct the vector to be multiplied with.
     * @param result the resulting vector.
     * @return a reference to the result vector
     */
    vector<T>& leftMultiply(const vector<T>& vct,
                                  vector<T>& result) const;

    /**
     * multiply the given vector <code>srcdest</code> with this
     * matrix, and save the result in the same vector.
     *
     * The given vector will be interpreted as a row-vector (or a
     * transposed column vector).
     *
     * @param srcDest the vector to be multiplied with, and where the
     *                result must be written.
     * @return a reference to the result vector
     */
    vector<T>& leftMultiply(vector<T>& srcDest) const;

    /**
     * multiply the matrix \a mat with \a this matrix, and leave
     * the result in \a this matrix.
     *
     * @param mat the matrix to be multiplied with.
     * @return a reference to \a this  matrix
     */
    matrix<T>& leftMultiply(const matrix<T>& mat);

    /**
     * divide the elements of the matrix by a constant value, and leave
     * the result here
     * @param value the constant value (divisor)
     * @return a reference to the actual matrix
     */
    matrix<T>& divide(const T value);

    /**
     * divide the elements of the other matrix by a constant value and leave
     * the result here.
     * @param other the other matrix
     * @param value the constant value (divisor)
     * @return a reference to the actual matrix
     */
    matrix<T>& divide(const matrix<T>& other,const T value);

    /**
     * alias for divide(const T& value)
     * @param value the constant value to be divided by
     * @return a reference to this matrix.
     */
    matrix<T>& operator/=(const T value) {return divide(value);};

    /**
     * element-wise multiplication with other matrix
     * @param other the other matrix.  It should have the same dimensions of
     *              this matrix.
     * @return a reference to the actual matrix
     */
    matrix<T>& emultiply(const matrix<T>& other);

    /**
     * element-wise multiplication of <code>a</code> and <code>b</code>.
     * @param a the first matrix
     * @param b the second matrix (with same dimensions of a)
     * @return a reference to the actual matrix
     */
    matrix<T>& emultiply(const matrix<T>& a, const matrix<T>& b);

    /**
     * element-wise division with other matrix
     * @param other the other matrix.  It should have the same dimensions of
     *              this matrix.
     * @return a reference to the actual matrix
     */
    matrix<T>& edivide(const matrix<T>& other);

    /**
     * element-wise division of <code>a</code> and <code>b</code>.
     * @param a the first matrix
     * @param b the second matrix (with same dimensions of a)
     * @return a reference to the actual matrix
     */
    matrix<T>& edivide(const matrix<T>& a, const matrix<T>& b);

    /**
     * outer-product of two vectors.
     * The result will be left in this matrix.
     * The dimensions of this matrix will change if needed.
     * The outer product of two column vectors is defined as
     * \f$a \cdot b^T\f$
     * @param a first vector (will determine the number of rows)
     * @param b second vector (will determine the number of columns)
     * @return a reference to the actual matrix
     */
    matrix<T>& outerProduct(const vector<T>& a,
                            const vector<T>& b);

    /**
     * transpose matrix and leave the result here.
     * @return a reference to the actual (now transposed) matrix
     */
    matrix<T>& transpose();

    /**
     * transpose the other matrix and leave the result here.
     * @return a reference to the actual (now transposed) matrix
     */
    matrix<T>& transpose(const matrix<T>& other);

    /**
     * calculate the sum of all elements of the matrix.
     * This member can be used with classes which define the operator '+='
     */
    T sumOfElements() const;

    /**
     * get the smallest element of the matrix
     */
    T minimum() const;

    /**
     * get the index of the smallest element of the matrix
     */
    ipoint getIndexOfMinimum() const;

    /**
     * get the biggest element of the matrix
     */
    T maximum() const;

    /**
     * get the index of the biggest element of the matrix
     */
    ipoint getIndexOfMaximum() const;

    /**
     * get the extremes of the matrix (smallest and biggest elements)
     */
    void getExtremes(T& theMinimum, T& theMaximum) const;

    /**
     * get the indices of the extremes of the matrix
     * (smallest and biggest elements)
     */
    void getIndexOfExtremes(point& theIdxMinimum, ipoint& theIdxMaximum) const;

    /**
     * Set the diagonal of this matrix to scale (default: 1), and all other
     * elements to 0. If the matrix is square (which it need not be), this
     * results in a scaled identity matrix.
     */
    void setIdentity(const T scale = T(1));

    /**
     * returns the trace (i.e. the sum of the diagonal elements) of this
     * matrix. If the matrix is not symmetrical, it will return the sum
     * of all elements (i,i) with i from 0 to n-1;
     * n being min(rows(),columns())
     */
    T trace() const;

    //@}
  protected:
    /**
     * Allocate \a n number of rows or the appropriate type.
     */
    inline virtual genericVector<T>* allocRows(const int n);
   
  };
}

#include "ltiMatrix_inline.h"

namespace lti {
  /**
   * Matrix of double
   */
  typedef matrix<double> dmatrix;
  /**
   * Matrix of float
   */
  typedef matrix<float>  fmatrix;
  /**
   * Matrix of integer
   */
  typedef matrix<int>    imatrix;

}

#endif

