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
 * file .......: ltiPointList.h
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 02.11.00
 * revisions ..: $Id$
 */

#ifndef _LTI_POINTLIST_H_
#define _LTI_POINTLIST_H_

#include "ltiObject.h"
#include "ltiIoObject.h"
#include "ltiTypes.h"
#include "ltiPoint.h"
#include "ltiRectangle.h"
#include "ltiException.h"
#include "ltiVector.h"

#include <iostream>

#include <list>
//#include "ltiSmallObjectList.h"

namespace lti {
  /**
   * tpointList template class.
   * The %lti::tpointList class allows the storage of a list of tpoints<T>.
   * The elements of the tpointList can be accessed through iterators.
   *
   * Example:
   *
   * \code
   * lti::pointList pts;  // a list of points with integer coodinates
   *
   * // create 10 points
   * for (int i=0;i<10;++i) {
   *   pts.push_back(point(i,i));
   * }
   *
   * // iterate on the list of points to add 1 to x and 2 to y:
   * lti::pointList::iterator it;
   * for (it=pts.begin();it!=pts.end();++pts) {
   *   (*pts).add(point(1,2));
   * }
   * \endcode
   *
   * @ingroup gAggregate
   */
  template<class T>
  class tpointList : public ioObject {
  public:
    // typedef smallObjectList<tpoint<T> > list_type;
    typedef std::list<tpoint<T> > list_type;

#ifndef SWIG
    /**
     * iterator type (allows read and write operations)
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::pointList::begin()
     * for an example
     */
    typedef typename list_type::iterator iterator;

    /**
     * const iterator type (allows read-only operations)
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::pointList::begin()
     * for an example.
     */
    typedef typename list_type::const_iterator const_iterator;
#endif // SWIG

    /**
     * reference type (allows read and write operations)
     * The use of the reference classes is similar to the references of
     * the STL (Standard Template Library).
     */
    typedef typename list_type::reference reference;

    /**
     * const_reference type (allows read-only operations)
     * The use of the reference classes is similar to the references of
     * the STL (Standard Template Library).
     */
    typedef typename list_type::const_reference const_reference;

    /**
     * default constructor creates an empty pointList;
     */
    tpointList();

    /**
     * create this pointList as a copy of another pointList
     * @param other the pointList to be copied.
     */
    tpointList(const tpointList<T>& other);

    /**
     * create this pointList as a copy of a list_type of tpoint<T>
     * @param other the pointList to be copied.
     */
    tpointList(const list_type& other);

    /**
     * destructor
     */
    virtual ~tpointList();

    /**
     * returns the name of this class: tpointList<T>
     */
    const char* getTypeName() const {return "tpointList<T>";};

    /**
     * returns the number of elements of the pointList
     */
    int size() const;

    /**
     * compares the size of this list with the size of the other point list
     * and returns true if this list has fewer points than the other one.
     */
    bool operator<(const tpointList<T>& other) const;

    /**
     * compares the size of this list with the size of the other point list
     * and returns true if this list has more points than the other one.
     */
    bool operator>(const tpointList<T>& other) const;

#ifndef SWIG
    /**
     * returns first element as a const_iterator.
     * Note that you can not change the values of the pointList
     * elements when you use a const_iterator. See also begin()
     */
    const_iterator begin() const;

    /**
     * returns an iterator pointing to the first element of the pointList
     * The use of the interators is similar to the iterators of the
     * Standard Template Library (STL).
     * If you need to iterate on all elements of the pointList, you can
     * use following code:
     * \code
     * lti::tpointList<int> myPL;               // an empty pointList
     * lti::tpointList<int>::iterator it;       // an iterator
     *
     * // Fill pointList with some arbitrary points
     * for (int i=0; i<10; i++) {
     *   myPL.push_back(lti::point(0,i));
     * }
     *
     * // Swap x and y for all points in list
     * for (it=myPL.begin();it!=myPL.end();it++) {
     *   int temp;
     *   temp = (*it).x;
     *   (*it).x = (*it).y;
     *   (*it).y = temp;
     * }
     * \endcode
     */
    iterator begin();

