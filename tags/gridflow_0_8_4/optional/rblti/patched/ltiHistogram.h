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
 * file .......: ltiHistogram.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 29.06.00
 * revisions ..: $Id$
 */

#ifndef _LTI_HISTOGRAM_H_
#define _LTI_HISTOGRAM_H_

#include "ltiVector.h"
#include "ltiMatrix.h"

namespace lti {
  /**
   *  Histogram template class.
   *
   * The lti::thistogram template class allows the generation of
   * histogram statistics for points in a n-dimensional space.
   *
   * The elements of the histogram will be indexed in each dimension \e i
   * between 0 an \f$n_i-1\f$.
   *
   * The class histogram is a typedef of thistogram<double>
   *
   * The use of this class for the one and two dimensional histograms
   * is not very efficient.  You can use the inherited classes
   * lti::histogram1D and lti::histogram2D instead.
   *
   * The mapping of the analysed m-dimensional space to the
   * n-dimensional index space will be done by mapping functors.
   * The most usual ones are declared as internal types of
   * the class lti::histogram.  @see lti::linearMapperFunctor,
   * lti::linearSatMapperFunctor
   *
   *
   *  Example:
   *  \code
   *    lti::thistogram<float> hist(3,10) // creates a 3D-histogram with 10
   *                              // cells pro dimension.  The total
   *                              // number of cells is 10^3=1000
   *  \endcode
   *
   * To read the content of a histogram cell use the access operator
   * at().
   * You can increment the value of a cell with the member put().
   *
   * @ingroup gAggregate
   */
  template<class T>
  class thistogram : public mathObject {
  
  #ifdef SWIG
  #ifdef SWIGRUBY
  %rename (ltibegin) begin();
  %rename (ltiend) end();
  %rename(init) initialize(const T& = T(0) );
  #endif
  #endif
  
  public:

    /**
     *  Variable with a negative value to indicate outer bounds access!
     */
    static inline const T& outerBoundsCell();

    typedef T value_type;

    /**
     *  The iterator is equivalent to a lti::vector<T>::iterator
     */
    typedef typename vector<T>::iterator iterator;

    /**
     *  The const_iterator is equivalent to a lti::vector<T>::const_iterator
     */
    typedef typename vector<T>::const_iterator const_iterator;

    /**
     * default constructor creates an empty histogram;
     */
    thistogram();

    /**
     * create a histogram of the given dimensionality.
     *
     * Each dimension will have the given number of cells, i.e. the histogram
     * will have \f$cells^{dimensions}\f$ cells.
     *
     * @param dimensions the dimensionality of the histogram.
     * @param cells the number of cells per dimension.
     */
    thistogram(const int& dimensions,const int& cells);

    /**
     * create a histogram with the given dimensionality.
     *
     * Each dimension \e i will have the number of cells indicated in
     * the \e i-th element of the vector <code>cells</code>.
     *
     * If the dimensionaly differs from the size of the given vector, the
     * number of cells of the dimension \e i will be given by
     * \f$dim_i = cells[i \, mod \, cells.size()]\f$.
     *
     * This means, if you want a 6-dimensional histogram, and your cells-vector
     * has only three elements [10,15,5], the number of cells per dimension
     * will be [10,15,5,10,15,5]
     *
     * @param dimensions the dimensionality of the histogram
     * @param cells a vector with the number of cell per dimension
     */
    thistogram(const int& dimensions,const ivector& cells);

    /**
     * create this histogram as a copy of another histogram
     * @param other the histogram to be copied.
     */
    thistogram(const thistogram<T>& other);

    /**
     * destructor
     */
    virtual ~thistogram();

    /**
     * returns the name of this class: "histogram"
     */
    const char* getTypeName() const {return "histogram";};

    /**
     * returns the number of dimensions of this histogram
     */
    inline int dimensions() const;

    /**
     * get the number of cells of the dimension <code>dim</code>
     * @param dimension the index of the dimension to be checked
     * @return the number of cells of the dimension specified by
     *         <code>dim</code>
     */
    inline int cellsInDimension(const int& dimension) const;

    /**
     * get the number of cells per dimension
     */
    inline const ivector& cellsPerDimension() const;

