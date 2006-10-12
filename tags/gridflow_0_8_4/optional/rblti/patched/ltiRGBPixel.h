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
 * file .......: ltiRGBPixel.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 01.11.2002
 * revisions ..: $Id$
 */

#ifndef LTI_RGB_PIXEL_H
#define LTI_RGB_PIXEL_H

#include <iostream>
#include "ltiIoHandler.h"
#include "ltiConfig.h"
#include "ltiAssert.h"
#include "ltiTypes.h"

/**
 * @file ltiRGBPixel.h
 *
 * This file contains the definitions of the basic color types used in
 * the LTI-Lib:
 *  - lti::rgbPixel
 *  - lti::trgbPixel
 */

namespace lti {

  /**
   * Color pixel representation in RGB color space.
   *
   * This type ensures that a size of 32-bits.
   * Each channel use values between 0 and 255, and a fourth "dummy" or "alpha"
   * channel is used to complete the 32-bits.
   *
   * If you need to use pixel values for the RGB channels of other types
   * you can use lti::trgbPixel.
   *
   * @ingroup gColor
   */
  class rgbPixel {
  protected:
    /**
     * @union rgbPixelType
     * this union allows to share the memory of a 32 bit integer with the
     * three ubytes of RGB and a dummy byte.
     *
     * The order of the data in this union assumes little endianness
     */
    union rgbPixelType {
      /**
       * all three channels (and dummy value) together
       */
      uint32 value;

      /**
       * the three channels
       */
      struct channelsType {
      //  the sequence blue, green red _is_ important!

        /**
         * blue channel
         */
        ubyte blue;
        /**
         * green channel
         */
        ubyte green;
        /**
         * red channel
         */
        ubyte red;
        /**
         * dummy channel
         */
        ubyte dummy;
      };

      /**
       * The channel values instance
       */
      channelsType channels;
    };

    /**
     * The data of the pixel
     */
    rgbPixelType data;

  public:

    /**
     * used for the template-based interface for pixels as vectors.
     */
    typedef ubyte value_type;

    /**
     * return type of the size() member
     */
    typedef int size_type;

    /**
     * default constructor
     */
    rgbPixel() {};

    /**
     * constructor with member initialization
     *
     * Per default a new rgbPixel will be initialized with the given value.
     * @param val a 4 byte value to be
     *            assign to the three channels + dummy.
     *             Note that the order depends on the system endianness:
     *             - If you use little endian (for example: Intel Processor)
     *               a value of 0x00010203 means red=01,green=02 and blue=03
     *             - If you use big endian (for example: PowerPC Processor)
     *               a value of 0x00010203 means red=02,green=01 and blue=00
     *             Avoid the use of this constructor if you want to maintain
     *             platform compatibility.
     */
    rgbPixel(const uint32 val) {
      data.value = val;
    };

    /**
     * rgb constructor
     * @param r 8 bit value for the red component
     * @param g 8 bit value for the green component
     * @param b 8 bit value for the blue component
     * @param d 8 bit value for the dummy byte (default value 0)
     */
    rgbPixel(const ubyte r,const ubyte g,const ubyte b,const ubyte d=0) {
      data.channels.dummy = d;
      data.channels.red   = r;
      data.channels.green = g;
      data.channels.blue  = b;
    };

    /**
     * set the red, green, blue and dummy values for the pixel
     */
    inline void set(const ubyte red, const ubyte green, const ubyte blue,
                    const ubyte dummy) {
      data.channels.dummy = dummy;
      data.channels.red   = red;
      data.channels.green = green;
      data.channels.blue  = blue;
    };

    /**
     * set the red, green, blue values for the pixel
     */
    inline void set(const ubyte red, const ubyte green, const ubyte blue) {
      data.channels.red   = red;
      data.channels.green = green;
      data.channels.blue  = blue;
    };

    /**
     * sets the red component to given value
     */
    inline void setRed(const ubyte r) {data.channels.red = r;};

    /**
     * sets the green component to given value
     */
    inline void setGreen(const ubyte g) {data.channels.green = g;};

    /**
     * sets the blue component to given value
     */
    inline void setBlue(const ubyte b) {data.channels.blue = b;};

    /**
     * sets the dummy component to given value
     */
    inline void setDummy(const ubyte d) {data.channels.dummy = d;};

    /**
     * sets the value component (all components together) to given value
     */
    inline void setValue(const uint32& v) {data.value = v;};

    /**
     * Get the three color components and write them in the given arguments
     */
    inline void get(ubyte& red,ubyte& green,ubyte& blue) const {
      red   = data.channels.red;
      green = data.channels.green;
      blue  = data.channels.blue;
    };

