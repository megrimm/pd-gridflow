#!/usr/bin/env python
#
# setup for pylti
#
#******************************************************************************
#
#      project:  pylti
#    copyright:  (c) 2005 by Michael Neuroth
#
#  description:  python modul for ltilib (using SWIG)
#
#  $Source$
#  $Revision$
#  $Date$
# 
#  $Log$
#  Revision 1.1  2006/02/25 23:18:57  matju
#  0.33.1
#
#  Revision 1.12  2005/12/25 16:41:20  Michael
#  better support for installation
#
#  Revision 1.11  2005/12/25 15:46:21  Michael
#  improved build process, splitted lti.i into different interface-files
#
#  Revision 1.10  2005/07/09 13:57:54  Michael
#  Patch to compile pylti under linux (set basedir to /usr/local)
#
#  Revision 1.9  2005/07/04 18:38:40  Michael
#  Added support to build pylti as a misc-project in the ltilib distribution.
#
#  Revision 1.8  2005/06/28 21:40:18  Michael
#  write output files in a directory (generated).
#
#  Revision 1.7  2005/06/28 20:38:40  Michael
#  Update infos about the project
#
#  Revision 1.6  2005/06/17 21:04:46  Michael
#  Some debug options added in comment
#
#  Revision 1.5  2005/06/14 20:17:40  Michael
#  Documentation updated. setup.cfg will now be generated, this results in a better swig support.
#
#  Revision 1.4  2005/05/16 13:40:19  Michael
#  Added lti_manual_impl.cpp.
#
#  Revision 1.3  2005/02/19 22:28:33  Michael
#  Fixes for the linux port
#
#  Revision 1.2  2005/02/14 22:15:40  Michael
#  improvements and updates for the new project structure
#
#  Revision 1.1  2005/02/12 16:42:30  Michael
#  build python modul with python setup
#
#
#
#******************************************************************************

#******************************************************************************
# requirements:
#   * compiled ltilib (with MFC_Shared_Release=1)
#     to compile ltilib: modify ../import/ltilib/win/buildLib_net/buildlib.net.bat and execute batch
#
#******************************************************************************
# call: 
#  * python setup.py build_ext
#  * python setup.py install
#  * python setup.py bdist_wininst
#  * python setup.py bdist_rpm
#
#******************************************************************************

from distutils.core import setup, run_setup, Extension
from distutils.spawn import spawn, find_executable
from distutils.file_util import write_file
from distutils.text_file import TextFile
import os
#import py2exe

swig_path = find_executable('swig')
#doxygen_path = find_executable('doxygen')

str_version = '0.33'

ext_src = []            # list of needed c++ sources
misc_src = []           # list of needed c++ sources for an other (misc) module
extras_src = []         # list of nneded c++ sources for the extras module

inc_root = '..'
inc_patched = '.'+os.sep+'patched'
inc_generated = '.'+os.sep+'generated'
inc_lti_path = ''
lib_lti_path = ''
swig_base = '.'+os.sep+'swig'+os.sep

base_lti_path = None

WORKAREA_PATH = os.getenv("WORKAREA")

# is pylti not part of the ltilib ? Yes --> than we need a directory structure of WORKAREA_PATH/import/ltilib
if WORKAREA_PATH<>None:
    base_lti_path = WORKAREA_PATH+os.sep+'import'+os.sep+'ltilib'

# Errorhandling if WORKAREAL not found ...

# check if we are in misc directory of the ltilib distribution (ltilib/misc/pylti)
# very simple way: navigate up and navigate down again
if WORKAREA_PATH==None: 
    strCheckPath = '..'+os.sep+'..'+os.sep+'misc'+os.sep+'pylti'+'-'+str_version
    try:
        # try to read lti.i, this is a good indication of a pylti directory
	aTestFile = TextFile(filename=strCheckPath+os.sep+'swig'+os.sep+'lti.i')
        base_lti_path = '..'+os.sep+'..'        # move up pylti and misc directory 
    except IOError :
        pass

# otherwise give an error message
if base_lti_path==None:
    print "Error: environment variable WORKAREA to access ltilib base directory is not set\nor project not installed in ltilib/misc!" 
    import sys
    sys.exit(-1)

# create all the path needed for the build process