    /**
     * returns a vector to the first element of the histogram
     * (usually every element of the vector is 0;
     */
    inline const ivector& getFirstCell() const;

    /**
     * returns a vector to the last element of the histogram
     */
    inline const ivector& getLastCell() const;

    /**
     * returns an iterator pointing to the first element.
     * Note that you can not change the values of the histogram
     * elements when you use a const_iterator. See also begin()
     */
    inline const_iterator begin() const {
      return theHistogram.begin();  
    };

    /**
     * returns an iterator pointing to the first element.
     * The use of the interators is similar to the iterators of the
     * Standard Template Library (STL).
     *
     * If you need to iterate on all elements of the histogram, you can
     * use following code:
     * \code
     *   int tmp,accu;                          // a temporal variable
     *   lti::thistogram<float> myHist(3,10);   // a 3D-histogram with 10 cells/dim. (of type float)
     *   lti::thistogram<float>::iterator it;   // an iterator
     *
     *   for (it=myHist.begin();it!=myHist.end();it++) {
     *     tmp = *it;                           // tmp has value of element pointed
     *                                          // by the iterator.
     *     accu += tmp;
     *     (*it) = accu;                        // change the value in the histogram.
     *   }
     * \endcode
     * Please note that if you define <code>it</code> as a const_iterator,
     * you can not make something like <code>*it=accu</code>.
     */
    inline iterator begin() {
      return theHistogram.begin();
    }

    /**
     * returns last index as a const iterator.
     * For an example see begin()
     */
    inline const_iterator end() const {
      return theHistogram.end();
    };

    /**
     * returns last index as an iterator
     * For an example see begin()
     */
    inline iterator end() {
      return theHistogram.end();
    };

    /**
     * change dimensionality and cell number of the histogram.  All data will
     * be lost!
     *
     * @param dimensions the new dimensionality of the histogram
     * @param cells      the number of cells per dimension
     *
     */
    void resize(const int& dimensions,
                const int& cells);

    /**
     * change dimensionality and cell number of the histogram.  All data will
     * be lost!
     *
     * @param dimensions the new dimensionality of the histogram
     * @param cells      the number of cells per dimension
     *
     */
    void resize(const int& dimensions,
                const ivector& cells);

    /**
     * equivalent to resize(0,0);
     */
    void clear();

    /**
     * initialize all cells of the histogram with 0 (or another specified
     * number).
     */
    void initialize(const T& value = T(0));

    /**
     * returns the number of entries registered by now.
     */
    inline const T& getNumberOfEntries() const;

    /**
     * Normalize the histogram and then denormalize it with the given number
     * of entries.
     *
     * You should not use this for thistogram<int>, since rounding
     * errors are likely to occur.
     */
    void setNumberOfEntries(const T& newNumberOfEntries);

    /**
     * counts the number of entries in the whole histogram and sets
     * the internal counter for the total number of entries.
     * if some direct access to the cell contents
     * have been done, you should update the number of entries with this
     * function
     */
    void updateNumberOfEntries();

    /**
     * fills the histogram elements with <code>iniValue</code> between
     * the n-dimensional points <code>from</code> and <code>to</code>.
     * @param iniValue the elements will be initialized with this
     *                 value.
     * @param from     first element index
     * @param to       last element index
     *
     * If <code>from</code> or <code>to</code> are out of bounds,
     * they will be (internaly) adjusted to a correct value.
     *
     * Example:
     * \code
     *   lti::thistogram<float> hist(1,10);         // 1D-histogram with 10 elements
     *   hist.clear;
     *   hist.fill(9.0f,ivector(1,1),ivector(1,3)); // hist=[0,9,9,9,0,0,0,0,0,0]
     * \endcode
     */
    void fill(const T& iniValue,
              const ivector& from=ivector(),
              const ivector& to=ivector());

    /**
     * read-only access to the element x of the histogram
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    const T& at(const ivector& x) const;

    /**
     * access element x of the histogram
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    T& at(const ivector& x);

    /**
     * increment the cell at \e x by the given number of elements (or 1.0 if
     * nothing is explicitly indicated!) and update the number of entries in
     * the histogram.
     * @param           x index of the histogram element to be incremented
     * @param increment amount of the incrementation (default: 1)
     * @return the new number of entries of the incremented cell, or 0
     *         if the index lies outer bounds
     */
    const T& put(const ivector& x,const T& increment=T(1));

