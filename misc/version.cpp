/*
Copyright (C) 2004-2006 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

namespace PentagramVersion {

const char *version = VERSION;

const char *features = ""
#ifdef DEBUG
  "debug "
#endif
#ifdef USE_VALGRIND
  "valgrind "
#endif
#ifdef USE_TIMIDITY_MIDI
  "Timidity "
#endif
#ifdef USE_FMOPL_MIDI
  "FMOPL "
#endif
#ifdef USE_FLUIDSYNTH_MIDI
  "FluidSynth "
#endif
#ifdef USE_ALSA_MIDI
  "ALSA "
#endif
#ifdef USE_HQ2X_SCALER
  "hq2x "
#endif
#ifdef BUILTIN_DATA
  "nodata "
#endif
;

const char *buildtime = __DATE__ " " __TIME__;

}
