/*
Copyright (C) 2003-2004 The Pentagram team

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

#ifndef U8BINDINGS_H
#define U8BINDINGS_H

#include "HIDBinding.h"

namespace HIDBindings {

// ! opens the Avatar's backback
// ! \see HIDBinding
bool openBackpack(const SDL_Event& event);

// ! opens the Avatar's Inventory Panel
// ! \see HIDBinding
bool openInventory(const SDL_Event& event);

// ! Activates recall
// ! \see HIDBinding
bool recall(const SDL_Event& event);

// ! run the execution scene at the docks
// ! \see HIDBinding
bool runExecutionEgg(const SDL_Event& event);

// ! run the begining scene 
// ! \see HIDBinding
bool runFirstEgg(const SDL_Event& event);

// ! toggles the avatar combat readiness
// ! \see HIDBinding
bool toggleCombat(const SDL_Event& event);

// ! U8 shape viewer
// ! \see HIDBinding
bool u8ShapeViewer(const SDL_Event& event);

};

#endif
