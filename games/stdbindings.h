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

#ifndef STDBINDINGS_H
#define STDBINDINGS_H

#include "HIDBinding.h"

namespace HIDBindings {

// ! toggles the avatar in and out of stasis
// ! \see HIDBinding
bool avatarInStasis(const HID_Event& event);

// ! displays pentagram's engine statistics
// ! \see HIDBinding
bool engineStats(const HID_Event& event);

// ! attempts to locate an item
// ! \see HIDBinding
bool itemLocator(const HID_Event& event);

// ! toggles the painting of editor items
// ! \see HIDBinding
bool paintEditorItems(const HID_Event& event);

// ! toggles the highlighting of items touching the main actor
// ! \see HIDBinding
bool showTouchingItems(const HID_Event& event);

// ! binding to quickload
// ! \see HIDBinding
bool quickLoad(const HID_Event& event);

// ! binding to quicksave
// ! \see HIDBinding
bool quickSave(const HID_Event& event);

// ! binding to toggle frame-by-frame mode
// ! \see HIDBinding
bool toggleFrameByFrame(const HID_Event& event);

// ! binding to go to the next frame in frame-by-frame mode
// ! \see HIDBinding
bool advanceFrameByFrame(const HID_Event& event);

// ! binding to ask if the user wishes to quit
// ! \see HIDBinding
bool quit(const HID_Event& event);

// ! toggles the console up and down
// ! \see HIDBinding
bool toggleConsole(const HID_Event& event);

// ! moves main actor up without playing any animation
// ! \see HIDBinding
bool quickMoveUp(const HID_Event& event);

// ! moves main actor down without playing any animation
// ! \see HIDBinding
bool quickMoveDown(const HID_Event& event);

// ! moves main actor left without playing any animation
// ! \see HIDBinding
bool quickMoveLeft(const HID_Event& event);

// ! moves main actor right without playing any animation
// ! \see HIDBinding
bool quickMoveRight(const HID_Event& event);

// ! main actor ascends without playing any animation
// ! \see HIDBinding
bool quickMoveAscend(const HID_Event& event);

// ! main actor descends without playing any animation
// ! \see HIDBinding
bool quickMoveDescend(const HID_Event& event);

// ! quick moves act at quater speed
// ! \see HIDBinding
bool quickMoveQuarterSpeed(const HID_Event& event);

// ! toggles clipping on quick moves
// ! \see HIDBinding
bool quickMoveClipping(const HID_Event& event);

};

#endif
