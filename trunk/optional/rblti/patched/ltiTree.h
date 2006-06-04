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
 * file .......: ltiTree.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.11.00
 * revisions ..: $Id$
 */

#ifndef _LTI_TREE_H_
#define _LTI_TREE_H_

#include "ltiObject.h"
#include "ltiMathObject.h"
#include "ltiException.h"
#include "ltiMath.h"
#include <list>
#include <vector>

namespace lti {
  /**
   * Tree container class.
   *
   * The lti::tree class allows the representation of rooted ordered trees.
   *
   * The tree class is a container class implemented as template.
   * The template type T is the type of a data element contained in each
   * node of the tree, independently if it is a leaf or not.  The only
   * requirement for the type T is that is must not depend on dynamic
   * allocation of internal data.
   *
   * All types defined in ltiTypes.h use static members and can be
   * contained by the lti::tree class.
   *
   * The tree is always created empty.   You can however specify in the
   * constructor a default degree, i.e. the maximal number of childs
   * to be expected per node.
   * 
   * The first element will be inserted with pushRoot().
   *
   * @ingroup gAggregate
   */
  template<class T>
  class tree : public mathObject {

  public:
    /**
     * Type of the data in the nodes of the tree
     */
    typedef T value_type;

    /**
     * Return type of the size() member
     */
    typedef int size_type;

    class node;
    class iterator;
    class const_iterator;

  protected:
    /**
     * The nodeManager takes care of the memory administration.  It is
     * required to ensure that the iterators do not access invalid memory
     */
    class nodeManager {
    public:

      /**
       * Constructor
       * @param n the degree of each new node created
       */
      nodeManager(const int& n) : theSize(0),theDegree(n) {};

      /**
       * Creates a new node and returns the index of that node in theNodes
       * @param n initial number of children (degree) for this node
       * @param theNewParent the parent node of this node
       * @param newData the data to be hold on this node
       * @return the index of this node in theNodes
       */
      node* createNode(const int& n,
                       node& theNewParent,
                       const T& newData=T()) {
        int idx;
        // check if there are free nodes in the index list
        if (freedNodes.empty()) {
          // no more nodes free!
          // add a new node in theNodes

          idx = static_cast<int>(theNodes.size());
          node* newNode = new node(n,theNewParent,*this,newData,idx);
          theNodes.push_back(newNode);

        } else {
          // take the first free index
          idx = freedNodes.front();
          freedNodes.pop_front();

          node* newNode = new node(n,theNewParent,*this,newData,idx);
          theNodes[idx] = newNode;
        }

        theSize++;

        return theNodes[idx];
      };

      /**
       * Creates a new node and returns the index of that node in theNodes
       * @param n initial number of children (degree) for this node
       * @param theNewParent pointer to the parent node of this node
       * @param newData the data to be hold on this node
       * @return the index of this node in theNodes
       */
      node* createNode(const int& n,
                       node* theNewParent,
                       const T& newData=T()) {
        int idx;
        // check if there are free nodes in the index list
        if (freedNodes.empty()) {
          // no more nodes free!
          // add a new node in theNodes

          idx = static_cast<int>(theNodes.size());
          node* newNode = new node(n,theNewParent,*this,newData,idx);
          theNodes.push_back(newNode);

        } else {
          // take the first free index
          idx = freedNodes.front();
          freedNodes.pop_front();

          node* newNode = new node(n,theNewParent,*this,newData,idx);
          theNodes[idx] = newNode;
        }

        theSize++;

        return theNodes[idx];
      };

      /**
       * Creates a new node with the standard degree (specified in
       * construction of the node manager) and returns the index of that
       * node in theNodes
       * @param theNewParent the parent node of this node
       * @param newData the data to be hold on this node
       * @return the index of this node in theNodes
       */
      node* createNode(node& theNewParent,
                       const T& newData=T()) {
        return createNode(theDegree,theNewParent,newData);
      };


      /**
       * Free the given node and all its sucessors
       */
      void freeNode(node& theNode) {
        if (isNull(&theNode)) {
          return;
        }

        // first, free all children
        int i,ldegree;
        ldegree = theNode.degree();
        for (i=0;i<ldegree;++i) {
          if (theNode.isChildValid(i)) {
            freeNode(theNode.child(i));
          }
        }

        int idx;

        idx = theNode.index();

        delete theNodes[idx];
        theNodes[idx]=0;

        freedNodes.push_back(idx);

        theSize--;
      };