    /**
     * Get the three color components and write them in the given arguments
     */
    inline void get(int& red,int& green,int& blue) const {
      red   = data.channels.red;
      green = data.channels.green;
      blue  = data.channels.blue;
    };

    /**
     * Get the four components and write them in the given arguments
     */
    inline void get(ubyte& red,ubyte& green,ubyte& blue,ubyte& dummy) const {
      red   = data.channels.red;
      green = data.channels.green;
      blue  = data.channels.blue;
      dummy = data.channels.dummy;
    };

    /**
     * Get the four components and write them in the given arguments
     */
    inline void get(int& red,int& green,int& blue,int& dummy) const {
      red   = data.channels.red;
      green = data.channels.green;
      blue  = data.channels.blue;
      dummy = data.channels.dummy;
    };

    /**
     * returns red component
     */
    inline const ubyte& getRed()   const {return data.channels.red;};

    /**
     * returns green component
     */
    inline const ubyte& getGreen() const {return data.channels.green;};

    /**
     * returns blue component
     */
    inline const ubyte& getBlue()  const {return data.channels.blue;};

    /**
     * returns dummy component
     */
    inline const ubyte& getDummy() const {return data.channels.dummy;};

    /**
     * returns 4 byte component with RGB value
     */
    inline const uint32& getValue() const {return data.value;};

    /**
     * returns 4 byte component with RGB value
     */
    inline uint32& getValue() {return data.value;};

    /**
     * Used to simulate vector access.
     * The correspondence between the elements of the vector and
     * the color components is at(0) for red, at(1) for green and
     * at(2) for blue.
     */
    inline ubyte& at(const int x) {
      assert(x<3 && x >= 0);
      return ((ubyte*)(&data.value))[2-x];
    }

    /**
     * Used to simulate read-only vector access.
     * The correspondence between the elements of the vector and
     * the color components still depents on the endianness of the
     * system, but is usually at(0) for red, at(1) for green and
     * at(2) for blue.
     */
    inline const ubyte& at(const int x) const {
      assert(x<3 && x >= 0);
      return ((ubyte*)(&data.value))[2-x];
    }

    /**
     * Used to simulate vector access.
     * The correspondence between the elements of the vector and
     * the color components is [0] for red, [1] for green and
     * [2] for blue.
     */
    inline ubyte& operator[](const int x) {
      assert(x<3 && x >= 0);
      return (&data.channels.red)[-x];
    }

    /**
     * Used to simulate read-only vector access.
     * The correspondence between the elements of the vector and
     * the color components still depents on the endianness of the
     * system, but is usually [0] for red, [1] for green and
     * [2] for blue.
     */
    inline const ubyte& operator[](const int x) const {
      assert(x<3 && x >= 0);
      return (&data.channels.red)[-x];
    }

    /**
     * Used to simulate the vector size
     */
    inline int size() const {
      return 3;
    }

    /**
     * copy the "other" pixel
     */
    inline rgbPixel& copy(const rgbPixel& other) {
      setValue(other.getValue());
      return (*this);
    };
    /**
     * alias for copy
     */
    inline rgbPixel& operator=(const rgbPixel& other) {
      return(copy(other));
    };

    /**
     * Compares this pixel with the specified other pixel. 
     * Two pixels are considered equal if all four (!) channels,
     * i.e. R, G, B, and dummy, are equal. A call to this method
     * is equivalent to this->getValue() == other.getValue().
     */
    inline bool isEqual(const rgbPixel& other) const {
      return (getValue() == other.getValue());
    }

    /**
     * Compares the color of this pixel with the specified
     * other pixel. Only the three RGB channels are considered;
     * the dummy value is ignored.
     */
    inline bool isEqualColor(const rgbPixel& other) const {
      return getRed() == other.getRed()
        && getGreen() == other.getGreen() 
        && getBlue() == other.getBlue();
    }

    /**
     * Alias for isEqual(). Please note the difference between
     * isEqual() and isEqualColor()!
     */
    inline bool operator==(const rgbPixel& other) const {
      return (isEqual(other));
    };

    /**
     * alias for !compare()
     */
    inline bool operator!=(const rgbPixel& other) const {
      return (!isEqual(other));
    };

    /**
     * An rgbPixel is said to be "smaller" than another one, if
     * getValue() < other.getValue()
     */
    inline bool operator<(const rgbPixel& other) const {
      return (getValue() < other.getValue());
    }

    /**
     * An rgbPixel is said to be "bigger" than another one, if
     * getValue() > other.getValue()
     */
    inline bool operator>(const rgbPixel& other) const {
      return (getValue() > other.getValue());
    }

