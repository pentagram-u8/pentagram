/*
Copyright (C) 2003 The Pentagram team

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

#ifndef HIDBINDING_H
#define HIDBINDING_H

#include "istring.h"
#include "SDL_events.h"
#include <map>

namespace HIDBindings { }

//! A function pointer to a action that is bound to input (usually keyboard)
//! \param event the SDL_Event to handle
//! \return true if the event could be handled
typedef bool (*HIDBinding)(const SDL_Event & event);

//<name, function>
//! The type of pair which is used in a HIDBindingMap
typedef std::pair<Pentagram::istring, HIDBinding> HIDBindingPair;

//! A Map containing HIDBindings and their names
typedef std::map<Pentagram::istring, HIDBinding> HIDBindingMap;

/**! \def HIDBINDING_PAIR(Function)
 * 	A Macro to automatically create a HIDBindingPair
 */
#define HIDBINDING_PAIR(Function)										\
	HIDBindingPair(Pentagram::istring(#Function), &HIDBindings::Function)

#define HID_UP		\
	SDL_MOUSEBUTTONUP:	\
	case SDL_KEYUP:			\
	case SDL_JOYBUTTONUP

#define HID_DOWN			\
	SDL_MOUSEBUTTONDOWN:	\
	case SDL_KEYDOWN:			\
	case SDL_JOYBUTTONDOWN

#endif
