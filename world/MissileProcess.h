/*
Copyright (C) 2003 The Pentagram team

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

class Item;

class MissileProcess : public Process
{
public:
	//! note: this probably needs some more parameters
	MissileProcess(Item* item, sint32 to_x, sint32 to_y, sint32 to_z,
					int speed, bool curve=false);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual bool run(const uint32 framenum);

private:
	uint16 item;

	sint32 from_x, from_y, from_z;
	sint32 to_x, to_y, to_z;

	int speed;
	bool curve;

	int currentpos;
};


#endif
