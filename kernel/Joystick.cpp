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

#include "SDL_timer.h"
#include "SDL_events.h"
#include "GUIApp.h"

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

DEFINE_RUNTIME_CLASSTYPE_CODE(JoystickCursorProcess,Process);

JoystickCursorProcess * JoystickCursorProcess::cursor_process = 0;

JoystickCursorProcess::JoystickCursorProcess(Joystick js_, int x_axis_, int y_axis_)
	: Process(), js(js_), x_axis(x_axis_), y_axis(y_axis_), ticks(0), accel(0)
{
	cursor_process = this;
	flags |= PROC_RUNPAUSED;
	type = 1;

	if(joy[js] && js < JOY_LAST)
	{
		int axes = SDL_JoystickNumAxes(joy[js]);
		if (x_axis >= axes && y_axis >= axes)
		{
			perr << "Failed to start JoystickCursorProcess: illegal axis for x (" << x_axis << ") or y (" << y_axis << ")" << std::endl;
			terminate();
		}
	}
	else
	{
		terminate();
	}

}

JoystickCursorProcess::~JoystickCursorProcess()
{
	cursor_process = 0;
}

#define AXIS_TOLERANCE 1000

//! CONSTANTS - and a lot of guess work
bool JoystickCursorProcess::run(const uint32 /*framenum*/)
{
	int dx = 0, dy = 0;
	int now = SDL_GetTicks();

	if(joy[js] && ticks)
	{
		int tx = now - ticks;
		int r = 350 - accel * 30;
		sint16 jx = SDL_JoystickGetAxis(joy[js], x_axis);
		sint16 jy = SDL_JoystickGetAxis(joy[js], y_axis);
		if (jx > AXIS_TOLERANCE || jx < -AXIS_TOLERANCE)
			dx = ((jx / 1000) * tx) / r;
		if (jy > AXIS_TOLERANCE || jy < -AXIS_TOLERANCE)
			dy = ((jy / 1000) * tx) / r;
	}

	ticks = now;

	if (dx || dy)
	{
		int mx, my;
		GUIApp * app = GUIApp::get_instance();
		app->getMouseCoords(mx, my);
		mx += dx;
		my += dy;
		app->setMouseCoords(mx, my);
		++accel;
		if (accel > 10)
			accel = 10;
	}
	else
	{
		accel = 0;
	}

	return false;
}

bool JoystickCursorProcess::loadData(IDataSource* ids, uint32 version)
{
	// saves no status
	return true;
}

void JoystickCursorProcess::saveData(ODataSource* ods)
{
	// saves no status
}
