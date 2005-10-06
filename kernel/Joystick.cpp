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

enum Joystick {
	JOY1 = 0,
	JOY_LAST
};

static SDL_Joystick * joy[JOY_LAST] = {0};

void InitJoystick()
{
	int i, buttons, axes, balls, hats;
	int joys = SDL_NumJoysticks();

	for (i = 0; i < joys; ++i)
	{
		if (i >= JOY_LAST)
		{
			perr << "Additional joysticks detected. Cannot initialize more than "
				<< JOY_LAST << "." << std::endl;
			break;
		}

		joy[i] = 0;

		if(! SDL_JoystickOpened(i))
		{
			joy[i] = SDL_JoystickOpen(i);
			if (joy[i])
			{
				buttons = SDL_JoystickNumButtons(joy[i]);
				axes = SDL_JoystickNumAxes(joy[i]);
				balls = SDL_JoystickNumBalls(joy[i]);
				hats = SDL_JoystickNumHats(joy[i]);

				pout << "Initialized joystick " << i + 1 << "." << std::endl;
				pout << "\tButtons: " << buttons << std::endl;
				pout << "\tAxes: " << axes << std::endl;
				pout << "\tBalls: " << balls << std::endl;
				pout << "\tHats: " << hats << std::endl;
			}
			else
			{
				perr << "Error while initializing joystick " << i + 1 << "."
					<< std::endl;
			}
		}
	}
}

void ShutdownJoystick()
{
	int i;
	for (i = 0; i < JOY_LAST; ++i)
	{
		if(joy[i] && SDL_JoystickOpened(i))
		{
			SDL_JoystickClose(joy[i]);
		}
		joy[i] = 0;
	}
}
