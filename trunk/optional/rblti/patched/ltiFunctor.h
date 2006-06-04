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
 * file .......: ltiFunctor.h
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 14.04.99
 * revisions ..: $Id$
 */

#ifndef _LTI_FUNCTOR_H_
#define _LTI_FUNCTOR_H_

#include "ltiIoObject.h"
#include "ltiStatus.h"
#include "ltiException.h"
#include "ltiIoHandler.h"

namespace lti  {
  /**
   * Base class for all lti functors.
   *
   * Every lti fuctor must have at least the member <em>apply()</em>,
   * which "applies" the functor's functionality on the data given through the
   * arguments of the apply method.
   *
   * There are two kinds of apply methods:
   *  - the "on-copy"-apply returns the result in a new
   *    object, and the original data will not be modified.
   *  - the "on-place"-apply returns the result on the same input object and
   *    therefore the original data will be destroyed.
   *
   * The operation of the functor can be controled with some parameters,
   * which will can be set with the "setParameters" member-function.
   *
   * Each functor may have also other setABC() members, to allow the
   * change of just one parameter-item at the time.
   */
  class functor : public ioObject, public status {
  public:
    /**
     * Base class for all lti parameter objects
     */
    class parameters : public ioObject {
    public:
      /**
       * default constructor
       */
      parameters();

      /**
       * copy constructor
       */
      parameters(const parameters& other);

      /**
       * destructor
       */
      virtual ~parameters();

      /**
       * copy data of "other" parameters
       */
      parameters& copy(const parameters& other);

      /**
       * returns a pointer to a clone of the parameters.
       */
      virtual parameters* clone() const = 0;

      /**
       * return the name of this type
       */
      const char* getTypeName() const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,
                         const bool complete=true) const;

      /**
       * read the parameters from the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool read(ioHandler& handler,const bool complete=true);

#     ifdef _LTI_MSC_6
      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug!, so please
       * NEVER EVER call this method directly
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug!, so please
       * NEVER EVER call this method directly
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif
      
    protected:
      /**
       * copy data of "other" parameters
       */
      parameters& operator=(const parameters& other);
    };

    /**
     * Exception thrown when the parameters are not set
     */
    class invalidParametersException : public exception {
    public:
      /**
       * Default constructor
       */
      invalidParametersException()
        : exception("wrong parameter type or parameters not set yet") {};

      /**
       * Constructor with the alternative object name, where the exception
       * was thrown
       */
      invalidParametersException(const std::string& str)
        : exception(std::string("wrong parameter type or parameters not set " \
                                "yet at ")+str) {};

      /**
       * Constructor with the alternative object name, where the exception
       * was thrown
       */
      invalidParametersException(const char* str)
        : exception(std::string("wrong parameter type or parameters not set " \
                                "yet at ")+std::string(str)) {};


      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

    /**
     * Exception thrown when a method of a functor is not implemented for
     * a specific parameter set.
     */
    class invalidMethodException : public exception {
    public:
      /**
       * Default constructor
       */
      invalidMethodException()
        : exception("Method not implemented for given parameters") {};

      /**
       * Constructor with alternative message
       */
      invalidMethodException(const std::string& str)
        : exception(str) {};

      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

#ifndef SWIG
typedef lti::functor::parameters functor_parameters;
#endif

    /**
     * default constructor
     */
    functor();

    /**
     * copy constructor
     */
    functor(const functor& other);

    /**
     * destructor
     */
    virtual ~functor();

    /**
     * Set functor's parameters.
     *
     * This member makes a copy of \a theParam: the functor
     * will keep its own copy of the parameters!
     *
     * Additionally, the updateParameters will be called, which in some
     * functors initializes some Look-Up Tables, or filters, etc. in order to
     * improve the execution times later in the apply methods.
     * 
     * Since the LTI-Lib follows a deep-copy philosophy, if you copy the
     * functor, the copy will create an exact copy of the parameters, no
     * matter if they have been specified with useParameters() or with
     * setParameters().
     *
     * You should always prefer this method over useParameters().  The latter
     * one is used only in very special cases, when the user needs to control
     * an instance of parameters outside the functor and he/she wants to take
     * care of the memory management.
     *
     * This method should never be overloaded.  If you want to make some
     * precomputations when setting the parameters, please overload
     * updateParameters().
     *
     * @see useParameters(),updateParameters()
     *
     * @return true if successful, false otherwise
     */
    virtual bool setParameters(const parameters& theParam);

    /**
     * Use the given parameters exactly as they are.  
     *
     * The difference of this method with set parameters is that the parameters
     * will not be copied.  They will be used exactly as they are given.  It
     * is therefore your responsability to ensure that the parameters instance
     * you give exists as long as the functor is being used.
     *
     * Of course, if the parameters content is changed while the functor makes
     * its computations, unpredictable behaviour has to be expected.  It is
     * also your responsibility to take care of all multi-threading details.
     *
     * Since the LTI-Lib follows a deep-copy philosophy, if you copy the
     * functor, the copy will create an exact copy of the parameters, no
     * matter if they have been specified with useParameters() or with
     * setParameters().
     *
     * This member will not administrate the given instance.  If you create it
     * with new, you have to take care about the deletion.
     *
     * If you are not sure what to use, always prefer setParameters() since it
     * is more secure than useParameters(), as the functor will take care of
     * the memory management.
     *
     * This method always calls updateParameters().
     *
     * \warning Use this method only if you really know what you are doing.
     *          If you are not sure, use setParameters() instead.
     *
     * Note that the given reference can be rewritten by the functor, specially
     * if some shortcuts are used or the read method is called, then their
     * content will be changed (another reason to prefer setParameters()).
     *
     * @return true if successful, false otherwise
     */
    virtual bool useParameters(parameters& theParam);