      /**
       * Get pointer to the node at the given index
       */
      node* getNode(const int& idx) {
        if (idx<static_cast<int>(theNodes.size())) {
          return theNodes[idx];
        } else {
          return 0;
        }
      };

      /**
       * Get pointer to the node at the given index
       */
      const node* getNode(const int& idx) const  {
        if (idx<static_cast<int>(theNodes.size())) {
          return theNodes[idx];
        } else {
          return 0;
        }
      };

      /**
       * Get index of the given node
       */
      int getIndexOfNode(const node& aNode) const {
        return aNode.index();
      };

      /**
       * Returns the number of valid nodes until now
       */
      const int& size() const {return theSize;}

    protected:
      /**
       * The container vector of all nodes of the tree
       */
      std::vector<node*> theNodes;

      /**
       * Freed nodes.
       *
       * This list contains the indices of freed nodes, which can be used
       * by the next createNode
       */
      std::list<int> freedNodes;

      /**
       * Number of valid nodes
       */
      int theSize;

      /**
       * Tree degree
       */
      int theDegree;
    };

  public:

    /**
     * The elements of the tree are instances of the "node" class
     */
    class node : public ioObject {
      /*
       * the node manager requires access to some protected member of
       * the node class, which should not be seen by the user
       */
      friend class nodeManager;
      friend class tree<T>;

    protected:
      /**
       * The children of this node as pointers to nodes
       */
      std::vector<node*> children;

      /**
       * A pointer to the parent node or 0 if this is a root node
       */
      node* theParent;

      /**
       * A pointer to the owner nodeManager
       */
      nodeManager* theOwner;

      /**
       * The data hold by this node
       */
      T theData;

      /**
       * The index of this node on the tree nodes-list
       */
      int myIndex;

      /**
       * Return a const reference to the index of this node in the nodeManager
       */
      inline const int& index() const {return myIndex;};

      /**
       * The level of this node, defined as the lenght of the path
       * from this node to the root.  i.e. if this node is root, its level is 0
       */
      int theLevel;


    public:
      // due to a bug in MSVC++ the class implementation must be here!

      /**
       * Copy constructor
       */
      node(const node& other) {
        copy(other);
      }

      /**
       * Destructor
       */
      virtual ~node() {
      }

      /**
       * Return true if the n-th child of this node has valid data or
       * false otherwise
       */
      bool isChildValid(const int& n) const {
        return ( n<degree() && children[n]!=0 );
      }

      /**
       * Access the n-th child of the node.
       *
       * If n is greater or equal than the degree, an assertion will be thrown
       * If the n-th child has not been initialized yet, an invalid reference
       * will be returned!.  To check if a child is valid use isChildValid().
       */
      const node& child(const int& n) const {
        assert(n<degree());
        return *children[n];
      }

      /**
       * Access a child.
       *
       * If n is greater or equal than the degree, an assertion will be thrown
       * If the n-th child has not been initialized yet, an invalid reference
       * will be returned!.  To check if a child is valid use isChildValid().
       */
      node& child(const int& n) {
        assert(n<degree());

        return *children[n];
      }

      /**
       * Change the degree of this node.
       *
       * @param n new degree
       * @param clear (default false) if true, this node will be
       *              (re)initialized as a leaf (i.e. all children NULL!)
       */
      void setDegree(const int& n,bool clear = false) {
        int i;
        int ldegree = degree();

        if (clear) {
          for (i=0;i<ldegree;++i) {
            // this will call recursively the destruction of the whole branch!
            theOwner->freeNode(*children[i]);
            children[i]=0;
          }
          children.resize(0,0);
        } else {
          if (n<ldegree) {
            for (i=n;i<ldegree;++i) {
              // this will call recursively the destruction of the
              // whole branch!
              theOwner->freeNode(*children[i]);
              children[i]=0;
            }
          }
        }

        children.resize(n,0); // initialize new elements with invalid
      };

      /**
       * Append a new child node with the given data.
       *
       * The new node will be inserted "at the end" of the node as a new child,
       * increasing the actual node degree.
       * The degree of the new child will be the specified degree for the tree
       * @return a reference to the new child
       */
      node& appendChild(const T& newChildData) {
//         int ldegree;

//         ldegree = degree();

        children.push_back(theOwner->createNode(*this,newChildData));

        return *children[children.size()-1];
      };


