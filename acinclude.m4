dnl When config.status generates a header, we must update config.h.stamp.
dnl This file resides in the same directory as the config header
dnl  that is generated.

dnl Autoconf calls _AC_AM_CONFIG_HEADER_HOOK (when defined) in the
dnl loop where config.status creates the headers, so we can generate
dnl our stamp files there.
AC_DEFUN([_AC_AM_CONFIG_HEADER_HOOK],
[[echo "# timestamp for $1" >$1.stamp]])




dnl Set CC_FOR_BUILD to a native compiler. This is used to compile
dnl tools needed for the Pentagram build process on the build machine.
dnl (adapted from config.guess, (C) 1992-2005 Free Software Foundation, Inc.)
AC_DEFUN([PENT_BUILDCC],[
if test x$cross_compiling = xyes; then
  AC_MSG_CHECKING(for a native C compiler)
  if test x${CC_FOR_BUILD+set} != xset ; then
    echo "int x;" > conftest.$ac_ext
    for c in cc gcc c89 c99 ; do
      if ($c -c -o conftest.o conftest.$ac_ext) >/dev/null 2>&1 ; then
        CC_FOR_BUILD="$c"; break
      fi
    done
    if test x"$CC_FOR_BUILD" = x ; then
      CC_FOR_BUILD=no_compiler_found
    fi
  fi
  if test "x$CC_FOR_BUILD" = xno_compiler_found ; then
    AC_MSG_RESULT(none found)
  else
    AC_MSG_RESULT($CC_FOR_BUILD)
  fi

  # Very basic check for EXEEXT_FOR_BUILD
  BUILDEXEEXT=
  case "$build_os" in
  mingw32* ) BUILDEXEEXT=.exe ;;
  cygwin* ) BUILDEXEEXT=.exe ;;
  esac
else
  CC_FOR_BUILD=$CC
  BUILDEXEEXT=$EXEEXT
fi
]);



dnl Check if we have SDL_ttf (header and library) version >= 2.0.7
dnl Input:
dnl SDL_CFLAGS, SDL_LIBS are set correctly
dnl Output:
dnl SDLTTF_LIBS contains the necessary libs/ldflags
dnl HAVE_SDL_TTF_H is AC_DEFINE-d if including "SDL_ttf.h" works
dnl HAVE_SDL_SDL_TTF_H is AC_DEFINE-d if including <SDL/SDL_ttf.h> works

AC_DEFUN([PENT_CHECK_SDLTTF],[
  REQ_MAJOR=2
  REQ_MINOR=0
  REQ_PATCHLEVEL=7
  REQ_VERSION=$REQ_MAJOR.$REQ_MINOR.$REQ_PATCHLEVEL

  AC_MSG_CHECKING([for SDL_ttf - version >= $REQ_VERSION])

  pent_backupcppflags="$CPPFLAGS"
  pent_backupldflags="$LDFLAGS"
  pent_backuplibs="$LIBS"

  CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
  LDFLAGS="$LDFLAGS $SDL_LIBS"

  pent_sdlttfok=yes

  dnl First: include "SDL_ttf.h"

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  #include "SDL_ttf.h"
  ]],)],sdlttfh_found=yes,sdlttfh_found=no)

  if test x$sdlttfh_found = xno; then

    dnl If failed: include <SDL/SDL_ttf.h>

    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <SDL/SDL_ttf.h>
    ]],)],sdlsdlttfh_found=yes,sdlsdlttfh_found=no)
    if test x$sdlsdlttfh_found = xno; then
      pent_sdlttfok=no
    else
      AC_DEFINE(HAVE_SDL_SDL_TTF_H, 1, [Define to 1 if you have the <SDL/SDL_ttf.h> header file but not "SDL_ttf.h"])
    fi
  else
    AC_DEFINE(HAVE_SDL_TTF_H, 1, [Define to 1 if you have the "SDL_ttf.h" header file])
  fi

  if test "x$pent_sdlttfok" = xyes; then
    dnl Version check (cross-compile-friendly idea borrowed from autoconf)

    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"
    #ifdef HAVE_SDL_TTF_H
    #include "SDL_ttf.h"
    #else
    #include <SDL/SDL_ttf.h>
    #endif

    int main()
    {
      static int test_array[1-2*!(SDL_TTF_MAJOR_VERSION>$REQ_MAJOR||(SDL_TTF_MAJOR_VERSION==$REQ_MAJOR&&(SDL_TTF_MINOR_VERSION>$REQ_MINOR||(SDL_TTF_MINOR_VERSION==$REQ_MINOR&&SDL_TTF_PATCHLEVEL>=$REQ_PATCHLEVEL))))];
      test_array[0] = 0;
      return 0;
    }
    ]])],,[[pent_sdlttfok="no, version < $REQ_VERSION found"]])
  fi

  dnl Next: try linking

  if test "x$pent_sdlttfok" = xyes; then

    LIBS="$LIBS -lSDL_ttf"

    AC_LINK_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"
    #ifdef HAVE_SDL_TTF_H
    #include "SDL_ttf.h"
    #else
    #include <SDL/SDL_ttf.h>
    #endif

    int main(int argc, char* argv[]) {
      TTF_Init();
      return 0;
    }
    ]])],sdlttflinkok=yes,sdlttflinkok=no)
    if test x$sdlttflinkok = xno; then

      dnl If failed: try -lSDL_ttf -lfreetype
      dnl Note: This is mainly for my personal convenience. It should not
      dnl       be necessary usually, but shouldn't hurt either -wjp, 20050727
      dnl       (The reason is that I often statically link against SDL_ttf)

      LIBS="$LIBS -lfreetype"
      AC_LINK_IFELSE([AC_LANG_SOURCE([[
      #include "SDL.h"
      #ifdef HAVE_SDL_TTF_H
      #include "SDL_ttf.h"
      #else
      #include <SDL/SDL_ttf.h>
      #endif

      int main(int argc, char* argv[]) {
        TTF_Init();
        return 0;
      }
      ]])],sdlttflinkok=yes,sdlttflinkok=no)

      if test x$sdlttflinkok = xno; then
        pent_sdlttfok=no
      else
        SDLTTF_LIBS="-lSDL_ttf -lfreetype"
      fi
    else  
      SDLTTF_LIBS="-lSDL_ttf"
    fi
  fi

  AC_MSG_RESULT($pent_sdlttfok)

  LDFLAGS="$pent_backupldflags"
  CPPFLAGS="$pent_backupcppflags"
  LIBS="$pent_backuplibs"

  if test "x$pent_sdlttfok" = xyes; then
    ifelse([$1], , :, [$1])
  else
    ifelse([$2], , :, [$2])
  fi
]);