    /**
     * add this pixel with another one
     */
    inline rgbPixel& add(const rgbPixel& other) {

      setRed(getRed()+other.getRed());
      setGreen(getGreen()+other.getGreen());
      setBlue(getBlue()+other.getBlue());

      return (*this);
    }

    /**
     * alias for add
     */
    inline rgbPixel& operator+=(const rgbPixel& other) {
      return add(other);
    }

    /**
     * add this pixel with the otherone without altering anything...
     */
    inline rgbPixel operator+(const rgbPixel& other) const {
      rgbPixel tmp(*this);
      tmp.add(other);
      return tmp;
    }

    /**
     * subtract 'other' from this pixel
     */
    inline rgbPixel& subtract(const rgbPixel& other) {

      setRed(getRed()-other.getRed());
      setGreen(getGreen()-other.getGreen());
      setBlue(getBlue()-other.getBlue());

      return (*this);
    }

    /**
     * alias for subtract
     */
    inline rgbPixel& operator-=(const rgbPixel& other) {
      return subtract(other);
    }

    /**
     * subtract 'other' from this pixel without altering anything...
     */
    inline rgbPixel operator-(const rgbPixel& other) const {
      rgbPixel tmp(*this);
      tmp.subtract(other);
      return tmp;
    }

    /**
     * multiply this pixel with another one
     * the pixel multiplication multiplies elementwise the elements of
     * the pixel!
     */
    inline rgbPixel& multiply(const rgbPixel& other) {

      setRed(getRed()*other.getRed());
      setGreen(getGreen()*other.getGreen());
      setBlue(getBlue()*other.getBlue());

      return (*this);
    }

    /**
     * alias for multiply()
     */
    inline rgbPixel& operator*=(const rgbPixel& other) {
      return multiply(other);
    }

    /**
     * multiply this pixel with another one without altering anything...
     */
    inline rgbPixel operator*(const rgbPixel& other) const {
      rgbPixel tmp(*this);
      tmp.multiply(other);
      return tmp;
    }

    /**
     * multiply this pixel with an integer
     */
    inline rgbPixel& multiply(const int& other) {

      setRed(getRed()*other);
      setGreen(getGreen()*other);
      setBlue(getBlue()*other);

      return (*this);
    }

    /**
     * multiply this pixel with a float.
     */
    inline rgbPixel& multiply(const float& other) {

      setRed(static_cast<ubyte>(other*getRed()));
      setGreen(static_cast<ubyte>(other*getGreen()));
      setBlue(static_cast<ubyte>(other*getBlue()));

      return (*this);
    }

    /**
     * multiply this pixel with a float.
     */
    inline rgbPixel& multiply(const double& other) {

      setRed(static_cast<ubyte>(other*getRed()));
      setGreen(static_cast<ubyte>(other*getGreen()));
      setBlue(static_cast<ubyte>(other*getBlue()));

      return (*this);
    }

    /**
     * alias for multiply
     */
    inline rgbPixel& operator*=(const int& other) {
      return multiply(other);
    }

    /**
     * alias for multiply
     */
    inline rgbPixel& operator*=(const float& other) {
      return multiply(other);
    }

    /**
     * alias for multiply
     */
    inline rgbPixel& operator*=(const double& other) {
      return multiply(other);
    }

    /**
     * multiply this pixel with another one without altering anything...
     */
    inline rgbPixel operator*(const int& other) const {
      rgbPixel tmp(*this);
      tmp.multiply(other);
      return tmp;
    }

    /**
     * multiply this pixel with a float an return a new one.
     */
    inline rgbPixel operator*(const float& other) const {
      rgbPixel tmp(*this);
      tmp.multiply(other);
      return tmp;
    }

    /**
     * multiply this pixel with a float an return a new one.
     */
    inline rgbPixel operator*(const double& other) const {
      rgbPixel tmp(*this);
      tmp.multiply(other);
      return tmp;
    }


    /**
     * divide this pixel with another one
     * the pixel division divides elementwise the elements of
     * the pixel!
     */
    inline rgbPixel& divide(const rgbPixel& other) {

      setRed(getRed()/other.getRed());
      setGreen(getGreen()/other.getGreen());
      setBlue(getBlue()/other.getBlue());

      return (*this);
    }

    /**
     * alias for divide()
     */
    inline rgbPixel& operator/=(const rgbPixel& other) {
      return divide(other);
    }

    /**
     * divide this pixel with another one without altering anything...
     */
    inline rgbPixel operator/(const rgbPixel& other) const {
      rgbPixel tmp(*this);
      tmp.divide(other);
      return tmp;
    }

    /**
     * divide this pixel with an integer
     */
    inline rgbPixel& divide(const int& other) {

      setRed(getRed()/other);
      setGreen(getGreen()/other);
      setBlue(getBlue()/other);

      return (*this);
    }