      /**
       * Insert a new child node with the given data.
       *
       * The new node will be inserted at the first uninitialized child.  If
       * all children are already initialized, the degree of this node will
       * be incremented and the new child will contain the given data.
       * The degree of the new child will be the specified degree for the tree
       * @return a reference to the new child
       */
      node& insertChild(const T& newChildData) {
        int i,ldegree;

        ldegree = degree();
        i=0;
        // search for the first not initialized child...
        while ((i<ldegree) && (children[i]!=0)) {
          ++i;
        }

        if (i>=ldegree) {
          // all children already initialized->create a new one
          setDegree(ldegree+1,false);
          children[ldegree] =
            theOwner->createNode(*this,newChildData);
          i = ldegree;
        } else {
          // i is the index to an uninizalized node
          children[i] = theOwner->createNode(*this,newChildData);
        }

        return *children[i];
      };

      /**
       * Insert a new child node with the given data at the given child index.
       *
       * If the node has already a child at the given position, the whole
       * branch will be inserted at the first child of the new node
       *
       * The degree of the new child will be the degree of this node
       */
      node& insertChild(const int& n,const T& newChildData) {
        assert(n < degree());

        node* newNode = theOwner->createNode(degree(),*this,newChildData);

        if (notNull(children[n])) {
          if (newNode->degree()<1) {
            newNode->setDegree(1);
          }
          newNode->setChild(0,*children[n]);
        }

        children[n]=newNode;

        return *children[n];
      }

      /**
       * Move a child branch to another position at this node.
       *
       * If the node has already a child at the given position, the whole
       * branch will be deleted.
       * @param m the old position
       * @param n the new position
       */
      node& moveChild(const int& m, const int& n) {
        assert(n < degree());
	
        if (notNull(children[n])) {
          theOwner->freeNode(*children[n]);
        }

        children[n] = children[m];
	children[m] = 0;
	
        return *children[n];
      }

      /**
       * Insert a new sibling node with the given data.
       *
       * The new node will be created at the first uninitialized sibling.  If
       * all siblings are already initialized, the degree of the parent will
       * be incremented and the new child will contain the given data.
       * The degree of the new child will be the new degree of the parent
       * An assertion will be thrown if this member is called for a root node
       * @return a reference to the new sibling
       */
      node& insertSibling(const T& newSiblingData) {
        assert(!isRoot());

        return parent().insertChild(newSiblingData);
      }

      /**
       * Delete the given child.
       *
       * @return a reference to this node
       */
      node& deleteChild(const int& n) {
        assert(n<degree());
        if (notNull(children[n])) {
          theOwner->freeNode(*children[n]);
          children[n]=0;
        }
        return *this;
      }

      /**
       * Check if this node is a root node (i.e. has no parent!)
       */
      bool isRoot() const {
        return (isNull(theParent));
      }

      /**
       * Access parent node.
       *
       * Please note that the returned reference can be invalid if this is the
       * root node.  To check this condition use isRoot()
       */
      node& parent() {
        return *theParent;
      };

      /**
       * Access parent node
       */
      const node& parent() const {
        return *theParent;
      };

      /**
       * Return the number of possible children of the node.
       * @see numberOfChildren()
       */
      int degree() const {
        return static_cast<int>(children.size());
      };

      /**
       * Calculate the number of valid children.
       *
       * This number is always less or equal degree().  The number of
       * valid children will be calculated and for that reason is
       * faster to ask for the degree() of the node.
       */
      int numberOfChildren() const {
        int i,ldegree,noc;
        ldegree = degree();
        noc = 0;
        for (i=0;i<ldegree;++i) {
          if (isChildValid(i)) {
            noc++;
          }
        }
        return noc;
      }

      /**
       * Return a reference to the node's data
       */
      T& getData() {
        return theData;
      }

      /**
       * Return a read-only reference to the node's data
       */
      const T& getData() const {
        return theData;
      }

      /**
       * Set the data of the node
       */
      void setData(const T& newData) {
        theData = newData;
      }

