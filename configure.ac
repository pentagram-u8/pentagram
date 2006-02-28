dnl Process this file with autoconf to produce a configure script.
AC_INIT()
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS(misc/config.h)
AC_CONFIG_AUX_DIR(system/auto)

# ---------------------------------------------------------------------
# System/version info
# ---------------------------------------------------------------------

PACKAGE=pentagram
VER_MAJOR=0
VER_MINOR=1
VER_EXTRA=cvs
VERSION=$VER_MAJOR.$VER_MINOR$VER_EXTRA

# check build/host/target systems
# (build  = system we're building on,
#  host   = system we're building for,
#  target = system the program we're building will build for)
AC_CANONICAL_BUILD
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
#   WIN32 (and maybe CYGWIN)
#   MACOSX

# HAVE_HOME is defined if the HOME env.var points to the user's home dir.

# We define three sets of libraries/ldflags
#   SYS_LIBS is used in all built applications
#   GUI_LIBS is only for full GUI applications, including SDL, audio
#   CON_LIBS is only for console applications

AC_MSG_CHECKING(system)
case "$host_os" in
	linux*)
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(LINUX, 1, [Linux])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(UNIX (GNU/Linux))
		;;
	mingw32* )
		AC_DEFINE(WIN32, 1, [Windows])
		AC_MSG_RESULT(Win32 (mingw32))
		# note that -mwindows is added by SDL_LIBS
		GUI_LIBS="$GUI_LIBS -lwinmm"
		CON_LIBS="$CON_LIBS -mconsole"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	cygwin* )
		AC_DEFINE(WIN32, 1, [Windows])
		AC_DEFINE(CYGWIN, 1, [Cygwin])
		AC_MSG_RESULT(Win32 (cygwin))
		CXXFLAGS="$CXXFLAGS -mno-cygwin"
		GUI_LIBS="$GUI_LIBS -lwinmm"
		enable_timidity="no"
		EMPTY_FILE=nul
		;;
	openbsd* )
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_DEFINE(OPENBSD, 1, [OpenBSD])
		AC_MSG_RESULT(UNIX (OpenBSD))
		GUI_LIBS="$GUI_LIBS -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga"
		;;
	solaris* )
		AC_DEFINE(UNIX, 1, [UNIX-variant])
		AC_DEFINE(HAVE_HOME, 1, [HOME env.var exists])
		AC_MSG_RESULT(UNIX (Solaris))
		GUI_LIBS="$GUI_LIBS -lX11"
		;;
	darwin*)
		dnl We have a problem here: both MacOS X and Darwin report 
		dnl the same signature "powerpc-apple-darwin*" - so we have
		dnl to do more to distinguish them. Plain Darwin will propably
		dnl use X-Windows; and it is of course lacking Cocoa.
		dnl For now I am lazy and do not add proper detection code.
		AC_DEFINE(MACOSX, 1, [Mac OS X])
		AC_MSG_RESULT(Mac OS X)
		GUI_LIBS="$GUI_LIBS -framework AudioUnit -framework CoreMIDI"
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
SAVED_CC=$CC

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_CXX

# set CC_FOR_BUILD and BUILDEXEEXT (for compiling tools on the build system)
PENT_BUILDCC

AC_MSG_CHECKING(if we should use GCC PCH)
AC_ARG_ENABLE(gcc-pch, [[  --enable-gcc-pch        Use GCC 3.4 precompiled headers [default no]]],,enable_gcc_pch=no)
if test x$GCC = xyes -a x$enable_gcc_pch = xyes; then
	AC_DEFINE(USE_PRECOMPILED_HEADER, 1, [use precompiled header])
	AC_MSG_RESULT(yes)
	USE_PCH=yes
else
	AC_MSG_RESULT(no)
	USE_PCH=no
fi


# ---------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------
# Switch to C++
AC_LANG([C++])

AC_CHECK_HEADERS(unistd.h sys/types.h sys/stat.h)