    /**
     * alias for divide
     */
    inline rgbPixel& operator/=(const int& other) {
      return divide(other);
    }

    /**
     * divide this pixel with another one without altering anything...
     */
    inline rgbPixel operator/(const int& other) const {
      rgbPixel tmp(*this);
      tmp.divide(other);
      return tmp;
    }

   /**
    * obtain the square of the magnitud of this pixel
    * \f$red^2+green^2+blue^2 \f$.
    */
    inline int absSqr() const {
      return (int(getRed()  )*int(getRed()  )+
              int(getGreen())*int(getGreen())+
              int(getBlue() )*int(getBlue() ));
    }

   /**
    * get the scalar product of this pixel with another one, considering
    * them as a 3D vector.  The dot product will be the sum of the 
    * red*other.red + green*other.green + blue*other.blue
    */
    inline int dot(const rgbPixel& other) const {
      return (int(getRed()  )*int(other.getRed()  )+
              int(getGreen())*int(other.getGreen())+
              int(getBlue() )*int(other.getBlue() ));
    }

    /**
     * returns the square of the distance of this pixel to the other
     * one defined as
     * \f$(red-other.red)^2+(green-other.green)^2+(blue-other.blue)^2\f$.
     */
    inline int distanceSqr(const rgbPixel& other) const {
      const int 
        r(static_cast<int>(other.getRed())-
          static_cast<int>(data.channels.red)),
        g(static_cast<int>(other.getGreen())-
          static_cast<int>(data.channels.green)),
        b(static_cast<int>(other.getBlue())-
          static_cast<int>(data.channels.blue));
      return (r*r+g*g+b*b);
    }

  };

  /**
   * @name Constants for the primary and secondary colors, as well as
   * black and white.
   */
  //@{
  /**
   * constant for the color black.
   */
  #ifndef SWIG
  const rgbPixel Black(0,0,0,0);

  /**
   * constant for the color read.
   */
  const rgbPixel Red(255,0,0,0);

  /**
   * constant for the color green.
   */
  const rgbPixel Green(0,255,0,0);

  /**
   * constant for the color blue.
   */
  const rgbPixel Blue(0,0,255,0);

  /**
   * constant for the color yellow.
   */
  const rgbPixel Yellow(255,255,0,0);

  /**
   * constant for the color magenta.
   */
  const rgbPixel Magenta(255,0,255,0);

  /**
   * constant for the color cyan.
   */
  const rgbPixel Cyan(0,255,255,0);

  /**
   * constant for the color white.
   */
  const rgbPixel White(255,255,255,0);
  //@}
#endif
  /**
   * read the vector from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   *
   * @ingroup gStorable
   */
  inline bool read(ioHandler& handler,rgbPixel& p,const bool complete=true) {
    int tmp;
    bool b = true;

    // the begin and end tokens are mandatory here! ignore the complete flag...
    handler.readBegin();

    b = b && handler.read(tmp);
    p.setRed(ubyte(tmp));

    b = b && handler.readDataSeparator();

    b = b && handler.read(tmp);
    p.setGreen(ubyte(tmp));

    b = b && handler.readDataSeparator();

    b = b && handler.read(tmp);
    p.setBlue(ubyte(tmp));

    if (!handler.tryEnd()) {
      // the new rgbPixel writes also the dummy value.  To allow
      // old stuff to be readed, the dummy value is optionally readed
      b = b && handler.readDataSeparator();

      b = b && handler.read(tmp);
      p.setDummy(ubyte(tmp));

      // the begin and end tokens are mandatory here! ignore the
      // complete flag...
      b = b && handler.readEnd();
    } else {
      p.setDummy(0);
    }

    return b;
  };

  /**
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   *
   * @ingroup gStorable
   */
  inline bool write(ioHandler& handler,const rgbPixel& p,
                    const bool complete=true) {
    bool b = true;

    // the begin token is mandatory here, so ignore the complete flag
    b = b && handler.writeBegin();

    b = b && handler.write(int(p.getRed()));
    b = b && handler.writeDataSeparator();
    b = b && handler.write(int(p.getGreen()));
    b = b && handler.writeDataSeparator();
    b = b && handler.write(int(p.getBlue()));
    b = b && handler.writeDataSeparator();
    b = b && handler.write(int(p.getDummy()));
    b = b && handler.writeEnd();

    return b;
  };

