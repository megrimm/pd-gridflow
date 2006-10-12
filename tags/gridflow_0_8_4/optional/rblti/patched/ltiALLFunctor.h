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

#ifndef _LTI_ALLFUNCTOR_H_
#define _LTI_ALLFUNCTOR_H_

#include "ltiConfig.h"
#include "ltiRGBPixel.h"
#include "ltiBMPFunctor.h"
#include "ltiJPEGFunctor.h"
#include "ltiPNGFunctor.h"
#include "ltiImage.h"

#include "ltiSplitImageToRGB.h" //for grey jpeg/png images
#include "ltiSplitImageToHSI.h" //for grey jpeg/png images

#include <fstream>

// define one macro if the support for jpeg is enabled
#undef LTI_HAVE_JPEG
#if defined(HAVE_LOCALJPEG) || defined(HAVE_LIBJPEG)
#define LTI_HAVE_JPEG 1
#endif

// define one macro if the support for png is enabled
#undef LTI_HAVE_PNG
#if defined(HAVE_LOCALPNG) || defined(HAVE_LIBPNG)
#define LTI_HAVE_PNG 1
#endif

namespace lti {
  /**
   * \file ltiALLFunctor.h provides classes to read and write images
   *                       in all formats supported by the LTI-Lib.
   *                       At this time these formats are PNG,JPEG,BMP
   */

  /**
   * Base class to functors which load and save images in all formats.
   * The parameter class for this functor is lti::ioImage::parameters.
   *
   * See the derived classes for more information.
   */
  class ioImage : public ioFunctor {
  public:
    /**
     * Parameter class of the ioImage class
     */
    class parameters : public ioFunctor::parameters {
    public:
      /**
       * default constructor
       */
      parameters();
      /**
       * copy constructor
       */
      parameters(const parameters& other) : ioFunctor::parameters() {
        copy(other);
      };

      /**
       * copy member
       */
      parameters& copy(const parameters& other);

      /**
       * returns a pointer to a clone of the parameters.
       */
      virtual functor::parameters* clone() const;

      /**
       * returns name of this class
       */
      virtual const char* getTypeName() const;

      /**
       * write the parameters in the given ioHandler
       * @param handler the ioHandler to be used
       * @param complete if true (the default) the enclosing begin/end will
       *        be also written, otherwise only the data block will be written.
       * @return true if write was successful
       */
      virtual bool write(ioHandler& handler,const bool complete=true) const;

      /**
       * write the parameters in the given ioHandler
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
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use read() instead!
       */
      bool readMS(ioHandler& handler,const bool complete=true);

      /**
       * this function is required by MSVC only, as a workaround for a
       * very awful bug, which exists since MSVC V.4.0, and still by
       * V.6.0 with all bugfixes (so called "service packs") remains
       * there...  This method is public due to another bug, so please
       * NEVER EVER call this method directly: use write() instead!
       */
      bool writeMS(ioHandler& handler,const bool complete=true) const;
#     endif

      // ------------------------------------------------
      // the parameters
      // ------------------------------------------------

      /**
       * Parameter object used when loading/saving BMP images
       */
      ioBMP::parameters paramBMP;

#ifdef LTI_HAVE_JPEG
      /**
       * Parameter object used when loading/saving JPEG images.
       *
       * You need to have enabled at least one of both kinds 
       * of JPEG support enabled to use this.  See the FAQ for
       * more information.
       */
      ioJPEG::parameters paramJPEG;
#endif

#ifdef LTI_HAVE_PNG
      /**
       * Parameter object used when loading/saving PNG images
       *
       * You need to have enabled at least one of both kinds 
       * of PNG support enabled to use this.  See the FAQ for
       * more information.
       */
      ioPNG::parameters paramPNG;
#endif
    };

    /**
     * default constructor
     */
    ioImage();

    /**
     * destructor
     */
    ~ioImage() {};

    /**
     * returns parameters.
     */
    const parameters& getParameters() const;

    /**
     * returns current parameters.
     */
    const ioFunctor::parameters& getCurrParameters() const;

#ifndef SWIG
    /**
     * Set the parameters of this functor.
     */
    virtual bool setParameters (const functor::parameters& theParam );

    /**
     * Shortcut to set the parameters for BMP files only.
     */
    bool setParameters(const ioBMP::parameters& theParam );

#ifdef LTI_HAVE_JPEG
    /**
     * Shortcut to set the parameters for JPEG files only.
     */
    bool setParameters(const ioJPEG::parameters& theParam );
#endif
#ifdef LTI_HAVE_PNG
    /**
     * Shortcut to set the parameters for PNG files only.
     */
    bool setParameters(const ioPNG::parameters& theParam );
#endif
#else
    virtual bool setParameters(const functor::parameters & theParam);
    bool setParameters(const ioBMP_parameters & theParam);
    bool setParameters(const ioJPEG_parameters & theParam);
    bool setParameters(const ioPNG_parameters & theParam);
#endif // SWIG
    /**
     * returns the name of this type
     */
    virtual const char* getTypeName() const;