    /**
     * read-only access to the element x of the histogram as a discrete
     * probability distribution term.  This is equivalent to \f$at(x)/n\f$,
     * where \e n is the number of entries in the histogram (see
     * getNumberOfEntries()).
     *
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the probability of the element x, respect to the histogram
     *         data, or 0 if <code>x</code> lies outer bounds.
     */
    double getProbability(const ivector& x) const;

    /**
     * @name Copy and Duplication methods
     */
    //@{

    /**
     * assigment operator.
     * copy the contents of <code>other</code> in this %object.
     * @param other the source histogram to be copied.
     * @return a reference to this object
     */
    thistogram<T>& copy(const thistogram<T>& other);

    /**
     * assigment operator (alias for copy(other)).
     * @param other the histogram to be copied
     * @return a reference to the actual histogram
     */
    inline thistogram<T>& operator=(const thistogram<T>& other) {
      return copy(other);
    };

    /**
     * copy the <code>other</code> histogram by casting each of its elements
     * @param other The histogram to be casted
     */
    template<class U>
    thistogram<T>& castFrom(const thistogram<U>& other) {
      resize(other.dimensions(), other.cellsPerDimension());

      typename thistogram<U>::const_iterator otherIt = other.begin();
      iterator thisIt = begin();

      while (! (otherIt == other.end())) {
        *thisIt = static_cast<T>(*otherIt);
        otherIt++;
        thisIt++;
      }

      updateNumberOfEntries();

      return (*this);
    };

    /**
     * create a clone of this histogram
     * @return a pointer to a copy of this histogram
     */
    virtual mathObject* clone() const;

    /** 
     * Free the data of this object and attach it to the "receiver".
     *
     * It is a very efficient way to make a copy of this histogram, if you
     * don't need the source data anymore!
     *
     * At the end of the detachment, this histogram will be empty.
     * @param receiver the histogram which will receive the memory.
     *                 this histogram!
     */
    void detach(thistogram<T>& receiver);
    //@}

    /**
     * @name Comparison methods
     */
    //@{

    /**
     * compare this histogram with another one.
     *
     * @param other the other histogram to be compared with
     * @return true if both histograms have the same elements and same size
     */
    bool equals(const thistogram<T>& other) const;

    /** compare this histogram with other
     *
     * @param other the other histogram to be compared with
     * @return true if both histograms have the same elements and same size
     */
    inline bool operator==(const thistogram<T>& other) const {
      return equals(other);
    };

    /**
     * compare this histogram with another one, and use the given tolerance to
     * determine if the value of each element of the other histogram
     * approximately equals the values of the actual histogram elements.
     *
     * An element \e x is approximately equal to another element \e y
     * with a tolerance \e t, if following equation holds:
     * <i>x</i>-t < <i>y</i> < <i>x</i>+t
     *
     * @param other the other histogram to be compared with
     * @param tolerance the tolerance to be used
     *
     * @return true if both histograms are approximatly equal
     */
    bool prettyCloseTo(const thistogram<T>& other,
                       const T& tolerance) const;


    //@}

    /**
     * Apply methods
     */
    //@{
    /**
     * applies a C-function to each element of the histogram.
     * @param function a pointer to a C-function
     * @return a reference to the actual histogram
     */
    thistogram<T>& apply(T (*function)(T));

    /**
     * applies a C-function to each element of the histogram.
     * @param function a pointer to a C-function
     * @return a reference to the actual histogram
     */
    thistogram<T>& apply(T (*function)(const T&));
    //@}

    /**
     * @name Arithmetical operations
     */
    //@{

    /**
     * Elementwise multiplication.
     * Each element of this histogram will be multiplied with the elements
     * of the other histogram and the result will be left in this %object!
     *
     * Both histograms are first normalized, then multiplied, and the number of
     * entries is after the multiplication 1!  After this multiplication, this
     * histogram cannot be anymore interpreted as a histogram, but as a
     * combined probabilty distribution.  You can use setNumberOfEntries() to
     * change this fact under your own risk (the semathical meaning of that is
     * left to you!).
     *
     * You should not use this with thistogram<int>.
     *
     * The returned histogram is this %object!
     *
     * @param other the other histogram to be multiplied with
     * @return a reference to the actual histogram
     */
    thistogram<T>& emultiply(const thistogram<T>& other);