      /**
       * Copy the contents of the other node and of all its children.
       *
       * The parent will be kept. All the children will be also copied.
       * @param other the "root" node of the branch to be copied
       * @return a reference to this node
       * If you need to copy only the data of the node, you can use
       * for example <code>setData(other.getData());</code>
       */
      node& copy(const node& other) {

        setData(other.getData());

        setDegree(other.degree(),true); // clean all old children!

        // copy all the children
        int i,ldegree;

        ldegree = degree();

        for (i=0;i<ldegree;++i) {
          if (other.isChildValid(i)) {
            children[i] = theOwner->createNode(other.child(i).degree(),
                                              *this,
                                              T());
            children[i]->copy(other.child(i));
          }
        }

        theLevel = other.theLevel;

        return *this;
      }

      /**
       * Calculate the height of this node, defined as the length of
       * the longest path from this node to some leaf node.  If this
       * node is a leaf, the returned value is 0.
       * Please note that this function is highly recursive and could take
       * a while...
       */
      int height() const {
        int i,ldegree,result;
        ldegree = degree();
        result = 0;
        for (i=0;i<ldegree;++i) {
          if (isChildValid(i)) {
            result = max(result,1+child(i).height());
          }
        }

        return result;
      };

      /**
       * Calculates the size (total number of valid nodes) of the
       * subtree beginning with this node.  If this node is a leaf,
       * the returned value is 1 Please note that this function is
       * highly recursive and could take a while...
       */
      int subtreeSize() const {
        int i,ldegree,result;
        ldegree = degree();
        result = 1;
        for (i=0;i<ldegree;++i) {
          if (isChildValid(i)) {
            result += child(i).subtreeSize();
          }
        }

        return result;
      };

      /**
       * Returns the level of this node, defined as the lenght of the path
       * from this node to the root.  i.e. if this node is root, its level is 0
       */
      const int& level() const {
        return theLevel;
      };

      /**
       * Calulates the position of this node for the parent.  If this
       * node is root, the returned value will be negative. 
       *
       * For example, let aNode be a non-root node, the following
       * expression will return always true:
       * aNode.isSameNode(aNode.parent().child(aNode.position()))
       */
      int position() const {
        if (isRoot()) {
          return -1;
        }

        int i,ldegree,idx;

        ldegree = parent().degree();
        idx = -1;
        for (i=0;(idx<0) && (i<ldegree);++i) {
          if (isSameNode(parent().child(i))) {
            idx = i;
          }
        }

        return idx;
      }

      /**
       * Return true if both nodes contains equal data AND if all its
       * children are equal.  If you need to compare only the data of this
       * node, you can do it explicitly with getData()==other.getData().
       */
      bool equals(const node& other) {
        bool result ;

        result = (getData() == other.getData) &&
          (degree() == other.degree()) &&
          (size() == other.size()) &&
          (height() == other.height());

        int i,ldegree;
        ldegree = degree();
        i=0;
        while ((i<ldegree) && (result)) {
          if (isChildValid(i) && other.isChildValid(i)) {
            result = result && (child(i).equals(other.child(i)));
          } else if (isChildValid(i) || other.isChildValid(i)) {
            result = false;
          }
          i++;
        }
        return result;
      }

      /**
       * Return true if both nodes are exactly the same node
       */
      inline bool isSameNode(const node& other) const {
        return (this == &other);
      }

      /**
       * Return true if the this node is the parent of the other node
       */
      inline bool isParentOf(const node& other) const {
        if (!other.isRoot()) {
          return (isSameNode(other.parent()));
        }
        return false;
      }

      /**
       * Return true if the other node is a sibling of this node.
       * A node is NOT a sibling of itself.
       */
      inline bool isSiblingOf(const node& other) const {
        if (!isRoot() && !other.isRoot() && !isSameNode(other)) {
          return (parent().isSameNode(other.parent()));
        }
        return false;
      }

      /**
       * Equals topology returns true if this and the other tree
       * have exactly the same topology (the same valid children and
       * all children have the same topology
       */
      bool equalsTopology(const node& other) {
        bool result;

        result = (degree() == other.degree());

        int i,ldegree;
        ldegree = degree();
        i=0;

        while ((i<ldegree) && (result)) {
          if (isChildValid(i) && other.isChildValid(i)) {
            result = result && (child(i).equalsTopology(other.child(i)));
          } else if (isChildValid(i) || other.isChildValid(i)) {
            result = false;
          }
          i++;
        }

        return result;
      }