    /**
     * returns last element as a const iterator.
     * For an example see begin()
     */
    const_iterator end() const;

    /**
     * returns last element as an iterator
     * For an example see begin()
     */
    iterator end();
#endif // SWIG

    /**
     * deletes all points from list and leaves empty pointList.
     */
    void clear();

#ifndef SWIG
    /**
     * erases point, which is denoted by it. Returns iterator to next element.
     */
    iterator erase(const iterator& it);

    /**
     * erases points between first and last. Returns iterator to next element.
     */
    iterator erase(const iterator& first, const iterator& last);

    /**
     * inserts point before position denoted by it.
     * returns iterator to inserted element.
     */
    iterator insert(const iterator& it, const tpoint<T>& thePoint);

    /**
     * inserts points before position denoted by it.
     * Returns iterator to first element of inserted elements.
     */
    iterator insert(const iterator& it,
                            const int& n,
                            const tpoint<T>& thePoint);

    /**
     * inserts the elements from first to last, before position denoted by it.
     * Returns iterator to first element of inserted elements.
     */
    iterator insert(const iterator& it,
                            const_iterator first,
                            const_iterator last);

    /**
     * Transfer all elements in the second list into this one.
     * At the end of the operation, the second list will be empty
     */
    void splice(const iterator& pos,
                tpointList<T>& other);
#endif // SWIG

    /**
     * inserts element at begin of pointList
     */
    void push_front(const tpoint<T>& thePoint);

    /**
     * removes element at begin of pointList
     */
    void pop_front();

    /**
     * inserts element at end of pointList
     */
    void push_back(const tpoint<T>& thePoint);

    /**
     * removes element at end of pointList
     */
    void pop_back();

#ifndef SWIG
	/**
     * returns a reference to the first element
     */
    reference front();

    /**
     * returns a const reference to the first element
     */
    const_reference front() const;

    /**
     * returns a reference to the last element
     */
    reference back();

    /**
     * returns a const reference to the last element
     */
    const_reference back() const;
#endif // SWIG

    /**
     * assigment operator.
     * copy the contents of <code>other</code> in this %object.
     * @param other the source pointList to be copied.
     */
    tpointList<T>& copy(const tpointList<T>& other);

    /**
     * assigment operator (alias for copy(other)).
     * @param other the pointList to be copied
     * @return a reference to the actual pointList
     */
    tpointList<T>& operator=(const tpointList<T>& other);

    /**
     * create a clone of this pointList
     * @return a pointer to a copy of this pointList
     */
    virtual object* clone() const;

    /**
     * @name Conversion Methods
     */
    //@{

    /**
     * copy the elements of the other standard list of tpoint<T> in this
     * %object
     * @param other the source pointList to be copied.
     */
    tpointList<T>& castFrom(const list_type& other);

    /**
     * copy the <code>other</code> point-list by casting each of its elements
     * @param other The point list to be casted
     */
    template<class U>
    tpointList<T>& castFrom(const tpointList<U>& other) {
      clear();
      typename tpointList<U>::const_iterator it,eit;
      for (it=other.begin(),eit=other.end();it!=eit;++it) {
        push_back(tpoint<T>(static_cast<T>((*it).x),
                            static_cast<T>((*it).y)));
      }
      return (*this);
    };

    /**
     * cast the given vector of points into a list, where the
     * first element in the vector will be the first element
     * in the list.
     * @param other the vector of points with the points
     * @return a reference to this instance
     */
    template<class U>
    tpointList<T>& castFrom(const vector< tpoint<U> >& other) {
      clear();
      typename vector< tpoint<U> >::const_iterator it,eit;
      for (it=other.begin(),eit=other.end();it!=eit;++it) {
        push_back(*it);
      }
      return *this;
    }

    /**
     * cast the given vector of points into a list, where the
     * first element in the vector will be the first element
     * in the list.
     */
    template<class U>
    tpointList<T>& castFrom(const vector< tpoint<U> >& other) const {
      clear();
      typename vector< tpoint<U> >::const_iterator it,eit;
      for (it=other.begin(),eit=other.end();it!=eit;++it) {
        push_back(*it);
      }
      return *this;
    }