    /**
     * Image File Type
     */
    enum eImageFileType {
      UNKNOWN = -1, /*! Unknown image file format */
      BMP = 0,      /*! MS Bitmap Format */
      JPEG,         /*! JPEG Standard */
      PNG           /*! Portable Network Graphics */
    };

    bool setType(const eImageFileType& theType);

  protected:
    /**
     * stores the actual extension type
     */
    eImageFileType type;

    /**
     * analyses the extension of filename and
     * returns the index of the filetype
     */
    eImageFileType getExtensionType(const std::string& filename) const;

    /**
     * set the filename in the corresponding(type) parameter
     */
    bool setFilenameInParam(const std::string& filename,
			    const eImageFileType& type);
  };

  /**
   * Functor to read an image file.
   *
   * It is NOT thread save, this means, the SAME instance can not be
   * used from different threads or processes at the same time.  If
   * this occurs an unpredictible behaviour must be expected!.  If you
   * need to read or write many images at the same time, use in each
   * thread an instance of this functor, or protect your code with
   * semaphores.
   *
   * This class is used to load lti::image or lti::channel objects
   * from files of one of the formats supported by the LTI-Lib.
   *
   * The load methods will use the extension in the filename to decide
   * the format in which the image should be read.
   *
   * Three formats are supported: PNG, BMP and JPEG.
   *
   * Example:
   *
   * \code
   * lti::loadImage loader; // functor to load images
   * lti::image img;        // the image loaded will be left here.
   *
   * loader.load("myimage.png",img);  // load PNG image
   * loader.load("myimage.bmp",img);  // load BMP image
   * loader.load("myimage.jpg",img);  // load JPG image
   * \endcode
   *
   * The valid extensions are case insensitive matches for .jpg, .jpeg, .png
   * and .bmp.
   */
  class loadImage : public ioImage {
  public:
    /**
     * default constructor
     */
    loadImage();

    /**
     * destructor
     */
    ~loadImage()  {};

    /**
     * returns the name of this type ("loadImage")
     */
    virtual const char* getTypeName() const;

    /**
     * load Image
     * @param theImage the file specified in the parameters will be loaded
     *                 in this image.  Note that independently of the soft
     *                 of image in the file, this will always be converted to
     *                 a color lti::image.
     * @return a reference to the loaded image.
     */
    bool apply(image& theImage);

    /**
     * load channel
     *
     * Use this method if you know that the file contains a gray
     * valued image.  If you try to load a 24-bit image with this
     * method, some quantization algorithms will be used to reduce the
     * number of colors to 256.
     *
     * @param theChannel the image on the file will be loaded here
     * @param colors theChannel contains just indexes to the pixel values
     *               in this vector.
     * @return true if successful, false otherwise.
     */
    bool apply(channel8& theChannel,lti::palette& colors);

    /**
     * load channel
     *
     * Use this method if you know that the file contains a gray
     * valued image.  If you try to load a 24-bit image with this
     * method, some quantization algorithms will be used to reduce the
     * number of colors to 256.
     *
     * @param theChannel the image on the file will be loaded here
     * @return true if successful, false otherwise.
     */
    bool apply(channel& theChannel);

    /**
     * shortcut for load an image
     * @param filename name of the file to be readed
     * @param theImage variable where the image will to be stored
     */
    bool load(const std::string& filename,image& theImage);

    /**
     * shortcut for load an image.
     *
     * Use this method if you know that the file contains a gray valued image.
     * If you try to load a 24-bit image with this method, some quantization
     * algorithms will be used to reduce the number of colors to 256.
     *
     * @param filename name of the file to be readed
     * @param theChannel variable where the image will be stored
     * @param colors the palette used will be stored here
     */
    bool load(const std::string& filename,
              channel8& theChannel,
              lti::palette& colors);

    /**
     * shortcut for load an image.
     *
     * Use this method if you know that the file contains a gray valued image.
     * If you try to load a 24-bit image with this method, some quantization
     * algorithms will be used to reduce the number of colors to 256.
     *
     * @param filename name of the file to be readed
     * @param theChannel variable where the image will be stored
     */
    bool load(const std::string& filename,
              channel& theChannel);


    /**
     * check the data of the image header
     * @param filename name of the bitmap file to be tested
     * @param imageSize returns the size of the bitmap: imageSize.x is the
     *                  number of columns and imageSize.y the numbeer of
     *                  rows.
     * @return true if file is ok
     */
    bool checkHeader(const std::string& filename,
                     point& imageSize);