  /**
   * Template to use RGB pixel representations of types other than
   * <code>ubyte</code>.
   *
   * The type <code>rgbPixel</code> optimize speed and memory use.
   * If you need RGB integer values between 0 and 255, you should use rgbPixel
   * instead (much more efficient!)
   *
   * @see lti::drgbPixel, lti::frgbPixel, lti::irgbPixel
   *
   * @ingroup gColor
   */
  template<class T>
  class trgbPixel {
  public:

    /**
     * used for the template-based interface for pixels as vectors.
     */
    typedef T value_type;

    /**
     * return type of the size() member
     */
    typedef int size_type;

    /**
     * default constructor
     */
    trgbPixel()
      : red(T(0)),green(T(0)),blue(T(0)) {
    };

    /**
     * gray value constructor
     */
    explicit trgbPixel(const T g)
      : red(g),green(g),blue(g) {
    };

    /**
     * default constructor
     */
    trgbPixel(const T r,
              const T g,
              const T b)
      : red(r),green(g),blue(b) {};

    /**
     * copy constructor
     */
    trgbPixel(const trgbPixel<T>& p)
      : red(p.red),green(p.green),blue(p.blue) {};

    /**
     * copy constructor
     */
    trgbPixel(const rgbPixel& p)
      : red(p.getRed()),green(p.getGreen()),blue(p.getBlue()) {
    };

    /**
     * red channel
     */
    T red;

    /**
     * green channel
     */
    T green;

    /**
     * blue channel
     */
    T blue;

    /**
     * Get the three color components and write them in the given arguments
     */
    inline void get(T& r,T& g,T& b) const {
      r = red;
      g = green;
      b = blue;
    };

    /**
     * returns red value
     */
    inline const T& getRed() const {return red;};

    /**
     * returns green value
     */
    inline const T& getGreen() const {return green;};

    /**
     * returns blue value
     */
    inline const T& getBlue() const {return blue;};

    /**
     * set the red, green, blue values for the pixel
     */
    inline void set(const T r, const T g, const T b) {
      red   = r;
      green = g;
      blue  = b;
    };

    /**
     * sets red value
     */
    inline void setRed(const T r)   {red   = r;};

    /**
     * sets green value
     */
    inline void setGreen(const T g) {green = g;};

    /**
     * sets blue value
     */
    inline void setBlue(const T b)  {blue  = b;};


    /**
     * Used to simulate vector access.  It is slower than the normal
     * access to the elements with getRed() or setRed() similar methods.
     *
     * The correspondence between the elements of the vector and
     * the color components will be at(0) for red, at(1) for green and
     * at(2) for blue.
     */
    inline T& at(const int x) {
      assert(x<4);
      switch (x) {
        case 0:
          return red;
        case 1:
          return green;
        case 2:
          return blue;
        default:
          assert(false);
      }

      return red; // we need to return something to keep the compiler quiet!
    }

    /**
     * Used to simulate read-only vector access.  It is slower than the normal
     * access to the elements with getRed() or similar methods.
     *
     *
     * The correspondence between the elements of the vector and
     * the color components will be at(0) for red, at(1) for green and
     * at(2) for blue.
     */
    inline const T& at(const int x) const {
      assert(x<4);
      switch (x) {
        case 0:
          return red;
        case 1:
          return green;
        case 2:
          return blue;
        default:
          assert(false);
      }

      return red; // we need to return something to keep the compiler quiet!
    }

    /**
     * Used to simulate vector access.  It is slower than the normal
     * access to the elements with getRed() or setRed() similar methods.
     *
     * The correspondence between the elements of the vector and
     * the color components will be [0] for red, [1] for green and
     * [2] for blue.
     */
    inline T& operator[](const int x) {
      assert(x<4);
      switch (x) {
        case 0:
          return red;
        case 1:
          return green;
        case 2:
          return blue;
        default:
          assert(false);
      }

      return red; // we need to return something to keep the compiler quiet!
    }

    /**
     * Used to simulate read-only vector access.  It is slower than the normal
     * access to the elements with getRed() or similar methods.
     *
     * The correspondence between the elements of the vector and
     * the color components will be [0] for red, [1] for green and
     * [2] for blue.
     */
    inline const T& operator[](const int x) const {
      assert(x<4);
      switch (x) {
        case 0:
          return red;
        case 1:
          return green;
        case 2:
          return blue;
        default:
          assert(false);
      }

      return red; // we need to return something to keep the compiler quiet!
    }



    /**
     * Used to simulate the vector size
     */
    inline int size() const {
      return 3;
    }

    /**
     * copy member
     */
    trgbPixel<T>& copy(const trgbPixel<T>& other) {
      red   = other.red;
      green = other.green;
      blue  = other.blue;
      return (*this);
    };

    /**
     * copy member
     */
    trgbPixel<T>& copy(const rgbPixel& other) {
      red   = T(other.getRed());
      green = T(other.getGreen());
      blue  = T(other.getBlue());
      return (*this);
    }

