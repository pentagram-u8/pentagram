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

# Path to an empty file (will be overridden for windows)
EMPTY_FILE=/dev/null
AC_SUBST(EMPTY_FILE)

# Various commands used in our Makefile.in
RM="rm -f"
AC_SUBST(RM)

MKDIR="mkdir -p"
AC_SUBST(MKDIR)

DATAPATH="$datadir/pentagram"

# determine various settings based on 'host_os'

# We define one of the following
#   UNIX  (and maybe LINUX, OPENBSD, ...)
#   BEOS
#   WIN32 (and maybe CYGWIN)
#   MACOSX

# HAVE_HOME is defined if the HOME env.var points to the user's home dir.

AC_MSG_CHECKING(system)
case "$host_os" in
	linux*)
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(LINUX, 1, [Linux])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(UNIX (GNU/Linux))
		;;
	beos*)
		AC_DEFINE(BEOS, 1, [BeOS])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(BeOS)
		LIBS="$LIBS -lbe -lmidi"
		enable_timidity="no"
		;;
	mingw32* )
		AC_DEFINE(WIN32, 1, [Windows])
		AC_MSG_RESULT(Win32 (mingw32))
		LIBS="$LIBS -lwinmm -lstdc++"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	cygwin* )
		AC_DEFINE(WIN32, 1, [Windows])
		AC_DEFINE(CYGWIN, 1, [Cygwin])
		AC_MSG_RESULT(Win32 (cygwin))
		CXXFLAGS="$CXXFLAGS -mno-cygwin"
		LIBS="$LIBS -lwinmm"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	openbsd* )
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_DEFINE(OPENBSD, 1, [OpenBSD])
		AC_MSG_RESULT(UNIX (OpenBSD))
		LIBS="$LIBS -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga"
		;;
	solaris* )
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(UNIX (Solaris))
		LIBS="$LIBS -lsocket -lX11"
		;;
	darwin*)
		dnl We have a problem here: both MacOS X and Darwin report 
		dnl the same signature "powerpc-apple-darwin*" - so we have
		dnl to do more to distinguish them. Plain Darwin will propably
		dnl use X-Windows; and it is of course lacking Cocoa.
		dnl For now I am lazy and do not add proper detection code.
		AC_DEFINE(MACOSX, 1, [Mac OS X])
		AC_MSG_RESULT(Mac OS X)
		LIBS="$LIBS -framework AudioUnit"
		DATAPATH="data"
		;;
	*)
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_MSG_RESULT(not sure... guessing a unix variant)
		;;
esac


# ---------------------------------------------------------------------
# Compilers and other tools
# ---------------------------------------------------------------------
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_CXX


dnl # objcopy (used for embedding binary data into linux binary)
dnl # not currently used
dnl AC_CHECK_PROG(have_objdump, "objdump", yes, no)
dnl AC_CHECK_PROG(have_objcopy, "objcopy", yes, no)
dnl if test x$have_objdump = xyes -a x$have_objcopy = xyes ; then
dnl   AC_MSG_CHECKING(objcopy command line)
dnl    AC_TRY_COMPILE(,,[
dnl 	OBJCOPY_format=`objdump -f conftest.$ac_objext | sed -e '/file format/!d' -e 's/.*format \(.*\)/\1/'`
dnl 	OBJCOPY_arch=`objdump -f conftest.$ac_objext | sed -e '/architecture/!d' -e 's/.*architecture: \(.*\),.*/\1/'`
dnl    ])
dnl    AC_MSG_RESULT(-I binary -O $OBJCOPY_format -B $OBJCOPY_arch)
dnl fi


# ---------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------
# Switch to C++
AC_LANG_CPLUSPLUS

AC_HEADER_STDC

AC_CHECK_HEADERS(sstream)
AC_CHECK_HEADERS(unistd.h sys/types.h sys/stat.h)

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
AC_ARG_ENABLE(mmx, [  --enable-mmx            Use MMX optimisations [default no]],,enable_mmx=no)
if test x$enable_mmx = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -mmmx"
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if we should optimise for 3DNOW)
AC_ARG_ENABLE(3dnow, [  --enable-3dnow          Use 3DNOW optimisations [default no]],,enable_3dnow=no)
if test x$enable_3dnow = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -m3dnow"
else
	AC_MSG_RESULT(no)
fi

# ---------------------------------------------------------------------
# Random Optimisations
# ---------------------------------------------------------------------

