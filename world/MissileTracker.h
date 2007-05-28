/*
Copyright (C) 2007 The Pentagram team

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

#ifndef MISSILETRACKER_H
#define MISSILETRACKER_H

class Item;

class MissileTracker
{
public:
	MissileTracker(Item* item, sint32 sx, sint32 sy, sint32 sz,
				   sint32 tx, sint32 ty, sint32 tz,
				   sint32 speed, sint32 gravity);
	MissileTracker(Item* item, sint32 tx, sint32 ty, sint32 tz,
				   sint32 speed, sint32 gravity);
	~MissileTracker();

	bool isPathClear();

	void launchItem();

protected:
	void init(sint32 sx, sint32 sy, sint32 sz, sint32 speed);

private:
	ObjId objid;
	sint32 destx, desty, destz;
	sint32 speedx, speedy, speedz;
	sint32 gravity;
	int frames;
};

#endif