    /**
     * alias for copy member
     */
    trgbPixel<T>& operator=(const trgbPixel<T>& other) {
      return(copy(other));
    };

    /**
     * alias for copy member
     */
    trgbPixel<T>& operator=(const rgbPixel& other) {
      return(copy(other));
    };

    /**
     * convert between pixels of different types
     */
    template <class U>
    inline trgbPixel<T>& castFrom(const trgbPixel<U>& other) {
      red   = static_cast<T>(other.getRed());
      green = static_cast<T>(other.getGreen());
      blue  = static_cast<T>(other.getBlue());
      return (*this);
    }

    /**
     * compare member: returns true if this is equal to other
     */
    bool isEqual(const trgbPixel<T>& other) const {
      return ((red == other.red) &&
        (green == other.green) &&
        (blue == other.blue));
    };

    /**
     * alias for compare()
     */
    inline bool operator==(const trgbPixel<T>& other) const {
      return (isEqual(other));
    };

    /**
     * alias for !compare()
     */
    inline bool operator!=(const trgbPixel<T>& other) const {
      return (!isEqual(other));
    };

    /**
     * returns true if the intensity of this color is smaller than
     * the intensity of the other color
     */
    inline bool operator<(const trgbPixel<T>& other) const {
      return ((red+green+blue) < (other.red+other.green+other.blue));
    }

    /**
     * returns true if the intensity of this color is bigger than
     * the intensity of the other color
     */
    inline bool operator>(const trgbPixel<T>& other) const {
      return ((red+green+blue) > (other.red+other.green+other.blue));
    }

    /**
     * add two pixels
     */
    trgbPixel<T>& add(const trgbPixel<T>& other) {
      red += other.red;
      green += other.green;
      blue += other.blue;
      return (*this);
    };

    /**
     * Add pixels a and b leave the result here.
     */
    trgbPixel<T>& add(const trgbPixel<T>& a,
                      const trgbPixel<T>& b) {

      red   = a.red   + b.red;
      green = a.green + b.green;
      blue  = a.blue  + b.blue;

      return (*this);
    };

    /**
     * add a constant to each component of the pixel
     */
    trgbPixel<T>& add(const T val) {
      red += val;
      green += val;
      blue += val;
      return (*this);
    };

    /**
     * alias for add
     */
    inline trgbPixel<T>& operator+=(const trgbPixel<T>& other) {
      return add(other);
    }

    /**
     * add this pixel with the otherone without altering anything...
     */
    inline trgbPixel<T> operator+(const trgbPixel<T>& other) const {
      trgbPixel<T> tmp(*this);
      tmp.add(other);
      return tmp;
    }

    /**
     * add to this pixel the other pixel scaled by the given constant
     */
    inline trgbPixel<T> addScaled(const T b,const trgbPixel<T>& other) {
      red += (other.red*b);
      green += (other.green*b);
      blue += (other.blue*b);
      return (*this);
    }

    /**
     * assign to this pixel the sum of the scaled pixels with their
     * respective scaling factors, i.e. *this = pa*a + pb*b
     */
    inline trgbPixel<T> addScaled(const T a,const trgbPixel<T>& pixa,
                                  const T b,const trgbPixel<T>& pixb) {
      red   = (pixa.red*a) + (pixb.red*b);
      green = (pixa.green*a) + (pixb.green*b);
      blue  = (pixa.blue*a) + (pixb.blue*b);
      return (*this);
    }

    /**
     * assign to this pixel the sum of the pixel pixa and the scaled
     * pixel pixb (scaling factor b), i.e. *this = pa + pb*b
     */
    inline trgbPixel<T> addScaled(const trgbPixel<T>& pixa,
                                  const T b,const trgbPixel<T>& pixb) {
      red   = (pixa.red) + (pixb.red*b);
      green = (pixa.green) + (pixb.green*b);
      blue  = (pixa.blue) + (pixb.blue*b);
      return (*this);
    }

    /**
     * scale this pixel with a, and add to the result the scaled
     * pixel pixb (scaling factor b), i.e. *this = *this*a + pb*b
     */
    inline trgbPixel<T> addScaled(const T a,
                                  const T b,const trgbPixel<T>& pixb) {
      red   = (red*a) + (pixb.red*b);
      green = (green*a) + (pixb.green*b);
      blue  = (blue*a) + (pixb.blue*b);
      return (*this);
    }

    /**
     * Subtract the \a other pixel from the current one and leave the result
     * here.
     */
    trgbPixel<T>& subtract(const trgbPixel<T>& other) {

      red -= other.red;
      green -= other.green;
      blue -= other.blue;

      return (*this);
    };

