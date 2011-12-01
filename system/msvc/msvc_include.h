//
// MSVC Inlcude... Kludges and precompiled header for Pentagram
//

#if defined(_MSC_VER) && !defined(UNDER_CE)

#pragma once

// This is 64bit safe
#define EX_TYPE_INT8	__int8
#define EX_TYPE_INT16	__int16
#define EX_TYPE_INT32	__int32
#ifdef _WIN64
#define EX_TYPE_INTPTR  __int64
#else
#define EX_TYPE_INTPTR  __w64 int
#endif

#ifndef VERSION
#define VERSION	"0.1svn"
#endif

// Fisrtly some things that need to be defined
#define PENTAGRAM 
#define BUILTIN_DATA
//#define SIZEOF_SHORT 2
//#define SIZEOF_INT 4
//#undef USE_PRECOMPILED_HEADER
//#define COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM
//#define WIN32_USE_MY_DOCUMENTS
//#define I_AM_COLOURLESS_EXPERIMENTING_WITH_D3D
//#define I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS

#define USE_FMOPL_MIDI
#define USE_TIMIDITY_MIDI
//#define USE_FLUIDSYNTH_MIDI

//#define SOFTRENDSURF_FORCE_HW_DOUBLEBUF

//#define COMPILE_GAMMA_CORRECT_SCALERS
//#define COMPILE_ALL_BILINEAR_SCALERS

//#define USE_HQ2X_SCALER 0
//#define USE_HQ3X_SCALER 0
//#define USE_HQ4X_SCALER 0

#define _USE_MATH_DEFINES

#ifdef _DEBUG
#define DEBUG
#endif

// Don't need everything in the windows headers
#define WIN32_LEAN_AND_MEAN

// Don't let windows define min and max as macros
#define NOMINMAX
template<class T> T &max(T &a, T &b) { return (((a) > (b)) ? (a) : (b)); }
template<class T> T &min(T &a, T &b) { return (((a) < (b)) ? (a) : (b)); }

// We need this defined
#define __STRING(x) #x
#define yyFlexLexer llcFlexLexer

//
// Hacks for MSVC 6
//
#if (_MSC_VER == 1200)

// We don't have it, but MSVC7 does
#define DONT_HAVE_HASH_SET
#define DONT_HAVE_HASH_MAP

// Define size_t, but don't define tm, we'll define that later
#define _TM_DEFINED
#include <wctype.h>
#undef _TM_DEFINED

#define _SIZE_T_DEFINED

// we can't put cstdio into std because MSVC MUST have it in the global namespace
#include <cstdio>

// Now put the headers that should be in std into the std namespace
namespace std {

	#include <wchar.h>
	#include <sys/types.h>
	#include <errno.h>
	#include <ctype.h>
	#include <signal.h>
	#include <cstdlib>
	#include <cstring>
	#include <ctime>
	#include <cmath>
	#include <cstdarg>

	// Kludge to make Exult think that size_t has been put into the
	// std namespace
	typedef ::size_t size_t2;
	#define size_t size_t2

	// These are declared/defined in <cstdio> and the need to be in 
	// the std namespace
	using ::FILE;
	using ::fread;
	using ::va_list;
	using ::fopen;
	using ::fprintf;
	using ::fclose;
	using ::fwrite;
	using ::remove;
	using ::printf;
	
#ifndef PENTAGRAM_NEW
	using ::_snprintf;
#endif

}

// These get put in std when they otherwise should be, or are required by other headers
using std::memcmp;
using std::memcpy;
using std::mbstate_t;
using std::size_t;
using std::time_t;
using std::_fsize_t;
using std::tm;
using std::_dev_t;
using std::_ino_t;
using std::_off_t;
using std::isspace;
using std::strlen;
using std::strtol;
using std::memset;
using std::atoi;

// Nope, stat isn't defined
#ifdef _STAT_DEFINED
#undef _STAT_DEFINED
#endif
#include <sys/stat.h>

//
// Hacks for MSVC.Net
//
#elif (_MSC_VER >= 1300) 

// TODO: Remove this warning perhaps?
#if (_MSC_VER != 1300) && (_MSC_VER != 1310) && (_MSC_VER != 1400) && (_MSC_VER != 1500)
// with line number
#define MYSTRING2(x) #x
#define MYSTRING(x) MYSTRING2(x)

#pragma message( __FILE__ "(" MYSTRING(__LINE__)") : warning : Compiling with untested version of MSVC" )
#undef MYSTRING2
#undef MYSTRING
#endif

#include <cctype>
#define HAVE_SYS_STAT_H
#define inline __forceinline
//#pragma inline_depth(255)

//
// Uh oh, unknown MSVC version
//

#else

#error Unknown Version of MSVC being used. Edit "msvc_include.h" and add your version

#endif

#if !defined(WIN32) && defined(_WIN32)
#define WIN32
#endif

#define snprintf _snprintf
#if (_MSC_VER < 1500) // VS2008 doesn't need this
#define vsnprintf _vsnprintf
#endif

// We've got snprintf
#define HAVE_SNPRINTF

// Some often used headers that could be included in out precompiled header
#include <fstream>
#include <exception>
#include <vector>
#include <iomanip>
#include <set>
#include <map>
#include <cassert>
#include <fcntl.h>
#include <direct.h>
#include <cmath>
#include <ctime>
#include <malloc.h>

#include <windows.h>
#include <windef.h>

#ifdef DEBUG
#define STRICTUNZIP
#define STRICTZIP
#endif

#ifndef _WIN64
using std::exit;
#else
namespace std 
{ 
	using ::exit; 
	using ::memcpy; 
	using ::memset; 
	using ::strcmp; 
	using ::size_t; 
	using ::atoi; 
	using ::atof; 
	using ::strstr; 
	using ::fclose; 
	using ::free; 
	using ::fopen; 
	using ::fprintf; 
	using ::fwrite; 
	using ::log10; 
	using ::malloc; 
	using ::memcmp; 
	using ::pow; 
	using ::rand; 
	using ::tolower; 
	using ::toupper; 
	using ::time; 
	using ::strtoul; 
	using ::strtol; 
	using ::strtod; 
	using ::strncmp; 
	using ::strncpy; 
	using ::strlen; 
	using ::strcpy; 
	using ::strcat; 
	using ::sqrt; 
	using ::sin; 
	using ::printf; 
	using ::calloc; 
	using ::abs;
	using ::isprint;
}
#endif

// Don't want SDL Parachute
#define NO_SDL_PARACHUTE

// This is for stat
#ifndef S_ISDIR
#define S_ISDIR(mode)    __S_ISTYPE((mode), _S_IFDIR)
#endif //S_ISDIR

#ifndef __S_ISTYPE
#define __S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#endif //__S_ISTYPE

#define va_copy(a,b) do { (a)=(b); } while(0)

#endif