      // -----------------------------
      /**
       * @name Storable interface
       */
      //@{

#     ifndef _LTI_MSC_6
      /**
       * Write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool complete=true) const
#     else
      /**
       * This function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const
#     endif
      {
        bool b = true;
        if (complete) {
          b = handler.writeBegin();
        }

        // write the data contained in this node first
        b = b && lti::write(handler,theData);
        b = b && handler.writeDataSeparator();
        // now write the number of children of this node
        const int ldegree = degree();
        b = b && handler.write(degree());
        b = b && handler.writeDataSeparator();

        // now write the children recursivelly
        b = b && handler.writeBegin();
        
        int i;
        for (i=0;b && (i<ldegree);++i) {
          if (isChildValid(i)) {
            child(i).write(handler);
          } else {
            // empty node
            handler.writeBegin();
            handler.writeEnd();
          }
        }        

        b = b && handler.writeEnd(); // end of children.
        b = b && handler.writeEOL();

        if (complete) {
          b = b && handler.writeEnd();
        }

        return b;
      }

#     ifdef _LTI_MSC_6
      /**
       * Write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool write(ioHandler& handler,
                 const bool complete=true) const {
        // ...we need this workaround to cope with another really
        // awful MSVC bug.
        return writeMS(handler,complete);
      }
#     endif


#     ifndef _LTI_MSC_6
      /**
       * Read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool complete=true)
#     else
      /**
       * This function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is also public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead
       */
      bool readMS(ioHandler& handler,const bool complete=true)
#     endif
      {
        bool b = true;
        if (complete) {
          b = handler.readBegin();
        }

        // read the data of the node
        b = b && lti::read(handler,theData);
        b = b && handler.readDataSeparator();

        // now read and set the number of children of this node
        int ldegree;
        b = b && handler.read(ldegree);
        setDegree(ldegree,true); // clear all subtrees!
        
        b = b && handler.readDataSeparator();

        // now read the rest of the nodes recursivelly
        b = b && handler.readBegin();

        int i;
        for (i=0;b && (i<ldegree);++i) {
          b = b && handler.readBegin();
          if (!handler.tryEnd()) {
            // a node was found, let's read it recursivelly
            insertChild(i,T());
            b = b && child(i).read(handler,false);
            b = b && handler.readEnd();
          }
        }
        
        b = b && handler.readEnd();
        
        if (complete) {
          b = b && handler.readEnd();
        }
        
        return b;
      }

#     ifdef _LTI_MSC_6
      /**
       * Read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      bool read(ioHandler& handler,const bool complete=true) {
        // ...we need this workaround to cope with another really awful MSVC
        // bug.
        return readMS(handler,complete);
      }
#     endif
      //@}


    protected:

      /**
       * Protected constructor: only the nodeManager is allowed to generate new
       * nodes!
       *
       * @param n initial number of children (degree) for this node
       * @param theNewParent the parent node of this node
       * @param owner the tree which owns this node
       * @param newData the data to be hold on this node
       * @param index of this node on the tree
       */
      node(const int& n,
           node& theNewParent,
           nodeManager& owner,
           const T& newData,
           const int& index)
        : theParent(&theNewParent),theOwner(&owner),
          theData(newData),myIndex(index),theLevel(0) {
        children.resize(n,0);
        if (notNull(theParent)) {
          theLevel = theParent->level()+1;
        }
      };

      /**
       * Protected constructor: only the nodeManager is allowed to generate new
       * nodes!
       *
       * @param n initial number of children (degree) for this node
       * @param theNewParent the parent node of this node
       * @param owner the tree which owns this node
       * @param newData the data to be hold on this node
       * @param index of this node on the tree
       */
      node(const int& n,
           node* theNewParent,
           nodeManager& owner,
           const T& newData,
           const int& index)
        : theParent(theNewParent),theOwner(&owner),
          theData(newData),myIndex(index),theLevel(0) {
        children.resize(n,0);
        if (notNull(theParent)) {
          theLevel = theParent->level()+1;
        }
      };

      /**
       * Change the reference to the parent node.  Note that you
       * should make sure that the old parent forgets about this node!  */
      void setParent(node& newParent) {
        theParent = &newParent;
        updateLevels();
      }