    /**
     * cast this list of points into a lti::vector, which can be
     * accessed in a faster way than the list.
     */
    template<class U>
    void castTo(vector< tpoint<U> >& other) const {
      other.resize(size(),tpoint<U>(),false,false);
      typename tpointList<T>::const_iterator it;
      int i;
      for (i=0,it=begin();i<other.size();++i,++it) {
        other.at(i).copy(*it);
      }
    }
    //@}

    /**
     * compare this pointList with other
     * @param other the other pointList to be compared with
     * @return true if both pointLists have the same elements and same size
     */
    bool equals(const tpointList<T>& other) const;

    /**
     * compare this pointList with other
     * @param other the other pointList to be compared with
     * @return true if both pointLists have the same elements and same size
     */
    bool operator==(const tpointList<T>& other) const;

    /**
     * returns true if the list is empty
     */
    bool empty() const;

    /**
     * returns the last set or calculated boundary.
     *
     * The boundary is the smallest rectangle that contains all the
     * points in the list.  Note that the boundary must be set by the
     * user, or the user must explicitly specify that it must be
     * updated (see lti::tpointList<T>::updateBoundary).  It will NOT
     * be updated automatically
     */
    const trectangle<T>& getBoundary() const {return boundary;};

    /**
     * set the boundary of the points.
     *
     * The boundary is the smallest rectangle that contains all the
     * points in the list.  Note that the boundary must be set by the
     * user using this method, or the user must explicitly specify
     * that it must be updated (see lti::tpointList<T>::updateBoundary).
     * It will \b NOT be updated automatically with each point you insert or
     * delete.
     */
    void setBoundary(const trectangle<T>& r) {boundary.copy(r);};

    /**
     * calculate the boundary box.
     *
     * The boundary is the smallest rectangle that contains all the
     * points in the list.  Note that the boundary must be set by the
     * user, or the user must explicitly specify that it must be
     * calculated.
     *
     * This member computes the boundary, but it does not set the compute
     * one into the internal boundary attribute.  See also updateBoundary().
     *
     * @return a the calculated boundary
     */
    const trectangle<T> computeBoundary() const;

     /**
     * calculate and update the boundary box.
     *
     * The boundary is the smallest rectangle that contains all the
     * points in the list.  Note that the boundary must be set by the
     * user, or the user must explicitly specify that it must be
     * calculated.
     *
     * This member computes the boundary AND set the internal boundary
     * attribute.  See also computeBoundary().
     *
     * @return a reference to the calculated boundary
     */
    const trectangle<T>& updateBoundary();

    /**
     * write the point list in the given ioHandler
     */
    virtual bool write(ioHandler& handler,const bool& complete = true) const;

    /**
     * read the point list from the given ioHandler
     */
    virtual bool read(ioHandler& handler,const bool& complete = true);


  protected:
    /**
     * this pointList class is implemented with a list_type<> instance
     */
    list_type thePointList;

    /**
     * boundary is the smallest rectangle which includes all the points
     * in the list
     */
    trectangle<T> boundary;
  };

  /**
   * read the vector from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   *
   * @ingroup gStorable
   */
  template<class T>
  bool read(ioHandler& handler,tpointList<T>& plst,const bool complete=true) {
      return plst.read(handler,complete);
  }

  /**
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   *
   * @ingroup gStorable
   */
  template<class T>
  bool write(ioHandler& handler, const tpointList<T>& plst,
             const bool complete=true) {
      return plst.write(handler,complete);
  }

  /**
   * pointList is a list of points with integer coordinates
   */
  typedef tpointList<int> pointList;

} // namespace lti

namespace std {

  template<class T>
  std::ostream& operator<<(std::ostream& s,const lti::tpointList<T>& pts) {

    typename lti::tpointList<T>::const_iterator it,eit;
    if (pts.empty()) {
      return s;
    }

    it  = pts.begin();
    eit = pts.end();

    s << *it;
    for (++it;it!=eit;++it) {
      s << " " << *it;
    }

    return s;
  }

}

#endif