AC_MSG_CHECKING(if we should add seemingly random optimisations)
AC_ARG_ENABLE(random_optimisations, [  --enable-random-optimisations   Use random optimisations [default no]],,enable_random_optimisations=no)
if test x$enable_random_optimisations = xyes; then
    AC_MSG_RESULT(yes)
    CXXFLAGS="$CXXFLAGS -O3 -funroll-loops"
	# Either of these 'may' generate better code: -fforce-mem -fforce-addr
	# For branch profiling optimisations: -fbranch-probabilities
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
AC_ARG_ENABLE(paranoid-warnings, [  --enable-paranoid-warnings      Enable paranoid warnings [default no]],,enable_paranoid_warnings=no)
if test x$enable_paranoid_warnings = xyes; then
        WARNINGS="-Wall -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Wdisabled-optimization -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wfloat-equal -Wundef -Wpointer-arith -Wcast-qual -Wcast-align"
		# -Wshadow produces a lot of pointless warnings, don't use it.
		# -Wunreachable-code
#       DEBUG_FLAGS="$DEBUG_FLAGS -Wall -O -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Winline"
        WARNING_LEVEL="Paranoid"
else
        WARNINGS="-Wall"
        WARNING_LEVEL="Normal"
fi

# ------------------
# Profiling!
# ------------------
AC_MSG_CHECKING(if we should include profiling information)
AC_ARG_ENABLE(profiling, [  --enable-profiling      Enable profiling [default no]],,enable_profiling=no)
if test x$enable_profiling = xyes; then
	AC_MSG_RESULT(yes)
	PROFILE="-pg"
	# -fprofile-arcs
else
	AC_MSG_RESULT(no)
fi

# ------------------
# Semi-pointless compiler, compilation flags
# ------------------
AC_MSG_CHECKING(if we want quirky compiler comments)
AC_ARG_ENABLE(compiler_quirks, [  --enable-compiler-quirks        Enable quirky compiler comments [default yes]],,enable_compiler_quirks=yes)
if test x$enable_compiler_quirks = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_CQUIRKS, 1, [Random quirky comments, rather then default normalness])
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if we want usecode compiler regression tester built)
AC_ARG_ENABLE(compiler_tester, [  --enable-compiler-tester        Usecode compiler regression tester [default no]],,enable_compiler_tester=no)
if test x$enable_compiler_tester = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(COMPILER_TEST, 1, [Enable the compiler regression tester])
else
	AC_MSG_RESULT(no)
fi


# ------------------
# Features
# ------------------
USESDLMIXER=no
#AC_CHECK_LIB(SDL_mixer, Mix_QuickLoad_RAW,
#	HAVESDLMIXER=yes,
#	HAVESDLMIXER=no)

AC_ARG_ENABLE(fmopl_midi,      [  --disable-fmopl-midi    Disable FMOPL midi],,enable_fmopl_midi=yes)
#if test x$HAVESDLMIXER = xyes; then
	AC_MSG_CHECKING(if we want to use FMOPL midi)
	if test x$enable_fmopl_midi = xyes; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_FMOPL_MIDI, 1, [Enable FMOPL midi])
#		USESDLMIXER=yes
	else
		AC_MSG_RESULT(no)
	fi
#fi

AC_ARG_ENABLE(timidity_midi,      [  --disable-timidity-midi         Disable timidity midi],,enable_timidity_midi=yes)
AC_ARG_WITH(timidity,             [  --with-timidity=path    path to timidity.cfg (optional)],,)
AC_MSG_CHECKING(if we want to use timidity midi)
if test x$enable_timidity_midi = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_TIMIDITY_MIDI, 1, [Enable timidity midi])

	if test x$with_timidity != x; then
		if test ! -d $with_timidity; then
			with_timidity=`echo "$with_timidity" | sed 's/timidity.cfg//'`
		fi
		AC_DEFINE_UNQUOTED(DEFAULT_TIMIDITY_PATH, "$with_timidity", [Default timidity path])
	fi
#	USESDLMIXER=yes
else
	AC_MSG_RESULT(no)
fi

#if test x$HAVESDLMIXER = xyes -a x$USESDLMIXER = xyes; then
#	SDL_LIBS="$SDL_LIBS -lSDL_mixer"
#fi


# ------------------
# Generate output
# ------------------
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
echo
echo Warning level ............. : $WARNING_LEVEL
echo
echo "Now type 'make' to build Pentagram"