    /**
     * check the data of the image header
     * @param filename name of the bitmap file to be tested
     * @param imageSize returns the size of the bitmap: imageSize.x is the
     *                  number of columns and imageSize.y the numbeer of
     *                  rows.
     * @param trueColor true if the image contains 3x256 bits per pixel,
     *                  false, otherwise.  If false, you can use the
     *                  methods to read channel8+palette
     * @return true if file is ok
     */
    bool checkHeader(const std::string& filename,
                     point& imageSize,
                     bool& trueColor);

    /**
     * returns a pointer to a clone of the functor.
     */
    virtual functor* clone() const;

  protected:
    /**
     * functor to load BMP images
     */
    lti::loadBMP loaderBMP;

#ifdef LTI_HAVE_JPEG
    /**
     * functor to load JPEG images
     */
    lti::loadJPEG loaderJPEG;
#endif

#ifdef LTI_HAVE_PNG
    /**
     * functor to load PNG images
     */
    lti::loadPNG loaderPNG;
#endif

    /** functor to split images to RGB */
    lti::splitImageToRGB rgbSplitter;

    /** functor to split images to HSI */
    lti::splitImageToHSI hsiSplitter;

  };

  /**
   * This class is used to save lti::image or lti::channel objects
   * in files of one of the formats supported by the LTI-Lib.
   *
   * The save methods will use the extension in the filename to decide
   * the format that should be used to save the image.
   *
   * Three formats are supported: PNG, BMP and JPEG.
   *
   * Example:
   *
   * \code
   * lti::saveImage saver; // functor to save images
   * lti::image img;       // the image to be saved
   *
   * // ... create an image somehow ...
   *
   * saver.save("myimage.png",img);  // save as PNG
   * saver.save("myimage.bmp",img);  // save as BMP
   * saver.save("myimage.jpg",img);  // save as JPEG
   * \endcode
   *
   * The valid extensions are case insensitive matches for .jpg, .jpeg, .png
   * and .bmp.
   */
  class saveImage : public ioImage {
  public:
    /**
     * default constructor
     */
    saveImage();

    /**
     * destructor
     */
    ~saveImage() {};

    /**
     * returns the name of this type
     */
    virtual const char* getTypeName() const;

    /**
     * save image.
     * @param theImage the image to be stored.
     */
    bool apply(const image& theImage);

    /**
     * save float channel as a gray-valued image.
     * The name of the file is taken from the parameters.
     * @param theChannel channel to be stored.
     *                   This channel must contain values between
     *                   0.0f and 1.0f.
     * @return true if successful, false otherwise
     */
    bool apply(const channel& theChannel);

    /**
     * save 8-bit channel as a gray-valued bitmap or as an indexed image.
     * The name of the file is taken from the parameters.
     * @param theChannel the channel to be saved
     * @param colors if a color palette is given, <code>theChannel</code>
     *               will be considered to contain indices to this palette.
     *               If nothing (or an empty palette) is given,
     *               <code>theChannel</code> will be considered as a gray
     *               valued channel.
     * @return true if successful, false otherwise
     */
    bool apply(const channel8& theChannel,
               const lti::palette& colors=emptyPalette);

    /**
     * This will save an image as a 24 bit RGB bitmap image.
     *
     * @param filename the name for the image file.  Its extention
     *                 will determine the format to be used.  The valid
     *                 extensions are case insensitive matches for .jpg, .jpeg,
     *                 .png * and .bmp.
     * @param theImage the image to be written.
     * @return true if successful, false otherwise
     */
    bool save(const std::string& filename,
              const image& theImage);

    /**
     * This will save a channel8 as an 8 bit RGB bitmap image
     *
     * @param filename the name for the image file.  Its extention
     *                 will determine the format to be used.  The valid
     *                 extensions are case insensitive matches for .jpg, .jpeg,
     *                 .png * and .bmp.
     * @param theChannel the channel to be save
     * @param colors the palette to be used
     *               (see apply(const channel& theChannel) for details)
     */
    bool save(const std::string& filename,
              const channel8& theChannel,
              const lti::palette& colors=emptyPalette);

    /**
     * This will save a channel as an 8 bit RGB bitmap image.
     *
     * The values of the channel must be between 0.0f and 1.0f!
     *
     * @param filename the name for the image file.  Its extention
     *                 will determine the format to be used.  The valid
     *                 extensions are case insensitive matches for .jpg, .jpeg,
     *                 .png * and .bmp.
     * @param theChannel the channel to be save
     */
    bool save(const std::string& filename,
              const channel& theChannel);

    /**
     * returns a pointer to a clone of the functor.
     */
    virtual functor* clone() const;

  protected:
    /**
     * functor to save BMP images
     */
    lti::saveBMP saverBMP;

#ifdef LTI_HAVE_JPEG
    /**
     * functor to save JPEG images
     */
    lti::saveJPEG saverJPEG;
#endif

#ifdef LTI_HAVE_PNG
    /**
     * functor to save PNG images
     */
    lti::savePNG saverPNG;
#endif

  };
}  //namespace lti

#endif