    /**
     * Subtract pixel b from pixel a and leave the result here.
     */
    trgbPixel<T>& subtract(const trgbPixel<T>& a,
                           const trgbPixel<T>& b) {

      red   = a.red   - b.red;
      green = a.green - b.green;
      blue  = a.blue  - b.blue;

      return (*this);
    };


    /**
     * subtract the given value from each component of the pixel
     */
    trgbPixel<T>& subtract(const T val) {
      red -= val;
      green -= val;
      blue -= val;
      return (*this);
    };

    /**
     * alias for subtract
     */
    inline trgbPixel<T>& operator-=(const trgbPixel<T>& other) {
      return subtract(other);
    }

    /**
     * subtract 'other' from this pixel without altering anything...
     */
    inline trgbPixel<T> operator-(const trgbPixel<T>& other) const {
      trgbPixel tmp(*this);
      tmp.subtract(other);
      return tmp;
    }

    /**
     * multiply with another pixel.
     * The pixel multiplication is a elementwise multiplication
     */
    trgbPixel<T>& multiply(const trgbPixel<T>& other) {
      red   *= other.red;
      green *= other.green;
      blue  *= other.blue;
      return (*this);
    }

    /**
     * Elementwise multiplication of the elements of a and b
     */
    trgbPixel<T>& multiply(const trgbPixel<T>& a,
                           const trgbPixel<T>& b) {

      red   = a.red   * b.red;
      green = a.green * b.green;
      blue  = a.blue  * b.blue;

      return (*this);
    };

    /**
     * multiply with a constant
     */
    trgbPixel<T>& multiply(const T c) {
      red   *= c;
      green *= c;
      blue  *= c;
      return (*this);
    }

    /**
     * multiply the elements of the other pixel with the given value, and
     * leave the result here.
     */
    inline trgbPixel<T>& multiply(const trgbPixel<T>& other, const T val) {
      red = other.red*val;
      green = other.green*val;
      blue = other.blue*val;
      return (*this);
    }

    /**
     * alias for multiply
     * The pixel multiplication is a elementwise multiplication
     */
    inline trgbPixel<T>& operator*=(const trgbPixel<T>& other) {
      return multiply(other);
    }

    /**
     * multiply this pixel with another one without altering anything...
     */
    inline trgbPixel<T> operator*(const trgbPixel<T>& other) const {
      trgbPixel<T> tmp(*this);
      tmp.multiply(other);
      return tmp;
    }

    /**
     * alias for multiply
     */
    inline trgbPixel<T> operator*=(const T other) {
      return multiply(other);
    }

    /**
     * multiply this pixel with another one without altering anything...
     */
    inline trgbPixel<T> operator*(const T c) const {
      return trgbPixel<T>(red*c,green*c,blue*c);
    }

    /**
     * divide by a constant
     */
    inline trgbPixel<T>& divide(const T c) {
      red   /= c;
      green /= c;
      blue  /= c;
      return (*this);
    }

    /**
     * divide by a constant
     */
    inline trgbPixel<T> operator/(const T c) const {
      return trgbPixel<T>(red/c,green/c,blue/c);
    }

    /**
     * divide the elements of the other pixel with the given value, and
     * leave the result here.
     */
    inline trgbPixel<T>& divide(const trgbPixel<T>& other, const T val) {
      red = other.red/val;
      green = other.green/val;
      blue = other.blue/val;
      return (*this);
    }

    /**
     * alias for divide
     */
    inline trgbPixel<T>& operator/=(const T c) {
      return divide(c);
    }

    /**
     * elementwise divide
     */
    inline trgbPixel<T>& divide(const trgbPixel<T>& c) {
      red   /= c.red;
      green /= c.green;
      blue  /= c.blue;
      return (*this);
    }

    /**
     * Elementwise divide between the elements of a and b
     */
    trgbPixel<T>& divide(const trgbPixel<T>& a,
                         const trgbPixel<T>& b) {

      red   = a.red   / b.red;
      green = a.green / b.green;
      blue  = a.blue  / b.blue;

      return (*this);
    };

    /**
     * elementwise divide
     */
    inline trgbPixel<T> operator/(const trgbPixel<T>& c) const {
      trgbPixel tmp(*this);
      tmp.divide(c);
      return tmp;
    }


    /**
     * alias for elementwise divide
     */
    inline trgbPixel<T>& operator/=(const trgbPixel<T>& c) {
      return divide(c);
    }

    /**
     * obtain the square of the magnitud of this pixel
     * \f$red^2+green^2+blue^2 \f$.
     */
    inline T absSqr() const {
      return (red*red+green*green+blue*blue);
    }