# on which platform do we run ?
if os.name=='nt':
    lib_name = 'ltilib7_rmfcd'      # build ltilib with the option: MFC_Shared_Release=1
    inc_lti_path = base_lti_path+os.sep+'lib'+os.sep+'headerFiles'
    lib_lti_path = base_lti_path+os.sep+'lib'
if os.name=='posix':
    # WARNING: this is not tested !!!
    lib_name = 'ltir'
    base_lti_path = '/usr/local/'    # this is the default place of the ltilib installation under linux...
    inc_lti_path = base_lti_path+os.sep+'include'+os.sep+'ltilib'
    lib_lti_path = base_lti_path+os.sep+'lib'+os.sep+'ltilib'

# wenn SWIG gefunden wird, den Wrapper erzeugen, ansonsten den existierenden verwenden
if swig_path<>None:             # TODO Patch, Probleme mit swig return wert !
    ext_src.append(swig_base+'lti.i')
    misc_src.append(swig_base+'ltimisc.i')
    extras_src.append(swig_base+'ltiextras.i')
else:
    ext_src.append(swig_base+'lti_wrap.cpp')
    misc_src.append(swig_base+'ltimisc_wrap.cpp')
    extras_src.append(swig_base+'ltiextras_wrap.cpp')
    
# add the implementation source files for the manually wrapping of functions
ext_src.append('.'+os.sep+'src'+os.sep+'lti_manual_impl.cpp') 

# maybe insert here the creation/update of the generated c++ header files
#TODO: does not work ! run_setup('gen_ltilib_classes.py')

# write the setup.cfg file, needed for a better swig support...
strSetupCfg = ('; WARNING: this file is automatically generated, do not modify !','[build_ext]','--swigopts=-v -c++ -features autodoc=1 -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated -I./patched -I'+inc_lti_path)
# some very interesting options for debugging: -dump_typedef -dump_tree -dump_classes 
# -E only preprocessing no wrapping
write_file('setup.cfg',strSetupCfg)

setup(name="pylti",
      version=str_version,
      author="Pablo Alvarado, Peter Doerfler, et. al.",
      maintainer="Michael Neuroth",
      maintainer_email="michael.neuroth@freenet.de",
      platforms=["nt","posix"],
      description="Python bindings for ltilib (ver. 1.9.12)",
      long_description="Python bindings for ltilib.\n\nLTI-Lib is an object oriented computer vision library written in C++ for\nWindows/MS-VC++ and Linux/gcc.\nIt provides lots of functionality to solve mathematical problems,\nmany image processing algorithms, some classification tools and much more...",
      url="http://ltilib.sourceforge.net/doc/homepage/index.shtml",
      license="LGPL",
      #py_modules=[""],
      #for py2exe test: console=["testpylti.py"],  #windows=["..."]
      ext_modules=[Extension("_lti",
                             ext_src,
                             include_dirs=[inc_root,inc_patched,inc_generated,inc_lti_path],
                             library_dirs=[lib_lti_path],
                             libraries=[lib_name],
                             #swig_opts does not work properly, maybe bug in disutils.build_ext.swig_sources(): swig_opts stehen im Argument extension und nicht in self.swig_opts !!!
                             swigopts=['-c++', '-features','autodoc=1', '-DHAVE_LIBJPEG', '-DHAVE_LIBPNG', '-I./patched', '-I./generated', '-I'+inc_lti_path],
                             depends=['.'+os.sep+'src'+os.sep+'lti_manual.h'],
                             language='c++'
                             ),
                   Extension("_ltimisc",
                             misc_src,
                             include_dirs=[inc_root,inc_patched,inc_generated,inc_lti_path],
                             library_dirs=[lib_lti_path],
                             libraries=[lib_name],
                             language='c++'
                             ),
                   Extension("_ltiextras",
                             extras_src,
                             include_dirs=[inc_root,inc_patched,inc_generated,inc_lti_path],
                             library_dirs=[lib_lti_path],
                             libraries=[lib_name],
                             language='c++'
                             )],
      packages=["","test"],                             
      extra_path="pylti",
      #package_dir={"": "py"},
	  #package_dir={"": "swig"},			# install only generated files !
	  package_dir={"test": "", "": "swig" },			# install generated files and test-scripts !
      #scripts=['scripts/moveproxyfile.py'],
      #data_files=[('',['testpylti.py'])],
      keywords=["images","imageprocessing","Python","ltilib"])

