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
 * file .......: ltiConfig.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.03.00
 * revisions ..: $Id$
 */

/** \file ltiConfig.h Load configuration file.
*/

#ifndef _LTI_CONFIG_H_
#define _LTI_CONFIG_H_

#include "ltiMacroSymbols.h"

#ifdef _LTI_MSC_VER
#  include "ltiWinConfig.h"
#else
#  include "ltiLinConfig.h"
#endif

// Symbols defined to cope with some underscored symbols

#if defined (HAVE__ISNAN) && ! defined (HAVE_ISNAN)
# define HAVE_ISNAN 1
# define isnan _isnan
#endif

#if defined (HAVE__ISINF) && ! defined (HAVE_ISINF)
# define HAVE_ISINF 1
# define isinf _isinf
#endif

#if defined (HAVE__FINITE) && ! defined (HAVE_FINITE)
# define HAVE_FINITE 1
# define finite _finite
#endif

// End of underscored symbols

#endif