    /**
     * Elementwise multiplication.
     * This histogram will contain the elementwise multiplication of the
     * elements in <code>first</code> and <code>second</code>.
     *
     * Both histograms are first normalized, then multiplied, and the number of
     * entries is after the multiplication 1!  After this multiplication, this
     * histogram cannot be anymore interpreted as a histogram, but as a
     * combined probabilty distribution.  You can use setNumberOfEntries() to
     * change this fact under your own risk (the semathical meaning of that is
     * left to you!).
     *
     * You should not use this with thistogram<int>.
     *
     * @param first the first histogram
     * @param second the second histogram will be multiplied with the
     *               first histogram
     * @return a reference to the actual histogram
     */
    thistogram<T>& emultiply(const thistogram<T>& first,
                                 const thistogram<T>& second);

    /**
     * Add another histogram of the same type and same dimension and
     * leave the result in this %object.  The number of entries of both
     * histograms are also added.
     *
     * @param other the other histogram to be added with
     * @return a reference to the actual histogram
     */
    thistogram<T>& add(const thistogram<T>& other);

    /**
     * Add two histogram and leave the result in this %object.
     * @param first the first histogram.  The number of entries of both
     * histograms are also added.
     * @param second the second histogram will be added with the first
     *               histogram
     * @return a reference to the actual histogram
     */
    thistogram<T>& add(const thistogram<T>& first,const thistogram<T>& second);

    /// Alias for add(const histogram& other)
    inline thistogram<T>& operator+=(const thistogram<T>& other) {
      return add(other);
    };

    /**
     * Subtracts another histogram of the same type and same dimension
     * and leaves the result in this %object
     *
     * @param other will be substracted from this histogram
     * @return a reference to the actual histogram
     */
    thistogram<T>& subtract(const thistogram<T>& other);

    /**
     * Subtracts two histograms and leaves the result in this %object.
     * @param first the first histogram
     * @param second the second histogram will be substracted from the
     *                   first histogram
     * @return a reference to the actual histogram
     */
    thistogram<T>& subtract(const thistogram<T>& first,
                           const thistogram<T>& second);

    /**
     * Alias for substract(const histogram& other)
     */
    inline thistogram<T>& operator-=(const thistogram<T>& other) {
      return subtract(other);
    };

    /**
     * Multiply this histogram with a constant.
     * Returns this histogram.  The total number of entries will also be
     * updated. Note that if you use this operation, the number of entries
     * could be false at the end due to numerical instabilities.
     * @see setNumberOfEntries()
     *
     * @param cst constant scalar to be multiplied with
     * @return a reference to the actual histogram
     */
    thistogram<T>& multiply(const T& cst);

    /**
     * Multiply the other %histogram with a constant and leave the result here.
     * Returns a reference to this histogram.
     * Note that if you use this operation, the number of entries
     * could be false at the end due to numerical instabilities.
     * @see setNumberOfEntries()
     *
     * @param other the other histogram to be multiplied with the
     *              constant value
     * @param cst constant scalar to be multiplied with the other histogram.
     * @return a reference to the actual histogram
     */
    thistogram<T>& multiply(const thistogram<T>& other,const T& cst);

    /**
     * alias for multiply(const T& cst)
     * @param cst constant scalar to be multiplied with
     * @return a reference to the actual histogram
     */
    inline thistogram<T>& operator*=(const T& cst) {
      return multiply(cst);
    };

    /**
     * Divide this histogram with a constant.
     *
     * Returns this histogram.
     *
     * Keep in mind, that rounding errors might occur if you use this
     * with thistogram<int>.
     *
     * @param cst the elements of the histogram will be divided with this
     *            constant
     * @return a reference to the actual histogram
     */
    thistogram<T>& divide(const T& cst);