    /**
     * Update parameters.
     *
     * Some functors make some pre-computations when the parameters are set.
     * For example, they initialize look-up tables, build some filter kernels,
     * etc.  This job has to be done by update parameters.
     *
     * In principle, you only need to overload this method if you need such
     * initializations, as setParameters() and useParameters() will always
     * call this method.
     *
     * For disobedient people, who ignore the recomendation of using
     * setParameters() over useParameters(), this method should be called 
     * if attributes of the parameters instance are changed, before calling
     * the apply() method.
     *
     * \warning If you call an apply() method of a functor in which you
     * changed the attributes of the parameters instance without updating
     * them, you have to expect an unpredictible behaviour: segmentation
     * fault, system crash, blue screens, etc. can occur as the sizes of
     * internal structures may not be consistent with the given parameters.
     * This problem does not exist if you indicate the parameters to be used
     * with setParameters(), as the functor will have an instance of its own.
     */
    virtual bool updateParameters();

    /**
     * returns current parameters.
     */
    const parameters& getParameters() const;

    /**
     * returns true if the parameters are valid
     */
    virtual bool validParameters() const;

    /**
     * copy data of "other" functor.
     * Please note that the status string will _NOT_ be copied!
     */
    functor& copy(const functor& other);

    /**
     * clone member
     */
    virtual functor* clone() const = 0;

    /**
     * returns the name of this type
     */
    const char* getTypeName() const;

//     /**
//      * return the last message set with setStatusString().  This will
//      * never return 0.  If no status-string has been set yet an empty string
//      * (pointer to a string with only the char(0)) will be returned.
//      */
//     virtual const char* getStatusString() const;

//     /**
//      * set a status string.
//      *
//      * @param msg the const string to be reported next time by
//      * getStatusString(). The given string will be copied.
//      * This message will be usually set within the apply methods to indicate
//      * an error cause.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void setStatusString(const char* msg) const;

//     /**
//      * append a message to the current status string. Take care to
//      * reset the status string by calling setStatusString() for each
//      * call of an apply() or similar method. appendStatusString()
//      * should only be used after setStatusString() has been called.
//      *
//      * @param msg message to be appended to the current status string.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void appendStatusString(const char* msg) const;

//     /**
//      * append an integer value to the current status string. Take care
//      * to reset the status string by calling setStatusString() for
//      * each call of an apply() or similar method. appendStatusString()
//      * should only be used after setStatusString() has been called.
//      *
//      * @param msg integer value to be appended to the current status
//      * string.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void appendStatusString(const int& msg) const;

//     /**
//      * append a double value to the current status string. Take care
//      * to reset the status string by calling setStatusString() for
//      * each call of an apply() or similar method. appendStatusString()
//      * should only be used after setStatusString() has been called.
//      *
//      * @param msg double value to be appended to the current status
//      * string.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void appendStatusString(const double&  msg) const;

//     /**
//      * Append the status string of another %functor to this functors
//      * status string. To achieve better readability of the resulting
//      * message a new line is started with the other functor's name and
//      * the message.
//      *
//      * @param other %functor whose status string is to be append to this
//      *        status string.
//      *
//      * Note that the change of the status string is not considered as
//      * a change in the functor status.
//      */
//     virtual void appendStatusString(const functor& other) const;

    /**
     * write the functor in the given ioHandler. The default implementation
     * is to write just the parameters object.
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool write(ioHandler& handler,
                       const bool complete=true) const;

    /**
     * Read the functor from the given ioHandler. 
     *
     * The default implementation is to read just the parameters object.
     *
     * Since this virtual method needs to know the exact type of the
     * parameters to call the proper read method, it will just assume that the
     * current functor instance has a valid parameter set.  If this is not
     * the case, you need to reimplement the read method to set first a dummy
     * parameter object.
     *
     * @param handler the ioHandler to be used
     * @param complete if true (the default) the enclosing begin/end will
     *        be also written, otherwise only the data block will be written.
     * @return true if write was successful
     */
    virtual bool read(ioHandler& handler,const bool complete=true);

  protected:
    /**
     * returns current parameters. (non const! -> protected)
     */
    parameters& getParameters() {return *params;};

  private:

    /**
     * This is private to avoid default implementation.
     * 
     * alias for copy (MUST be reimplemented by each class or an
     * exception will be thrown!)
     */
    functor& operator=(const functor& other);

    /**
     * current parameters.
     */
    parameters* params;

    /**
     * Flag that indicates if the functor owns the parameters or not
     */
    bool ownParams;

//     /**
//      * the status string written with setStatusString
//      */
//     mutable char* statusString;

//     /**
//      * the empty string returned if the statusString is empty
//      */
//     static const char *const emptyString;
  };

  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const functor::parameters& p,
             const bool complete = true);

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,functor::parameters& p,
             const bool complete = true);

  /**
   * write the functor in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not.
   * The default implementation is to write only the parameters object,
   * since most functors do not have a state.
   */
  bool write(ioHandler& handler,const functor& p,
             const bool complete = true);

  /**
   * read the functor from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not.
   * The default implementation is to write only the parameters object,
   * since most functors do not have a state.
   */
  bool read(ioHandler& handler,functor& p,
             const bool complete = true);

} // namespace lti

#endif

