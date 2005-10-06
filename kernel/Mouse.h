/*
Copyright (C) 2002-2005 The Pentagram team

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

const unsigned int DOUBLE_CLICK_TIMEOUT = 200;

struct MButton
{
	uint16 downGump;
	uint32 lastDown;
	uint32 curDown;
	int downX, downY;
	int state;
};

enum MouseButtonState {
	MBS_DOWN = 0x1,
	MBS_HANDLED = 0x2,
	MBS_RELHANDLED = 0x4
};

enum MouseButton {
	BUTTON_LEFT = 1,
	BUTTON_MIDDLE,
	BUTTON_RIGHT,
	MOUSE_LAST
};

#endif