    /**
     * Divide the other histogram with a constant and leave the result here.
     *
     * Returns a reference to this histogram.
     *
     * Keep in mind, that rounding errors might occur if you use this
     * with thistogram<int>.
     *
     * @param other the histogram to be divide by the constant value
     * @param cst the elements of the histogram will be divided with this
     *            constant
     * @return a reference to the actual histogram
     */
    thistogram<T>& divide(const thistogram<T>& other,const T& cst);

    /**
     * Add constant to this histogram.  This histogram is changed.
     * Returns this histogram.
     * @param cst constant scala to be added with each element
     * @return a reference to the actual histogram
     */
    thistogram<T>& add(const T& cst);

    /**
     * Alias for add(const T& cst)
     */
    thistogram<T>& operator+=(const T& cst) {
      return add(cst);
    }

    /**
     * Add constant to the other histogram and leave the result here.
     * Returns a reference to this histogram.
     * @param other the oder histogram
     * @param cst constant scala to be added with each element of the other
     *            histogram
     * @return a reference to the actual histogram
     */
    thistogram<T>& add(const thistogram<T>& other,const T& cst);

    /**
     * Normalize the histogram
     *
     * The default behaviour just divides the content of each bin by the
     * current number of entries.  Since some operations (especially if you
     * use apply()) can "corrupt" the consistence of this internal attribute,
     * you can additionally specify with the boolean parameter, that you want
     * to force the recomputation the number of entries.
     * 
     * The total number of entries will be set to 1.0
     * You should not use this with thistogram<int>.
     *
     * @param forceUpdateOfNumEntries if true, the number of entries will be
     *                                recomputed, ignoring their previous
     *                                content.  If false (default), the current
     *                                value will be used.
     * 
     * @return a reference to the actual histogram
     */
    thistogram<T>& normalize(const bool forceUpdateOfNumEntries=false);

    //@}

    /**
     * @name Search for extrema
     */
    //@{
    /**
     * search for the maximum entry in the histogram and return its value
     */
    inline T maximum() const {
      return theHistogram.maximum();
    }

    /**
     * get the index of the biggest element in the histogram
     */
    inline ivector getIndexOfMaximum() const;

    /**
     * search for the minimum entry in the histogram and return its value
     */
    inline T minimum() const {
      return theHistogram.minimum();
    }

    /**
     * get the index of the biggest element in the histogram
     */
    inline ivector getIndexOfMinimum() const;

    //@}

    /**
     * @name Serialization interface
     */
    //@{
    /**
     * write the object in the given ioHandler
     */
    virtual bool write(ioHandler& handler,const bool complete = true) const;

    /**
     * read the object from the given ioHandler
     */
    virtual bool read(ioHandler& handler,const bool complete = true);
    //@}

  protected:
    /**
     * the total number of cells
     */
    int totalNumberOfCells;

    /**
     * the registered number of entries
     */
    T numberOfEntries;

    /**
     * the dimensionality of this histogram
     */
    int dimensionality;

    /**
     * the data of the histogram
     */
    vector<T> theHistogram;

    /**
     * number of cells
     */
    ivector theCellsPerDimension;

    /**
     * a vector with the right dimension initialized with 0
     */
    ivector firstCell;

    /**
     * a vector with the right dimension initialized with the
     * number of cells - 1 per dimension
     */
    ivector lastCell;

    /**
     * compute the integer index for the data vector using the
     * given index vector.
     */
    inline int vectorToIndex(const ivector& x) const;

    /*
     * compute the index vector for the data vector using the
     * given integer index.
     */
    inline ivector indexToVector(const int x) const;
  };

  /**
   * read the matrix from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be read
   *
   * @ingroup gStorable
   */
  template <class T>
  bool read(ioHandler& handler,thistogram<T>& hist,const bool complete=true) {
    return hist.read(handler,complete);
  } // immediate implementation to avoid MSVC++ bug!!

  /**
   * write the matrix in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   *
   * @ingroup gStorable
   */
  template <class T>
  bool write(ioHandler& handler,const thistogram<T>& hist,
             const bool complete=true) {
    return hist.write(handler,complete);
  } // immediate implementation to avoid MSVC++ bug!!


  // --------------------------------------------------------------------

  typedef thistogram<double> histogram;

  // --------------------------------------------------------------------

#ifdef SWIG
    %template(histogram)  thistogram<double>;
    %template(fhistogram) thistogram<float>;
    %template(ihistogram) thistogram<int>;
#endif