      /**
       * Change the reference of a child node.
       * Be careful with the use of this
       * member function: after the use of the function there is no
       * way to get a reference to the old child, and this can produce
       * huge memory leaks!!!
       * the parent of the child will be changed to this node!
       */
      void setChild(const int& n,node& newChild) {
        assert(n<degree());
        children[n] = &newChild;
        newChild.theParent = this;
        updateLevels();
      }

      /**
       * Update parent size and height
       */
      void updateLevels() {
        if (notNull(theParent)) {
          theLevel = theParent->level()+1;
        }

        const int& ldegree = degree();
        const int tmpLevel = theLevel+1;
        int i;

        for (i=0;i<ldegree;++i) {
          if (isChildValid(i) && (child(i).level() != tmpLevel)) {
            child(i).updateLevels();
          }
        }
      }

    };

    /**
     * Iterator type (allows read and write operations)
     *
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::tree::begin() for
     * an example
     *
     * CAUTION: Try to use the prefix incremental operator
     * (i.e. ++it) instead of the postfix operator (i.e. it++) to
     * allow efficient code!
     *
     * CAUTION: Do not delete sibling nodes of already iterated nodes:
     * the iterator keeps track of them, and if you delete them
     * invalid references to unexistent nodes could be returned by
     * late iterations.  It IS save to insert or delete children of
     * the nodes pointed by the iterator (but this could invalidate
     * other iterators on the same tree).  If you really really need
     * to do things like that, you should use your own node pointers
     * and iteration mechanisms :-(
     *
     * The iterators will traverse the tree in a prefix form (first the node
     * and then the children)
     */
    class iterator {

#     ifdef _LTI_MSC_6
      friend class const_iterator;
#     else
      friend class tree<T>::const_iterator;
#     endif

      friend class tree<T>;
    protected:
      /**
       * Stack with all still-to-be-iterated-children
       */
      std::list<int> stack;

      /**
       * The pointed node
       */
      node* pointedNode;

      /**
       * The owner tree
       */
      nodeManager* theOwner;

      /**
       * Create an iterator initialized with the given pointer to a node
       */
      explicit iterator(node& theNode,nodeManager& owner)
        : pointedNode(&theNode),theOwner(&owner) {};

    public:
      /**
       * Default constructor
       */
      iterator() : pointedNode(0),theOwner(0) {};


      /**
       * Copy constructor
       */
      iterator(const iterator& other)
        : stack(other.stack),pointedNode(other.pointedNode),
          theOwner(other.theOwner) {};

