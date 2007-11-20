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
 * file .......: ltiTree.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 24.11.00
 * revisions ..: $Id$
 */

#include <iostream>

namespace lti {
  // implementation of template functions

  // -------------------------------------------------------------------
  // tree
  // -------------------------------------------------------------------
  // default constructor
  template<class T>
  tree<T>::tree(const int& n)
    : mathObject(),theRoot(0),rootDegree(n),theNodeManager(n) {
  }

  // copy constructor
  template<class T>
  tree<T>::tree(const tree<T>& other)
    : mathObject(),theRoot(0),rootDegree(0),theNodeManager(other.rootDegree) {
    copy(other);
  }

  template<class T>
  tree<T>::~tree() {
    clear();
  }

  template<class T>
    void tree<T>::clear() {
    if (!empty()) {
      theNodeManager.freeNode(*theRoot);
    }

    theRoot = 0;
  }

  template<class T>
  void tree<T>::fill(const T& data) {
    iterator it,eit;

    for (it=begin(),eit=end();it!=eit;++it) {
      (*it).setData(data);
    }
  }

  template<class T>
  void tree<T>::pushRoot(const T& newData) {
    if (empty()) {
      // the tree is empty! -> generate a new root node
      theRoot = theNodeManager.createNode(rootDegree,
                                          static_cast<node*>(0),
                                          newData);
    } else {
      int ldegree;
      ldegree = max(1,rootDegree);
      node* tmp = theNodeManager.createNode(ldegree,
                                            static_cast<node*>(0),
                                            newData);
      theRoot->setParent(*tmp);
      tmp->setChild(0,*theRoot);
      theRoot = tmp;
    }
  }

  // copy member
  template<class T>
  tree<T>& tree<T>::copy(const tree& other) {
    clear();

    if (!other.empty()) {
      theRoot = theNodeManager.createNode(other.root().degree(),
                                          static_cast<node*>(0),T());
      theRoot->copy(other.root());
    }

    rootDegree = other.rootDegree;
    return *this;
  }

  // creates a clone of this tree
  template<class T>
  mathObject* tree<T>::clone() const {
    tree<T>* tmp = new tree<T>(*this);
    return tmp;
  }

  /* compare this tree with other
     @param other the other tree to be compared with
     @return true if both trees have the same elements and same size
  */
  template<class T>
  bool tree<T>::equals(const tree<T>& other) const {

    bool result = (rootDegree == other.rootDegree) &&
      ((empty() && other.empty()) || (!empty() && !other.empty()));

    if (result && !empty()) {
      //result = root().equals(other.root());
      result = false;
      (*theRoot).equals( *(other.theRoot) );
    }

    return result;
  }

  template<class T>
    bool tree<T>::empty() const {
    return isNull(theRoot);
  }

  template<class T>
  const typename tree<T>::node& tree<T>::root() const {
    return *theRoot;
  }

  template<class T>
  typename tree<T>::node& tree<T>::root() {
    return *theRoot;
  }

  // storable interface
  /*
   * write the functor in the given ioHandler. The default implementation
   * is to write just the parameters object.
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  template<class T>
  bool tree<T>::write(ioHandler& handler,
                      const bool complete) const {
    bool b = true;
    if (!empty()) {
      root().write(handler,complete);
    } else if (!complete) {
      // in case the tree is empty, ensure that a begin-end token pair is
      // present
      b = handler.writeBegin();
      b = handler.writeEnd();
    }   
    
    return b;
  }
  
  /*
   * read the parameters from the given ioHandler. The default implementation
   * is to read just the parameters object.
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  template<class T>
  bool tree<T>::read(ioHandler& handler,const bool complete) {
    bool b = true;
    clear();

    if (complete) {
      // complete it's easy
      b = b && handler.readBegin();
      if (handler.tryEnd()) {
        // empty tree
        return true;
      }

      // insert a dummy root
      pushRoot(T());
      // read the tree
      root().read(handler,false);

      b = b && handler.readEnd();
    } else {
      // not complete is that's more tricky
      if (handler.tryBegin()) {
        // we are here only if the node is empty
        b = b && handler.readEnd();
      } else {
        // insert a dummy root
        pushRoot(T());
        // read the tree
        b=root().read(handler,false);
      }
    }

    return b;
  }

} // namespace lti

namespace std {

  /// ASCII output of a tree to a std stream.
  template <class T>
  std::ostream& operator<<(std::ostream& s,
                           const lti::tree<T>& v) {
    if (v.empty()) {
      return s;
    }

    string prefix;
    const typename lti::tree<T>::node *back, *tmp;
    std::list< const typename lti::tree<T>::node* > stack;
    stack.push_back(&v.root());
    int level;
    
    while (!stack.empty()) {
      // take the top
      back = stack.back();
      stack.pop_back();
      prefix="";
      if (isNull(back)) {
        // if pointer is null, it is an empty child
        // get the level also from the stack
        back = stack.back();
        stack.pop_back();

        level = reinterpret_cast<int>(back);
        prefix.append(2*level,' ');
        prefix += "+ NIL";
        s << prefix << endl;        
      } else {
        level = back->level();
        // show this node
        if (level>0) {
          prefix.append(2*level,' ');
        }
        prefix += "+ ";
        s << prefix.c_str() << back->getData() << endl;
        
        int i;
        for (i=back->degree()-1;i>=0;--i) {
          if (back->isChildValid(i)) {
            stack.push_back(&(back->child(i)));
          } else {
            tmp =
              reinterpret_cast<const typename lti::tree<T>::node*>(level+1);
            stack.push_back(tmp);
            stack.push_back(0);
          }
        }
      }
    }

    return s;
  }

} // end namespace std