  /**
   * one dimensional histogram of type double
   *
   * The implementation of the 1D histogram allows an efficient way
   * to create 1D histogram... much faster than using n-dimensional
   * histograms with dimension 1.
   *
   * @ingroup gAggregate
   */
  class histogram1D : public histogram {
  
  #ifdef SWIG
  #ifdef SWIGRUBY
  %rename(init) initialize(const value_type& = value_type(0) );
  #endif
  #endif
  
  public:

    /**
     * default constructor creates an empty histogram;
     */
    histogram1D();

    /**
     * create a one dimensional histogram of the given dimensionality.
     *
     * @param cells the number of cells.
     */
    histogram1D(const int& cells);

    /**
     * create this histogram as a copy of another histogram
     * @param other the histogram to be copied.
     */
    histogram1D(const histogram1D& other);

    /**
     * destructor
     */
    virtual ~histogram1D();

    /**
     * returns the name of this class: "histogram"
     */
    const char* getTypeName() const {return "histogram1D";};

    /**
     * Returns the index of the first histogram element, which is always zero
     */
    inline const int& getFirstCell() const;

    /**
     * Returns a vector to the last element of the histogram
     */
    inline const int& getLastCell() const;

    /**
     * Returns the total number of cells in this histogram
     *
     * This method is slower than getLastCell, since it need to make some
     * arithmetical operations.  You should use getLastCell instead.
     */
    inline int size() const;

    /**
     * change cell number of the histogram.  All data will
     * be lost! (it will be initialized with 0)
     *
     * @param cells      the number of cells per dimension
     *
     */
    void resize(const int& cells);

    /**
     * initialize all cells of the histogram with 0 (or another specified
     * number).
     */
    void initialize(const value_type& value = value_type(0));

    /**
     * fills the histogram elements with <code>iniValue</code> between
     * the n-dimensional points <code>from</code> and <code>to</code>.
     * @param iniValue the elements will be initialized with this
     *                 value.
     * @param from     first element index
     * @param to       last element index
     *
     * If <code>from</code> or <code>to</code> are out of bounds,
     * they will be (internaly) adjusted to a correct value.
     *
     * Example:
     * \code
     *   lti::histogram1D hist(10); // 1D-histogram with 10 elements
     *   hist.clear();
     *   hist.fill(9,1,3);          // hist=[0,9,9,9,0,0,0,0,0,0]
     * \endcode
     */
    void fill(const value_type& iniValue,
              const int& from=0,
              const int& to=MaxInt32);

    /**
     * read-only access to the element x of the histogram
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    inline const value_type& at(const int& x) const;

    /**
     * access element x of the histogram
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    inline value_type& at(const int& x);

    /**
     * increment the cell at \e x by the given number of elements (or 1.0 if
     * nothing is explicitly indicated!) and update the number of entries in
     * the histogram.
     * @param x         index of the histogram element to be incremented
     * @param increment amount of the incrementation (default: 1)
     * @return the new number of entries of the incremented cell.
     */
    inline const value_type& put(const int& x,
                                 const value_type& increment=1.0f);

    /**
     * read-only access to the element x of the histogram as a discrete
     * probability distribution term.  This is equivalent to \f$at(x)/n\f$,
     * where \e n is the number of entries in the histogram (see
     * getNumberOfEntries()).
     *
     * @param x index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the probabilty of the element x, respect to the histogram
     *         data.
     */
    inline value_type getProbability(const int& x) const;

    /**
     * assigment operator.
     * copy the contents of <code>other</code> in this %object.
     * @param other the source histogram to be copied.
     * @return a reference to this object
     */
    virtual histogram1D& copy(const histogram1D& other);

    /**
     * create a clone of this histogram
     * @return a pointer to a copy of this histogram
     */
    mathObject* clone() const;


    /**
     * assigment operator (alias for copy(other)).
     * @param other the histogram to be copied
     * @return a reference to the actual histogram
     */
    histogram1D& operator=(const histogram1D& other) {return copy(other);};

    /**
     * get the index of the biggest element in the histogram
     */
    inline int getIndexOfMaximum() const;

    /**
     * get the index of the smallest element in the histogram
     */
    inline int getIndexOfMinimum() const;

