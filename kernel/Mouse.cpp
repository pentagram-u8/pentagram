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

#include "pent_include.h"
#include "Mouse.h"

static const char* mouseButtonName[] = {
	"",
	"LeftButton",
	"MiddleButton",
	"RightButton",
	"WheelUp",
	"WheelDown",
	"Mouse6",
	"Mouse7",
	"Mouse8",
	"Mouse9",
	"Mouse10"
};

const char* GetMouseButtonName(MouseButton button)
{
	assert(button > 0 && button <= NUM_MOUSEBUTTONS);
	return mouseButtonName[button];
}
