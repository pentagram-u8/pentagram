/*
Copyright (C) 2003-2007 The Pentagram team

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

#ifndef MISSILEPROCESS_H
#define MISSILEPROCESS_H

#include "Process.h"

// This class is DEPRECATED.
// It only exists to provide backward compatibility for savegames

class MissileProcess : public Process
{
public:
	MissileProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	sint32 from_x, from_y, from_z;
	sint32 to_x, to_y, to_z;

	int speed;
	bool curve;

	int currentpos;
};


#endif