    /**
     * write the object in the given ioHandler
     */
    virtual bool write(ioHandler& handler,const bool complete = true) const;

    /**
     * read the object from the given ioHandler
     */
    virtual bool read(ioHandler& handler,const bool complete = true);

  protected:

    /**
     * first index ( 0 )
     */
    int firstCell;

    /**
     * last index (cellsInDimension(0) - 1)
     */
    int lastCell;

  };

  // --------------------------------------------------------------------


  /**
   * two dimensional histogram of type double
   *
   * The implementation of the 2D histogram allows an efficient way
   * to create 2D histograms... much faster than using n-dimensional
   * histograms with dimension 2.
   *
   * @ingroup gAggregate
   */
  class histogram2D : public histogram {
  
  #ifdef SWIG
  #ifdef SWIGRUBY
  %rename(init) initialize(const value_type& = value_type(0));
  #endif
  #endif
  
  public:
    /**
     * default constructor creates an empty histogram;
     */
    histogram2D();

    /**
     * create a two dimensional histogram of \e cells x  \e cells
     *
     * @param cells the number of cells per dimension.
     */
    histogram2D(const int& cells);

    /**
     * create a two dimensional histogram of \e cellsY x \e cellsX
     *
     * Please note the use of matrix notation (y,x) and NOT (x,y)
     * @param cellsY the number of cells in the first dimension.
     * @param cellsX the number of cells in the second dimension.
     */
    histogram2D(const int& cellsY,const int& cellsX);

    /**
     * create a two dimensional histogram of \e cells.x x \e cells.y
     *
     * @param cells the number of cells per dimension.
     *        (cells.x is the number of columns of the histogram
     *         cells.y is the number of rows of the histogram)
     */
    histogram2D(const point& cells);

    /**
     * create this histogram as a copy of another histogram
     * @param other the histogram to be copied.
     */
    histogram2D(const histogram2D& other);

    /**
     * destructor
     */
    virtual ~histogram2D();

    /**
     * returns the name of this class: "histogram"
     */
    const char* getTypeName() const {return "histogram2D";};

    /**
     * returns a vector to the first element of the histogram
     * (usually every element of the vector is 0;
     */
    inline const point& getFirstCell() const;

    /**
     * returns a vector to the last element of the histogram
     */
    inline const point& getLastCell() const;

    /**
     * change cell number of the histogram2D.  All data will
     * be lost!
     *
     * @param cells the number of cells
     *
     */
    void resize(const point& cells);

    /**
     * change cell number of the histogram.  All data will
     * be lost!
     *
     * @param cellsY the number of rows
     * @param cellsX the number of columns
     *
     */
    void resize(const int& cellsY,const int& cellsX);

    /**
     * initialize all cells of the histogram with 0 (or another specified
     * number).
     */
    void initialize(const value_type& value = value_type(0));

    /**
     * fills the histogram elements with <code>iniValue</code> between
     * the n-dimensional points <code>from</code> and <code>to</code>.
     * @param iniValue the elements will be initialized with this
     *                 value.
     * @param from     first element index
     * @param to       last element index
     *
     * If <code>from</code> or <code>to</code> are out of bounds,
     * they will be (internaly) adjusted to a correct value.
     *
     * Example:
     * \code
     *   lti::histogram1D hist(10); // 1D-histogram with 10 elements
     *   hist.clear();
     *   hist.fill(9,1,3);          // hist=[0,9,9,9,0,0,0,0,0,0]
     * \endcode
     */
    void fill(const value_type& iniValue,
              const point& from=point(0,0),
              const point& to=point(MaxInt32,MaxInt32));

    /**
     * read-only access to the element (x,y) of the histogram
     *
     * Note the use of the matrix notation: first row (y) and then column (x)
     * @param y row of the histogram element to be accessed.  It should
     *          be between getFirstCell().y and getLastCell().y
     * @param x column of the histogram element to be accessed.  It should
     *          be between getFirstCell().x and getLastCell().x
     * @return the number of entries in the given cell
     */
    inline const value_type& at(const int& y, const int& x) const;