    /**
     * returns the square of the distance of this pixel to the other
     * one defined as
     * \f$(red-other.red)^2+(green-other.green)^2+(blue-other.blue)^2\f$.
     */
    inline T distanceSqr(const trgbPixel<T>& other) const {
      const T r(red-other.red),g(green-other.green),b(blue-other.blue);
      return (r*r+g*g+b*b);
    }

    /**
     * computes the dot product with another pixel, which is define as
     * the sum of the products of all corresponding components, i.e.
     * p1.red*p2.red + p1.green*p2.green + p1.blue*p2.blue, where p1 is
     * this pixel.
     */
    inline T dot(const trgbPixel<T>& other) const {
      return (red*other.red + green*other.green + blue*other.blue);
    }

    /**
     * return a normal lti::rgbPixel
     * You should ensure that the values are all in a valid interval!
     * @see getClippedRGBPixel()
     */
    inline rgbPixel getRGBPixel() const {
      return rgbPixel(static_cast<ubyte>(red),
                      static_cast<ubyte>(green),
                      static_cast<ubyte>(blue));
    }

    /**
     * return a normal lti::rgbPixel
     * Values lower than 0 are set to 0 and values higher than 255 are set to
     * 255.
     *
     * This method is slower than getRGBPixel() due to the extra comparisons
     *
     * @see getRGBPixel()
     */
    inline rgbPixel getClippedRGBPixel() const {
      return rgbPixel(static_cast<ubyte>(red<0?0:(red>255?255:red)),
                      static_cast<ubyte>(green<0?0:(green>255?255:green)),
                      static_cast<ubyte>(blue<0?0:(blue>255?255:blue)));
    }

    /**
     * apply a C-function to each component of the pixel
     * @return a reference to the pixel
     */
    inline trgbPixel<T>& apply(T (*function)(T)) {
      red = (*function)(red);
      green = (*function)(red);
      blue = (*function)(blue);

      return (*this);
    }

    /**
     * apply a C-function to each component of the pixel
     * @return a reference to the pixel
     */
    inline trgbPixel<T>& apply(T (*function)(const T&)) {
      red = (*function)(red);
      green = (*function)(red);
      blue = (*function)(blue);

      return (*this);
    }

  };

  /**
   * alias for trbgPixel<double>
   */
  typedef trgbPixel<double> drgbPixel;
  /**
   * alias for trgbPixel<float>
   */
  typedef trgbPixel<float>  frgbPixel;
  /**
   * alias for trgbPixel<int>
   */
  typedef trgbPixel<int>    irgbPixel;


  /**
   * read the vector from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   *
   * @ingroup gStorable
   */
  template <class T>
  bool read(ioHandler& handler,
            trgbPixel<T>& p,
            const bool complete=true) {
    T tmp;
    bool b;

    if (complete) {
      handler.readBegin();
    }

    handler.read(tmp); p.setRed(tmp);
    handler.readDataSeparator();
    handler.read(tmp); p.setGreen(tmp);
    handler.readDataSeparator();
    b = handler.read(tmp); p.setBlue(tmp);

    if (complete) {
      b = handler.readEnd();
    }

    return b;
  };

  /**
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   *
   * @ingroup gStorable
   */
  template <class T>
  bool write(ioHandler& handler,
             const trgbPixel<T>& p,
             const bool complete=true) {
    bool b;

    if (complete) {
      handler.writeBegin();
    }
    handler.write(p.getRed());
    handler.writeDataSeparator();
    handler.write(p.getGreen());
    handler.writeDataSeparator();
    b = handler.write(p.getBlue());
    if (complete) {
      b = handler.writeEnd();
    }

    return b;
  };
}

namespace std {
  inline ostream& operator<<(ostream& s,const lti::rgbPixel& p) {
    s << "(" << int(p.getRed())   << ","
      << int(p.getGreen()) << ","
      << int(p.getBlue())  << ")";
    return s;
  };

  //inline ostream& operator>>(istream& s,const lti::rgbPixel& p);
  inline istream& operator>>(istream& s,lti::rgbPixel& p) {
    char c;
    int r,g,b;
    s >> c
      >> r >> c
      >> g >> c
      >> b >> c;
    p.setRed(static_cast<lti::ubyte>(r));
    p.setGreen(static_cast<lti::ubyte>(g));
    p.setBlue(static_cast<lti::ubyte>(b));

    return s;
  };

  template <class T>
  inline ostream& operator<<(ostream& s,const lti::trgbPixel<T>& p) {
    s << "("
      << p.getRed()   << ","
      << p.getGreen() << ","
      << p.getBlue()  << ")";
    return s;
  };
}

#endif




