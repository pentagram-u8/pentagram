/*
Copyright (C) 2002-2004 The Pentagram team

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

#ifndef MOUSE_H
#define MOUSE_H

const int DOUBLE_CLICK_TIMEOUT = 200;
const int NUM_MOUSEBUTTONS = 10;

struct MButton
{
	uint16 downGump;
	uint32 lastDown;
	int downX, downY;
	int state;
};

enum MouseButtonState {
	MBS_DOWN = 0x1,
	MBS_HANDLED = 0x2
};

enum MouseButton {
	BUTTON_LEFT = 1,
	BUTTON_MIDDLE = 2,
	BUTTON_RIGHT = 3,
	WHEEL_UP = 4,
	WHEEL_DOWN = 5,
	MOUSE_6 = 6,
	MOUSE_7 = 7,
	MOUSE_8 = 8,
	MOUSE_9 = 9,
	MOUSE_10 = 10
};

const char* GetMouseButtonName(MouseButton button);

#endif