      /**
       * Advance to next item (prefix increment operator)
       */
      inline iterator& operator++() {
        assert(notNull(pointedNode)); // the end() cannot be incremented!

        int i;

        const int size = pointedNode->degree() - 1;

        for (i=size;i>=0;i--) {
          if (pointedNode->isChildValid(i)) {
            stack.push_front(theOwner->getIndexOfNode(pointedNode->child(i)));
          }
        }

        if (stack.empty()) {
          pointedNode = 0;
        } else {
          do {
            const int idx = stack.front();
            stack.pop_front();

            pointedNode = theOwner->getNode(idx);
          } while (isNull(pointedNode) && (!stack.empty()));

        }

        return *this;
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator==(const iterator& other) const {
        return (pointedNode == other.pointedNode);
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator!=(const iterator& other) const {
        return (pointedNode != other.pointedNode);
      };

      /**
       * Get pointed data
       */
      inline node& operator*() {return *pointedNode;};

      /**
       * Copy member
       */
      inline iterator& operator=(const iterator& other) {
        pointedNode = other.pointedNode;
        stack = other.stack;
        theOwner = other.theOwner;
        return *this;
      };
    };

    /**
     * Const iterator type (allows read-only operations)
     *
     * The use of the iterator classes is similar to the iterators of
     * the STL (Standard Template Library). See lti::tree::begin() for
     * an example.
     *
     * CAUTION: Do not delete sibling nodes of already iterated nodes:
     * the iterator keeps track of them, and if you delete them
     * invalid references to unexistent nodes could be returned by
     * late iterations.  It IS save to insert or delete children of
     * the nodes pointed by the iterator (but this could invalidate
     * other iterators on the same tree).  If you really really need
     * to do things like that, you should use your own node pointers
     * and iteration mechanisms :-(
     */
    class const_iterator {
      friend class tree<T>;
    protected:
      /**
       * Stack with all still-to-be-iterated children
       */
      std::list<int> stack;

      /**
       * The pointed node
       */
      const node* pointedNode;

      /**
       * The owner tree
       */
      const nodeManager* theOwner;

      /**
       * Create an iterator initialized with the given pointer to a node
       */
      explicit const_iterator(const node& theNode,const nodeManager& owner)
        : pointedNode(&theNode),theOwner(&owner) {};

    public:
      /**
       * Default constructor
       */
      const_iterator() : pointedNode(0),theOwner(0) {};

      /**
       * Copy constructor
       */
      const_iterator(const const_iterator& other)
        : stack(other.stack),pointedNode(other.pointedNode),
          theOwner(other.theOwner) {};

      /**
       * Copy constructor
       */
      const_iterator(const iterator& other)
        : stack(other.stack),pointedNode(other.pointedNode),
          theOwner(other.theOwner) {
      };

      /**
       * Advance to next item (prefix increment operator)
       */
      inline const_iterator& operator++() {
        assert(notNull(pointedNode)); // the end() cannot be incremented!

        int i;

        const int size = pointedNode->degree() - 1;

        for (i=size;i>=0;i--) {
          if (pointedNode->isChildValid(i)) {
            stack.push_front(theOwner->getIndexOfNode(pointedNode->child(i)));
          }
        }

        if (stack.empty()) {
          pointedNode = 0;
        } else {
          do {
            const int idx = stack.front();
            stack.pop_front();

            pointedNode = theOwner->getNode(idx);
          } while (isNull(pointedNode) && (!stack.empty()));
        }

        return *this;
      };

      /**
       * Advance to next item (this postfix version is slower than the
       * prefix one
       */
      inline const_iterator operator++(int) { // postfix
        assert(notNull(pointedNode)); // the end() cannot be incremented!
        const_iterator tmp(*this);

        int i;

        const int size = pointedNode->degree() - 1;

        for (i=size;i>=0;i--) {
          if (pointedNode->isChildValid(i)) {
            stack.push_front(theOwner->getIndexOfNode(pointedNode->child(i)));
          }
        }

        if (stack.empty()) {
          pointedNode = 0;
        } else {
          do {
            const int idx = stack.front();
            stack.pop_front();

            pointedNode = theOwner->getNode(idx);
          } while (isNull(pointedNode) && (!stack.empty()));
        }

        return tmp;
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator==(const const_iterator& other) const {
        return (pointedNode == other.pointedNode);
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator!=(const const_iterator& other) const {
        return (pointedNode != other.pointedNode);
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator==(const iterator& other) const {
        return (pointedNode == other.pointedNode);
      };

      /**
       * Compare this iterator with the other one
       */
      inline bool operator!=(const iterator& other) const {
        return (pointedNode != other.pointedNode);
      };

      /**
       * Get pointed data
       */
      inline const node& operator*() const {return *pointedNode;};

      /**
       * Copy member
       */
      inline const_iterator& operator=(const const_iterator& other) {
        pointedNode = other.pointedNode;
        stack = other.stack;
        theOwner = other.theOwner;
        return *this;
      };

      /**
       * Copy member
       */
      inline const_iterator& operator=(const iterator& other) {
        pointedNode = other.pointedNode;
        stack = other.stack;
        theOwner = other.theOwner;
        return *this;
      };
    };

    // ---------------------------------------------------------------
    //   TREE CLASS
    // ---------------------------------------------------------------

    /**
     * Default constructor
     *
     * If you do not provide any arguments, an empty tree of zero-degree
     * will be created.
     *
     * You can also provide the degree of the tree, which is the maximal number
     * of children a node can have, but even if you provide the degree, the
     * tree will be empty.  Use pushRoot to insert the first node in the tree.
     *
     * @param n the default number of children (degree) of the root node.
     */
    tree(const int& n=0);

    /**
     * Create this tree as a copy of another tree.
     *
     * @param other the tree to be copied.
     */
    tree(const tree<T>& other);

    /**
     * Destructor
     */
    virtual ~tree();

    /**
     * Returns the name of this class: "tree"
     */
    const char* getTypeName() const {return "tree";};

    /**
     * Returns the total number of nodes of the tree
     */
    inline size_type size() const;

    /**
     * Returns the height of the tree, defined as the length of the longest
     * path from the root to some leaf node
     */
    inline size_type height() const;

    /**
     * Returns const_iterator to a "first" node of the tree.  This node will
     * be allways the root
     * Note that you can not change the values of the tree
     * elements when you use a const_iterator. See also begin()
     */
    inline const_iterator begin() const;

    /**
     * Returns an iterator to the first element.
     *
     * The use of the interators is similar to the iterators of the
     * Standard Template Library (STL).
     * If you need to iterate on all elements of the tree, you can
     * use following code:
     *
     * \code
     * int tmp,accu;                   // a temporal variable
     * lti::tree<int> myTree();        // empty tree
     * // fill your tree here
     * lti::tree<int>::iterator it;    // an iterator
     *
     * for (it=myTree.begin();it!=myTree.end();++it) {
     *   tmp = (*it).getData();        // tmp has value of element pointed
     *                                 // by the iterator.
     *   accu += tmp;
     *   (*it).setData(accu);          // change the value in the tree.
     * }
     * \endcode
     *
     * Please note that if you define <code>it</code> as a const_iterator,
     * you can NOT make something like <code>*it=accu</code>.
     */
    inline iterator begin();

    /**
     * Returns last index as a const iterator.
     * For an example see begin()
     */
    inline const_iterator end() const;

    /**
     * Returns last index as an iterator
     * For an example see begin()
     */
    inline iterator end();

    /**
     * Return an iterator which points to the given node
     */
    inline iterator getIteratorTo(node& aNode);

    /**
     * Return a const_iterator which points to the given node
     */
    inline const_iterator getIteratorTo(const node& aNode) const;

    /**
     * Clears the tree (this tree will be empty!).
     * see fill(const T&) to initialize all tree nodes with some value
     */
    void clear();

    /**
     * Fills all tree elements with <code>iniValue</code>.
     *
     * @param iniValue the elements will be initialized with this
     * value.
     */
    void fill(const T& iniValue);

    /**
     * Return the root node of the tree.  Note that if the tree is empty
     * the reference is invalid.  Use empty() to check this
     */
    node& root();

    /**
     * Access element x of the tree in a read-only modus.  Note that if the
     * tree is empty, the reference is invalid.  Use empty() to check this.
     */
    const node& root() const;

    /**
     * Insert a node at the root position.
     *
     * pushRoot will insert the given data as the root of the tree,
     * and the old root will be inserted as its first child.  The root
     * will have the mininum between 1 and the degree specified in the
     * creation of the tree.
     *
     * If the tree was empty, it will have a new node with the given data.
     */
    void pushRoot(const T& newData);

    /**
     * Assigment operator.
     * copy the contents of <code>other</code> in this %object.
     * @param other the source tree to be copied.
     */
    tree<T>& copy(const tree<T>& other);

    /**
     * Create a clone of this tree
     * @return a pointer to a copy of this tree
     */
    virtual mathObject* clone() const;

    /**
     * Returns true if the tree is empty
     */
    bool empty() const;

    /**
     * Compare this tree with other
     * @param other the other tree to be compared with
     * @return true if both trees have the same elements and topology
     */
    bool equals(const tree<T>& other) const;

    /**
     * Compare this tree with other
     * @param other the other tree to be compared with
     * @return true if both trees have the same elements and topology
     */
    inline bool operator==(const tree<T>& other) const;

    /**
     * Assigment operator (alias for copy(other)).
     * @param other the tree to be copied
     * @return a reference to the actual tree
     */
    tree<T>& operator=(const tree<T>& other) {return copy(other);};

    /**
     * Write the functor in the given ioHandler. The default implementation
     * is to write just the parameters object.
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool write(ioHandler& handler,
                       const bool complete=true) const;

    /**
     * Read the parameters from the given ioHandler. The default implementation
     * is to read just the parameters object.
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool read(ioHandler& handler,const bool complete=true);


  protected:
    /**
     * The root of the tree
     */
    node* theRoot;

    /**
     * Degree for the root specified by the user in constructor
     */
    int rootDegree;

    /**
     * The node manager administrates the memory allocation and deallocation
     * for all nodes in the tree
     */
    nodeManager theNodeManager;

  };
} // namespace lti

#include "ltiTree_inline.h"
#include "ltiTree_template.h"


#endif

