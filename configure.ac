dnl Process this file with autoconf to produce a configure script.
AC_INIT(README)
AM_CONFIG_HEADER(misc/config.h)

# ---------------------------------------------------------------------
# System/version info
# ---------------------------------------------------------------------

PACKAGE=pentagram
VER_MAJOR=0
VER_MINOR=1
VER_EXTRA=cvs
VERSION=$VER_MAJOR.$VER_MINOR$VER_EXTRA

# check host/target systems
# (build  = system we're building on,
#  host   = system we're building for,
#  target = system the program we're building will build for)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Package Name])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version Number])
AC_DEFINE_UNQUOTED(VER_MAJOR, "$VER_MAJOR", [Version Number Major])
AC_DEFINE_UNQUOTED(VER_MINOR, "$VER_MINOR", [Verion Number Minor])
AC_DEFINE_UNQUOTED(VER_EXTRA, "$VER_EXTRA", [Version Number Extra])


# ---------------------------------------------------------------------
# Host system settings
# ---------------------------------------------------------------------
AC_EXEEXT

AC_DEFINE(HAVE_MIDI, 1, [Do we have midi support?])

# Path to an empty file (will be overriden for windows)
EMPTY_FILE=/dev/null
AC_SUBST(EMPTY_FILE)

# Various commands used in our Makefile.in
RM="rm -f"
AC_SUBST(RM)

MKDIR="mkdir -p"
AC_SUBST(MKDIR)

DATAPATH="$datadir/pentagram"

# TODO: Fix this XWIN/UNIX/Linux mess.

# determine windowing system based on 'host_os'
AC_MSG_CHECKING("windowing system")
case "$host_os" in
	linux*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(X11 (GNU/Linux))
		;;
	beos)
		WINDOWING_SYSTEM="-DBEOS"
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(BeOS)
		LIBS="$LIBS -lbe -lmidi"
		enable_timidity="no"
		;;
	mingw32* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_MSG_RESULT(Win32 (mingw32))
		LIBS="$LIBS -lwinmm -lstdc++"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	cygwin* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_DEFINE(CYGWIN, 1, [Compiling using Cygwin])
		AC_MSG_RESULT(Win32 (cygwin))
		LIBS="$LIBS -lwinmm"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	openbsd* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_DEFINE(OPENBSD, 1, [Compiling on OpenBSD])
		AC_MSG_RESULT(X11 (OpenBSD))
		LIBS="$LIBS -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga"
		;;
	solaris* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(X11 (Solaris))
		LIBS="$LIBS -lsocket -lX11"
		;;
	darwin*)
		dnl We have a problem here: both MacOS X and Darwin report 
		dnl the same signature "powerpc-apple-darwin*" - so we have
		dnl to do more to distinguish them. Plain Darwin will propably
		dnl use X-Windows; and it is of course lacking Cocoa.
		dnl For now I am lazy and do not add proper detection code.
		
		WINDOWING_SYSTEM="-DMACOSX"
		AC_DEFINE(MACOSX, 1, [Compiling on Mac OS X])
		AC_MSG_RESULT(Mac OS X)
		LIBS="$LIBS -framework QuickTime"
		ARCH=macosx
		DATAPATH="data"
		;;
	*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT(not sure... guessing X11)
		;;
esac


# ---------------------------------------------------------------------
# Compilers and other tools
# ---------------------------------------------------------------------
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_YACC
AC_PROG_CXX

# ---------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------
# Switch to C++
AC_LANG_CPLUSPLUS

AC_HEADER_STDC

#AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(sstream)

# ---------------------------------------------------------------------
# Checks for readline.
# ---------------------------------------------------------------------
#AC_CHECK_HEADERS(readline.h history.h readline/readline.h readline/history.h)
#AC_CHECK_LIB(curses, clear)
#AC_CHECK_LIB(history, add_history)
#AC_CHECK_LIB(readline, readline)

# ---------------------------------------------------------------------
# Checks for specific functions.
# ---------------------------------------------------------------------
AC_MSG_CHECKING(for snprintf())
AC_TRY_LINK([
#include <stdio.h>
],
[
        snprintf(0,0,0,0);
],
  ac_cv_func_snprintf=yes,
  ac_cv_func_snprintf=no)
AC_MSG_RESULT($ac_cv_func_snprintf)
if test x$ac_cv_func_snprintf = xyes ; then
	AC_DEFINE(HAVE_SNPRINTF, 1, [Some platforms don't have snprintf])
fi


# ---------------------------------------------------------------------
# Check sizes of integer types.
# Cross-compiling sizes set to 2,4,4,8. (May need to be changed...)
# ---------------------------------------------------------------------
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)

# ---------------------------------------------------------------------
# Architecture dependent optimisations
# ---------------------------------------------------------------------

AC_MSG_CHECKING(if we should optimise for MMX)
AC_ARG_ENABLE(timidity, [  --enable-mmx       Use MMX optimisations [defult no]],,enable_mmx=no)
if test x$enable_mmx = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -mmmx"
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if we should optimise for 3DNOW)
AC_ARG_ENABLE(timidity, [  --enable-3dnow       Use 3DNOW optimisations [defult no]],,enable_3dnow=no)
if test x$enable_3dnow = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -m3dnow"
else
	AC_MSG_RESULT(no)
fi

# ---------------------------------------------------------------------
# SDL
# ---------------------------------------------------------------------
# FIXME: For some reason this dies when trying to test a compile with
#	the language set to CPLUSPLUS. Find out why and hopefully fix
#	it. *grin*
AC_LANG_C
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
	:,
	AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
AC_LANG_CPLUSPLUS

# ------------------
# Warning level
# ------------------
AC_ARG_ENABLE(paranoid-warnings, [  --enable-paranoid-warnings       Enable paranoid warnings [default no]],,enable_paranoid_warnings=no)
if test x$enable_paranoid_warnings = xyes; then
        WARNINGS="-Wall -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Winline -Wdisabled-optimization -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual"
#       DEBUG_FLAGS="$DEBUG_FLAGS -Wall -O -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Winline"
        WARNING_LEVEL="Paranoid"
	echo pwarn
else
        WARNINGS="-Wall"
        WARNING_LEVEL="Normal"
	echo nwarn
fi

# ------------------
# Profiling!
# ------------------
AC_ARG_ENABLE(profiling, [  --enable-profiling       Enable Profiling [default no]],,enable_profiling=no)
if test x$enable_profiling = xyes; then
	PROFILE="-pg -fprofile-arcs"
fi

# ------------------
# Generate output
# ------------------
AC_SUBST(WINDOWING_SYSTEM)
AC_SUBST(WARNINGS)
AC_SUBST(PACKAGE)
AC_SUBST(PROFILE)
AC_SUBST(DATAPATH)

AC_OUTPUT([
Makefile
])


# ------------------
# Configure Status
# ------------------
echo
echo Pentagram v$VERSION
echo
echo SDL ....................... :  `$SDL_CONFIG --version`
echo Warning level ............. : $WARNING_LEVEL

echo
echo "Now type 'make' to build Pentagram"
