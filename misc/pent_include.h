/*
 *  pent_include.h - Basic Pentagram types, and precompiled header
 *
 *  Copyright (C) 2002, 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef	PENT_INCLUDE_H
#define	PENT_INCLUDE_H

#define PENTAGRAM_NEW

// Include config.h first if we have it
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


// Include msvc_include under MSVC
#ifdef _MSC_VER
#  include "msvc_include.h"
#endif


//
// Common/base types
//
#include "common_types.h"


//
// p_dynamic_cast support
//
#include "p_dynamic_cast.h"


//
// assert
//
#include <cassert>


//
// Strings
//
#include <string>

// Empty string
extern const std::string c_empty_string;


//
// Base Errors
//

#include "Errors.h"


//
// The Console
//

#include "Console.h"


//
// Debugging
//
#ifdef DEBUG
#  define POUT(x)		do { pout << x << std::endl; pout.flush(); } while (0)
#  define PERR(x)		do { perr << x << std::endl; perr.flush(); } while (0)
#else
#  define POUT(x)		do { } while(0)
#  define PERR(x)		do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they 
// will additionally set the old object pointer to 0, thus helping prevent
// double deletes (note that "delete 0" is a no-op).
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


//
// Can't happen.
// For things that really can't happen. Or shouldn't anyway.
//
#define CANT_HAPPEN() do { assert(false); } while(0)

//
// Can't happen return.
// Were we're guaranteed to return before this, but we want to shut the compiler warning up.
//
#define CANT_HAPPEN_RETURN() do { return 0; assert(false); } while(0)

// 
// Can't happen with a message 
//
// Allows a message to be supplied.
// May not work on all compilers or run times as expected
//
#define CANT_HAPPEN_MSG(msg) do { assert(msg && false); } while(0)


//
// Precompiled Header Support
//
#ifdef USE_PRECOMPILED_HEADER

// C Standard Library and STL
#include <fstream>
#include <cstdio>
#include <list>
#include <vector>

// Useful Pentagram header
//#include "Kernel.h"
//#include "Object.h"
//#include "Item.h"
//#include "FileSystem.h"
//#include "IDataSource.h"
//#include "Flex.h"
//#include "ShapeFlex.h"
//#include "RenderSurface.h"
//#include "Shape.h"
#endif

#endif //PENT_INCLUDE_H
