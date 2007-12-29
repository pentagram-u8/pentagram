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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "Process.h"

/*
   For now, we are limiting to one joystick, 16 buttons,
   and we are ignoring anything other than buttons and axes (hats and balls)
*/

enum Joystick {
	JOY1 = 0,
	JOY_LAST
};

void InitJoystick();
void ShutdownJoystick();

class JoystickCursorProcess : public Process {
public:
	JoystickCursorProcess();
	JoystickCursorProcess(Joystick js_, int x_axis_, int y_axis_);
	virtual ~JoystickCursorProcess();

	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	Joystick js;
	int x_axis, y_axis;
	int ticks;
	int accel;
};

#endif