dnl Check if we have SDL (sdl-config, header and library) version >= 1.2.0
dnl Output:
dnl SDL_CFLAGS and SDL_LIBS are set and AC_SUBST-ed
dnl HAVE_SDL_H is AC_DEFINE-d

AC_DEFUN([PENT_CHECK_SDL],[
  REQ_MAJOR=1
  REQ_MINOR=2
  REQ_PATCHLEVEL=0
  REQ_VERSION=$REQ_MAJOR.$REQ_MINOR.$REQ_PATCHLEVEL

  pent_backupcppflags="$CPPFLAGS"
  pent_backupldflags="$LDFLAGS"
  pent_backuplibs="$LIBS"

  pent_sdlok=yes

  AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)], sdl_prefix="$withval", sdl_prefix="")
  AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)], sdl_exec_prefix="$withval", sdl_exec_prefix="")

  dnl First: find sdl-config

  sdl_args=""
  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_exec_prefix/bin/sdl-config
     fi
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_prefix/bin/sdl-config
     fi
  fi

  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
  if test "$SDL_CONFIG" = "no" ; then
    pent_sdlok=no
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_patchlevel=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi

  AC_MSG_CHECKING([for SDL - version >= $REQ_VERSION])


  dnl Second: include "SDL.h"

  if test x$pent_sdlok = xyes ; then
    CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include "SDL.h"
    ]],)],sdlh_found=yes,sdlh_found=no)

    if test x$sdlh_found = xno; then
      pent_sdlok=no
    else
      AC_DEFINE(HAVE_SDL_H, 1, [Define to 1 if you have the "SDL.h" header file])
    fi
  fi

  dnl Next: version check (cross-compile-friendly idea borrowed from autoconf)
  dnl (First check version reported by sdl-config, then confirm
  dnl  the version in SDL.h matches it)

  if test x$pent_sdlok = xyes ; then

    if test ! \( \( $sdl_major_version -gt $REQ_MAJOR \) -o \( \( $sdl_major_version -eq $REQ_MAJOR \) -a \( \( $sdl_minor_version -gt $REQ_MINOR \) -o \( \( $sdl_minor_version -eq $REQ_MINOR \) -a \( $sdl_patchlevel -gt $REQ_PATCHLEVEL \) \) \) \) \); then
      pent_sdlok="no, version < $REQ_VERSION found"
    else
      AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
      #include "SDL.h"

      int main()
      {
        static int test_array[1-2*!(SDL_MAJOR_VERSION==$sdl_major_version&&SDL_MINOR_VERSION==$sdl_minor_version&&SDL_PATCHLEVEL==$sdl_patchlevel)];
        test_array[0] = 0;
        return 0;
      }
      ]])],,[[pent_sdlok="no, version of SDL.h doesn't match that of sdl-config"]])
    fi
  fi

  dnl Next: try linking

  if test "x$pent_sdlok" = xyes; then
    LIBS="$LIBS $SDL_LIBS"

    AC_LINK_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"

    int main(int argc, char* argv[]) {
      SDL_Init(0);
      return 0;
    }
    ]])],sdllinkok=yes,sdllinkok=no)
    if test x$sdllinkok = xno; then
      pent_sdlok=no
    fi
  fi

  AC_MSG_RESULT($pent_sdlok)

  LDFLAGS="$pent_backupldflags"
  CPPFLAGS="$pent_backupcppflags"
  LIBS="$pent_backuplibs"

  if test "x$pent_sdlok" = xyes; then
    AC_SUBST(SDL_CFLAGS)
    AC_SUBST(SDL_LIBS)
    ifelse([$1], , :, [$1])
  else
    ifelse([$2], , :, [$2])
  fi
]);