# ---------------------------------------------------------------------
# Checks for specific functions.
# ---------------------------------------------------------------------
AC_MSG_CHECKING(for snprintf())
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
]], [[
        snprintf(0,0,0,0);
]])],[have_snprintf=yes],[have_snprintf=no])
AC_MSG_RESULT($have_snprintf)
if test x$have_snprintf = xyes ; then
	AC_DEFINE(HAVE_SNPRINTF, 1, [Some platforms don't have snprintf])
fi


# ---------------------------------------------------------------------
# Check sizes of integer types.
# ---------------------------------------------------------------------
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(int*)

# ---------------------------------------------------------------------
# Architecture dependent optimisations
# ---------------------------------------------------------------------

AC_MSG_CHECKING(if we should optimise for MMX)
AC_ARG_ENABLE(mmx, [[  --enable-mmx            Use MMX optimisations [default no]]],,enable_mmx=no)
if test x$enable_mmx = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -mmmx"
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if we should optimise for 3DNOW)
AC_ARG_ENABLE(3dnow, [[  --enable-3dnow          Use 3DNOW optimisations [default no]]],,enable_3dnow=no)
if test x$enable_3dnow = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -m3dnow"
else
	AC_MSG_RESULT(no)
fi

# ---------------------------------------------------------------------
# Random Optimisations
# ---------------------------------------------------------------------

AC_MSG_CHECKING(if we should add some extra optimisation options)
AC_ARG_ENABLE(random_optimisations, [[  --enable-extra-optimisations    Enable some extra optimisations [default no]]],,enable_random_optimisations=no)
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
AM_PATH_SDL(1.2.0,:,AC_MSG_ERROR([[*** SDL version $SDL_VERSION not found!]])
)

GUI_LIBS="$GUI_LIBS $SDL_LIBS"

# -----
# zlib
# -----

AC_CHECK_HEADER(zlib.h,,AC_MSG_ERROR([*** zlib not found!]))
AC_CHECK_LIB(z,deflate,,AC_MSG_ERROR([*** zlib not found!]))
SYS_LIBS="$SYS_LIBS -lz"

# --------
# SDL_ttf
# --------

PENT_CHECK_SDLTTF(:,AC_MSG_ERROR([*** SDL_ttf not found!]))
GUI_LIBS="$GUI_LIBS $SDLTTF_LIBS"

# --------
# libpng
# --------

AC_CHECK_HEADER(png.h,,AC_MSG_ERROR([*** libpng not found!]))
AC_CHECK_LIB(png,png_create_read_struct,,AC_MSG_ERROR([*** libpng not found!]))
GUI_LIBS="$GUI_LIBS -lpng"

# ------------------
# Warning level
# ------------------
AC_ARG_ENABLE(paranoid-warnings, [[  --enable-paranoid-warnings      Enable paranoid warnings [default no]]],,enable_paranoid_warnings=no)
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
AC_ARG_ENABLE(profiling, [[  --enable-profiling      Enable profiling [default no]]],,enable_profiling=no)
if test x$enable_profiling = xyes; then
	AC_MSG_RESULT(yes)
	PROFILE="-pg"
	# -fprofile-arcs
else
	AC_MSG_RESULT(no)
fi

# ------------------
# Semi-pointless usecode compiler, compilation flags
# ------------------
#AC_MSG_CHECKING(if we want quirky usecode compiler comments)
#AC_ARG_ENABLE(compiler_quirks, [[  --enable-compiler-quirks        Enable quirky compiler comments [default yes]]],,enable_compiler_quirks=yes)
#if test x$enable_compiler_quirks = xyes; then
#	AC_MSG_RESULT(yes)
#	AC_DEFINE(USE_CQUIRKS, 1, [Random quirky comments, rather then default normalness])
#else
#	AC_MSG_RESULT(no)
#fi

#AC_MSG_CHECKING(if we want usecode compiler regression tester built)
#AC_ARG_ENABLE(compiler_tester, [[  --enable-compiler-tester        Usecode compiler regression tester [default no]]],,enable_compiler_tester=no)
#if test x$enable_compiler_tester = xyes; then
#	AC_MSG_RESULT(yes)
#	AC_DEFINE(COMPILER_TEST, 1, [Enable the compiler regression tester])
#else
#	AC_MSG_RESULT(no)
#fi


# ------------------
# Features
# ------------------

# debugging
AC_ARG_ENABLE(debug, [[  --disable-debug         Disable debugging mode]],,enable_debug=yes)
AC_MSG_CHECKING(for debug mode)
if test x$enable_debug = xyes; then
	AC_DEFINE(DEBUG, 1, [Debugging mode])
	pent_debug=yes
else
	pent_debug=no
fi
AC_MSG_RESULT($pent_debug)


# extra valgrind memory debugging code
AC_ARG_ENABLE(valgrind, [[  --enable-valgrind       Enable extra valgrind support]],,enable_valgrind=no)
AC_CHECK_HEADER(valgrind/memcheck.h, HAVEVALGRIND=yes, HAVEVALGRIND=no)
AC_MSG_CHECKING(for valgrind support)
pent_valgrind=no
if test x$enable_valgrind = xyes; then
	if test x$HAVEVALGRIND = xyes; then
		AC_MSG_RESULT(yes)
		pent_valgrind=yes
		AC_DEFINE(USE_VALGRIND, 1, [Enable Valgrind support])
	else
		AC_MSG_RESULT(no; valgrind header not found)
	fi
else
	AC_MSG_RESULT(no)
fi


# fmopl midi
AC_ARG_ENABLE(fmopl_midi, [[  --disable-fmopl-midi    Disable built-in FMOPL midi]],,enable_fmopl_midi=yes)
AC_MSG_CHECKING(if we want to use FMOPL midi)
if test x$enable_fmopl_midi = xyes; then
	AC_DEFINE(USE_FMOPL_MIDI, 1, [Enable FMOPL midi])
	pent_fmopl=yes
else
	pent_fmopl=no
fi
AC_MSG_RESULT($pent_fmopl)


# timidity midi
AC_ARG_ENABLE(timidity_midi, [[  --disable-timidity-midi Disable built-in timidity midi]],,enable_timidity_midi=yes)
AC_ARG_WITH(timidity, [[  --with-timidity=path    path to timidity.cfg (optional)]],,)
AC_MSG_CHECKING(if we want to use timidity midi)
if test x$enable_timidity_midi = xyes; then
	AC_DEFINE(USE_TIMIDITY_MIDI, 1, [Enable timidity midi])
	pent_timidity=yes

	if test x$with_timidity != x; then
		if test ! -d $with_timidity; then
			with_timidity=`echo "$with_timidity" | sed 's/timidity.cfg//'`
		fi
		AC_DEFINE_UNQUOTED(DEFAULT_TIMIDITY_PATH, "$with_timidity", [Default timidity path])
	fi
else
	pent_timidity=no
fi
AC_MSG_RESULT($pent_timidity)

# alsa midi
AC_CHECK_HEADER(alsa/asoundlib.h, HAVEALSA=yes, HAVEALSA=no)
AC_ARG_ENABLE(alsa, [[  --disable-alsa          Disable ALSA midi support]],,enable_alsa=yes)
AC_MSG_CHECKING(if we want to use ALSA midi)
pent_alsa=no
if test x$HAVEALSA = xyes; then
	if test x$enable_alsa = xyes; then
		AC_MSG_RESULT(yes)
		pent_alsa=yes
		AC_DEFINE(USE_ALSA_MIDI, 1, [Enable ALSA midi])
		GUI_LIBS="$GUI_LIBS -lasound"
	else
		AC_MSG_RESULT(no)
	fi
else
	AC_MSG_RESULT([no; libasound not found])
fi

# fluidsynth midi
AC_CHECK_HEADER(fluidsynth.h, HAVEFLUIDSYNTH=yes, HAVEFLUIDSYNTH=no)
AC_ARG_ENABLE(fluidsynth, [[  --disable-fluidsynth    Disable fluidsynth midi support]],,enable_fluidsynth=yes)
AC_MSG_CHECKING(if we want to use fluidsynth midi)
pent_fluidsynth=no
if test x$HAVEFLUIDSYNTH = xyes; then
        if test x$enable_fluidsynth = xyes; then
		pent_fluidsynth=yes
                AC_MSG_RESULT(yes)
                AC_DEFINE(USE_FLUIDSYNTH_MIDI, 1, [Enable fluidsynth midi])
                GUI_LIBS="$GUI_LIBS -lfluidsynth"
        else
                AC_MSG_RESULT(no)
        fi
else
        AC_MSG_RESULT([no; fluidsynth.h not found])
fi


# HQ2X scaler
AC_ARG_ENABLE(hq2x, [[  --enable-hq2x           Enable hq2x scaler support [default no]]],,enable_hq2x=no)
AC_MSG_CHECKING(if we should build the hq2x scaler)
if test x$enable_hq2x = xyes; then
	pent_hq2x=yes
	AC_DEFINE(USE_HQ2X_SCALER, 1, [Build hq2x scaler])
else
	pent_hq2x=no
fi
AC_MSG_RESULT($pent_hq2x)


# gamma-correcting scalers
AC_ARG_ENABLE(gc_scalers, [[  --enable-gc-scalers     Enable gamma-correcting scalers [default no]]],,enable_gc_scalers=no)
AC_MSG_CHECKING(if we should build the gamma-correcting scalers)
if test x$enable_gc_scalers = xyes; then
	pent_gc_scalers=yes
	AC_DEFINE(COMPILE_GAMMA_CORRECT_SCALERS, 1, [Build gamma-correcting scalers])
else
	pent_gc_scalers=no
fi
AC_MSG_RESULT($pent_gc_scalers)

# all bilinear scaler variants
AC_ARG_ENABLE(all_bilinear, [[  --enable-all-bilinear   Enable all bilinear scaler variants [default no]]],,enable_all_bilinear=no)
AC_MSG_CHECKING(if we should build all bilinear scaler variants)
if test x$enable_all_bilinear = xyes; then
	pent_all_bilinear=yes
	AC_DEFINE(COMPILE_ALL_BILINEAR_SCALERS, 1, [Build all bilinear scaler variants])
else
	pent_all_bilinear=no
fi
AC_MSG_RESULT($pent_all_bilinear)


# Built-in data files
AC_ARG_ENABLE(builtin_data, [[  --disable-builtin-data  Disable built-in data files]],,enable_builtin_data=yes)
AC_MSG_CHECKING(if we should use built-in data files)
if test x$enable_builtin_data = xyes; then
	AC_DEFINE(BUILTIN_DATA, 1, [Use built-in data files])
	USE_BUILTIN_DATA=yes
else
	USE_BUILTIN_DATA=no
fi
pent_nodata=$USE_BUILTIN_DATA
AC_MSG_RESULT($pent_nodata)


# -------------------
# GIMP plugin
# -------------------
AM_CONDITIONAL(GIMP_PLUGIN, false)
AC_ARG_ENABLE(gimp-plugin, [[  --enable-gimp-plugin    Build the GIMP plugin [default no]]],,enable_gimp_plugin=no)
AC_MSG_CHECKING(whether to build the GIMP plugin)
pent_gimp=no
if test x$enable_gimp_plugin = xyes; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING(for gimptool)
	AC_CHECK_PROGS(GIMPTOOL, gimptool-2.2 gimptool-2.0 gimptool)
	if test -z "$GIMPTOOL"; then
		AC_MSG_RESULT(no, not building GIMP plugin)
	else
		AC_MSG_CHECKING(for GIMP version)
		gimp_version=`$GIMPTOOL --version |  awk 'BEGIN { FS = "."; } { print $1 * 1000 + $2*100+$3;}'`
		if test "$gimp_version" -ge 2200; then
			AC_MSG_RESULT(found >= 2.2.0)
			AC_SUBST(GIMPTOOL)
			AM_CONDITIONAL(GIMP_PLUGIN, true)
			GIMP_PLUGIN_PREFIX=`$GIMPTOOL --gimpplugindir`
			GIMP_PLUGIN_PREFIX="$GIMP_PLUGIN_PREFIX/plug-ins"
			AC_SUBST(GIMP_PLUGIN_PREFIX)
			GIMP_INCLUDES=`$GIMPTOOL --cflags`
			GIMP_LIBS=`$GIMPTOOL --libs`
			AC_SUBST(GIMP_INCLUDES)
			AC_SUBST(GIMP_LIBS)
			pent_gimp=yes
		else
			AC_MSG_RESULT(found < 2.2.0 - disabling plugin)
		fi
	fi
else
	AC_MSG_RESULT(no)
fi

# ------------------
# Generate output
# ------------------
AC_SUBST(CC_FOR_BUILD)
AC_SUBST(BUILDEXEEXT)
AC_SUBST(WARNINGS)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(PROFILE)
AC_SUBST(DATAPATH)
AC_SUBST(USE_PCH)
AC_SUBST(USE_BUILTIN_DATA)
AC_SUBST(SYS_LIBS)
AC_SUBST(GUI_LIBS)
AC_SUBST(CON_LIBS)

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT


# ------------------
# Configure Status
# ------------------
echo ""
echo "Pentagram v$VERSION"
echo ""
echo "Configure results:"
echo ""
echo "Built-in features:"
echo "Debugging:			$pent_debug"
echo "Extra valgrind support:		$pent_valgrind"
echo "FMOPL midi:			$pent_fmopl"
echo "Timidity midi:			$pent_timidity"
echo "HQ2x scaler:			$pent_hq2x"
echo "Gamma-correcting scalers:	$pent_gc_scalers"
echo "All bilinear scaler variants:	$pent_all_bilinear"
echo "Built-in data files:		$pent_nodata"
echo ""
echo "Optional libraries:"
echo "ALSA midi:			$pent_alsa"
echo "FluidSynth midi:		$pent_fluidsynth"
echo ""
echo "External tools:"
echo "Experimental GIMP plugin:	$pent_gimp"
echo "Miscellaneous tools:		yes"
echo ""
echo "Now type 'make' to build Pentagram"