    /**
     * access element x of the histogram
     * @param y row of the histogram element to be accessed.  It should
     *          be between getFirstCell().y and getLastCell().y
     * @param x column of the histogram element to be accessed.  It should
     *          be between getFirstCell().x and getLastCell().x
     * @return the number of entries in the given cell
     */
    inline value_type& at(const int& y, const int& x);

    /**
     * read-only access to the element p of the histogram
     * @param p index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    inline const value_type& at(const point& p) const;

    /**
     * access element p of the histogram
     * @param p index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the number of entries in the given cell
     */
    inline value_type& at(const point& p);

    /**
     * increment the cell at row \e y and column \e x by the given number of
     * entries (or 1.0 if nothing is explicitly indicated!) and update the
     * total number of entries in the histogram.
     * @param y row of the histogram element to be incremented.  It should
     *          be between getFirstCell().y and getLastCell().y
     * @param x column of the histogram element to be incremented.  It should
     *          be between getFirstCell().x and getLastCell().x
     * @param increment amount of the incrementation (default: 1)
     * @return the new number of entries of the incremented cell.
     */
    inline const value_type& put(const int& y,const int& x,
                                 const value_type& increment=value_type(1));

    /**
     * increment the cell at p by the given number of
     * entries (or 1.0 if nothing is explicitly indicated!) and update the
     * total number of entries in the histogram.
     * @param p index of the histogram element to be incremented.  It should
     *          be between getFirstCell() and getLastCell()
     * @param increment amount of the incrementation (default: 1)
     * @return the new number of entries of the incremented cell.
     */
    inline const value_type& put(const point& p,
                                 const value_type& increment=value_type(1));


    /**
     * read-only access to the element p of the histogram as a discrete
     * probability distribution term.  This is equivalent to \f$at(x)/n\f$,
     * where \e n is the number of entries in the histogram (see
     * getNumberOfEntries()).
     *
     * @param p index of the histogram element to be accessed.  It should
     *          be between getFirstCell() and getLastCell()
     * @return the probabilty of the element x, respect to the histogram
     *         data.
     */
    inline value_type getProbability(const point& p) const;


    /**
     * read-only access to the element of the row y and the column x of the
     * histogram as a discrete probability distribution term.  This is
     * equivalent to \f$at(x)/n\f$, where \e n is the number of entries in the
     * histogram (see getNumberOfEntries()).
     *
     * @param y row of the histogram element to be accessed.  It should
     *          be between getFirstCell().y and getLastCell().y
     * @param x column of the histogram element to be accessed.  It should
     *          be between getFirstCell().x and getLastCell().x
     * @return the probabilty of the element (y,x), respect to the histogram
     *         data.
     */
    inline value_type getProbability(const int& y, const int& x) const;

    /**
     * assigment operator.
     * copy the contents of <code>other</code> in this %object.
     * @param other the source histogram to be copied.
     * @return a reference to this object
     */
    virtual histogram2D& copy(const histogram2D& other);

    /**
     * create a clone of this histogram
     * @return a pointer to a copy of this histogram
     */
    mathObject* clone() const;

    /**
     * assigment operator (alias for copy(other)).
     * @param other the histogram to be copied
     * @return a reference to the actual histogram
     */
    histogram2D& operator=(const histogram2D& other) {return copy(other);};

    /**
     * get the index of the biggest element in the histogram
     */
    inline point getIndexOfMaximum() const;

    /**
     * get the index of the smallest element in the histogram
     */
    inline point getIndexOfMinimum() const;

    /**
     * write the object in the given ioHandler
     */
    virtual bool write(ioHandler& handler,const bool complete = true) const;

    /**
     * read the object from the given ioHandler
     */
    virtual bool read(ioHandler& handler,const bool complete = true);


  protected:

    /**
     * first index ( 0,0 )
     */
    point firstCell;

    /**
     * last index (cellsInDimension(0) - 1,cellsInDimension(1) - 1 )
     */
    point lastCell;

    /**
     * wrapper matrix for histogram::theHistogram
     */
    matrix<value_type> theHistoMatrix;
  };

} // namespace lti


/// outputs the elements of the histogram on a stream
template <class T>
std::ostream& operator<<(std::ostream& s,const lti::thistogram<T>& v);

#include "ltiHistogram_inline.h"


#endif

