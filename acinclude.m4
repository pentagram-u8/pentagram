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
  case $CC_FOR_BUILD,$HOST_CC,$SAVED_CC in
   ,,)    echo "int x;" > conftest.$ac_ext ;
          for c in cc gcc c89 c99 ; do
            if ($c -c -o conftest.o conftest.$ac_ext) >/dev/null 2>&1 ; then
               CC_FOR_BUILD="$c"; break ;
            fi ;
          done ;
          if test x"$CC_FOR_BUILD" = x ; then
            CC_FOR_BUILD=no_compiler_found ;
          fi
          ;;
   ,,*)   CC_FOR_BUILD=$SAVED_CC ;;
   ,*,*)  CC_FOR_BUILD=$HOST_CC ;;
  esac ;
  if test x$CC_FOR_BUILD = xno_compiler_found ; then
    AC_MSG_RESULT(none found)
  else
    AC_MSG_RESULT($CC_FOR_BUILD)
  fi

  # basis check for EXEEXT_FOR_BUILD
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



dnl Check if we have SDL_ttf (header and library)
dnl Input:
dnl SDL_CFLAGS, SDL_LIBS are set correctly by AM_PATH_SDL
dnl Output:
dnl SDLTTF_LIBS contains the necessary libs/ldflags
dnl HAVE_SDL_TTF_H is AC_DEFINE-d if including "SDL_ttf.h" works
dnl HAVE_SDL_SDL_TTF_H is AC_DEFINE-d if including <SDL/SDL_ttf.h> works

dnl TODO: accept arguments to run if SDL_ttf found/not found like other check
dnl       macros

AC_DEFUN([PENT_CHECK_SDLTTF],[
  AC_MSG_CHECKING(for SDL_ttf)

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


  dnl Next: try linking

  if test x$pent_sdlttfok = xyes; then

    LIBS="$LIBS -lSDL_ttf"

    AC_LINK_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"
    #ifdef HAVE_SDL_TTF
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
      #ifdef HAVE_SDL_TTF
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

  if test x$pent_sdlttfok = xyes; then
    ifelse([$1], , :, [$1])
  else
    ifelse([$2], , :, [$2])
  fi
]);


# Configure paths for SDL
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

# Modified by Willem Jan Palenstijn (2005-05-07)
# use CPPFLAGS instead of CFLAGS. (Fixes check with AC_LANG_CPLUSPLUS)


dnl AM_PATH_SDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL, and define SDL_CFLAGS and SDL_LIBS
dnl
AC_DEFUN([AM_PATH_SDL],
[dnl 
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)],
            sdl_prefix="$withval", sdl_prefix="")
AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)],
            sdl_exec_prefix="$withval", sdl_exec_prefix="")
AC_ARG_ENABLE(sdltest, [  --disable-sdltest       Do not try to compile and run a test SDL program],
		    , enable_sdltest=yes)

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

  AC_REQUIRE([AC_CANONICAL_TARGET])
  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
  min_sdl_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)
  no_sdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_sdl=yes
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_micro_version=`$SDL_CONFIG $sdl_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdltest" = "xyes" ; then
      ac_save_CPPFLAGS="$CPPFLAGS"
      ac_save_LIBS="$LIBS"
      CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
      LIBS="$LIBS $SDL_LIBS"
dnl
dnl Now check if the installed SDL is sufficiently new. (Also sanity
dnl checks the results of sdl-config to some extent
dnl
      rm -f conf.sdltest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sdl=yes,no_sdl=)
       CPPFLAGS="$ac_save_CPPFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl" = x ; then
     if test x$cross_compiling = xno; then
       AC_MSG_RESULT(yes)
     else
       AC_MSG_RESULT(cross compiling; assumed OK)
     fi
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDL_CONFIG" = "no" ; then
       echo "*** The sdl-config script installed by SDL could not be found"
       echo "*** If SDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL_CONFIG environment variable to the"
       echo "*** full path to sdl-config."
     else
       if test -f conf.sdltest ; then
        :
       else
          echo "*** Could not run SDL test program, checking why..."
          CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
          LIBS="$LIBS $SDL_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "SDL.h"

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL or finding the wrong"
          echo "*** version of SDL. If it is not finding SDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occurred. This usually means SDL was incorrectly installed"
          echo "*** or that you have moved SDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CPPFLAGS="$ac_save_CPPFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL_CFLAGS=""
     SDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
  rm -f conf.sdltest
])

