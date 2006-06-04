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
 * file .......: ltiViewer.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.11.2001
 * revisions ..: $Id$
 */

#ifndef _LTI_VIEWER_BASE_H_
#define _LTI_VIEWER_BASE_H_

#include "ltiObject.h"
#include "ltiFunctor.h"
#include "ltiImage.h"
#include "ltiMatrix.h"
#include "ltiVector.h"
#include <string>

namespace lti {

  /**
   * Abstract class parent for all viewer objects in the LTI-Lib.
   */
  class viewerBase : public object {
  public:
    /**
     * Base class for all lti parameter objects
     */
    class parameters : public functor::parameters {
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
       * copy data of "other" parameters
       */
      parameters& operator=(const parameters& other);

      /**
       * returns a pointer to a clone of the parameters.
       */
      virtual functor::parameters* clone() const;

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

      // ------------------------
      // the parameters
      // ------------------------

      /**
       * title of the viewer window.
       *
       * Default value: value returned by getTypeName()
       */
      std::string title;

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
       * Constructor with alternative message
       */
      invalidParametersException(const std::string& str)
        : exception(str) {};

      /**
       * returns the name of this exception
       */
      virtual const char* getTypeName() const;
    };

    //

#ifndef SWIG
typedef lti::viewerBase::parameters viewerBase_parameters;
#endif
   
   /**
     * default constructor
     */
    viewerBase();

    /**
     * copy constructor
     */
    viewerBase(const viewerBase& other);

    /**
     * destructor
     */
    virtual ~viewerBase();

    /**
     * returns the name of this type ("viewerBase")
     */
    virtual const char* getTypeName() const;

    /**
     * shows a color image.
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const image& data) = 0;

    /**
     * shows a 8-bit channel
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const channel8& data);

    /**
     * shows a channel or matrix of float
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const channel& data);

    /**
     * shows a channel or matrix of float
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const matrix<float>& data);

    /**
     * shows a vector of double
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const vector<double>& data);

    /**
     * shows a vector of double
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const vector<float>& data);

    /**
     * shows a vector of double
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const vector<int>& data);

    /**
     * shows a matrix of doubles as a channel
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const matrix<double>& data);

    /**
     * shows a matrix of integers as a channel
     * @param data the object to be shown.
     * @return true if successful, false otherwise.
     */
    virtual bool show(const matrix<int>& data);

    /**
     * hides the display window
     * @return true if successful, false otherwise.
     */
    virtual bool hide() = 0;

    /**
     * copy data of "other" functor.
     */
    viewerBase& copy(const viewerBase& other);

    /**
     * copy data of "other" functor.
     */
    viewerBase& operator=(const viewerBase& other);

    /**
     * returns a pointer to a clone of the functor.
     */
    virtual viewerBase* clone() const = 0;

    /**
     * returns used parameters
     */
    const parameters& getParameters() const;

    /**
     * returns used parameters
     */
    parameters& getParameters();

    /**
     * set the parameters to be used.  This object makes a copy of the given
     * object and manages the memory of the copy
     */
    virtual bool setParameters(const parameters& param);

    /**
     * set the parameters to be used.  Just a reference to the given object
     * is done.  The memory managment must be done outside this object.
     *
     * Usually the viewers provide GUI to specify the parameters, that is why
     * the given reference is not const.
     */
    virtual bool useParameters(parameters& param);

    /**
     * returns true if the parameters are valid
     */
    virtual bool validParameters() const;

    /**
     * return the last message set with setStatusString().  This will
     * never return 0.  If no status-string has been set yet an empty string
     * (pointer to a string with only the char(0)) will be returned.
     */
    virtual const char* getStatusString() const;

    /**
     * set a status string.
     *
     * @param msg the const string to be reported next time by
     * getStatusString(). The given string will be copied.
     * This message will be usually set within the apply methods to indicate
     * an error cause.
     *
     * Note that the change of the status string is not considered as
     * a change in the functor status.
     */
    virtual void setStatusString(const char* msg) const;


  private:
    /**
     * the actual parameters
     */
    parameters* params;

    /**
     * own parameters is true, if this instance must take care of the
     * memory management of params, otherwise false
     */
    bool ownParam;

    /**
     * the status string written with setStatusString
     */
    mutable char* statusString;

    /**
     * the empty string returned if the statusString is empty
     */
    static const char *const emptyString;
  };

  /**
   * write the viewerBase::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not.
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler,const viewerBase::parameters& p,
             const bool complete = true);

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler,viewerBase::parameters& p,
             const bool complete = true);
}

#endif

