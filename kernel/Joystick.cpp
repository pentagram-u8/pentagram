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
#include "Joystick.h"

#include "SDL_events.h"

static SDL_Joystick * joy1 = 0;

static const char * joyButtonName[] = {
	"Joy1-Bx00",
	"Joy1-Bx01",
	"Joy1-Bx02",
	"Joy1-Bx03",
	"Joy1-Bx04",
	"Joy1-Bx05",
	"Joy1-Bx06",
	"Joy1-Bx07",
	"Joy1-Bx08",
	"Joy1-Bx09",
	"Joy1-Bx0A",
	"Joy1-Bx0B",
	"Joy1-Bx0C",
	"Joy1-Bx0D",
	"Joy1-Bx0E",
	"Joy1-Bx0F"
};

const char* GetJoystickButtonName(uint16 joystick, uint16 button)
{
	// Limitations
	if (button < NUM_JOYBUTTONS && joystick < NUM_JOYSTICKS)
		return joyButtonName[button];

	return 0;
}

void InitJoystick()
{
	if (SDL_NumJoysticks() > 0)
	{
		if(! SDL_JoystickOpened(0))
		{
			joy1 = SDL_JoystickOpen(0);
			if (joy1)
			{
				pout << "Initialized joystick." << std::endl;
				pout << "\tButtons: " << SDL_JoystickNumButtons(joy1)
					 << std::endl;
				pout << "\tAxes: " << SDL_JoystickNumAxes(joy1)
					 << std::endl;
				pout << "\tBalls: " << SDL_JoystickNumBalls(joy1)
					 << std::endl;
				pout << "\tHats: " << SDL_JoystickNumHats(joy1)
					 << std::endl;
			}
			else
			{
				pout << "Error while initializing joystick." << std::endl;
			}
		}
	}
}

void ShutdownJoystick()
{
	if(SDL_JoystickOpened(1))
	{
		SDL_JoystickClose(joy1);
	}
	joy1 = 0;
}
