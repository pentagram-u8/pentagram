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

#include "pent_include.h"

#include "GravityProcess.h"
#include "Item.h"
#include "World.h"
#include "CurrentMap.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GravityProcess,Process);

GravityProcess::GravityProcess(Item* item, int gravity_)
	: xspeed(0), yspeed(0), zspeed(0)
{
	assert(item);

	gravity = gravity_;
	item_num = item->getObjId();

	//TODO: process type?
}

void GravityProcess::init()
{
	Item* item = World::get_instance()->getItem(item_num);
	assert(item);

	// we're bouncing (I guess...)
	item->setFlag(Item::FLG_BOUNCING);

	item->setGravityProcess(getPid());
}

void GravityProcess::move(int xs, int ys, int zs)
{
	xspeed += xs;
	yspeed += ys;
	zspeed += zs;
}

void GravityProcess::setGravity(int gravity_)
{
	// only apply gravity if stronger than current gravity
	//!!! is this correct?
	if (gravity_ > gravity)
		gravity = gravity_;
}

bool GravityProcess::run(uint32 framenum)
{
	// move item in (xs,ys,zs) direction
	Item* item = World::get_instance()->getItem(item_num);
	if (!item) {
		terminate();
		return false;
	}

	CurrentMap* cm = World::get_instance()->getCurrentMap();

	// What to do:
	//   - check if item can move to the given position
	//     (in intervals in the z direction, since the z movement
	//      can be quite large)
	//   - if item can move, move it
    //   - if item can't move, it hit something:
    //     - bounce off the item (need to consider FLG_LOW_FRICTION?)
    //     - call the relevant events: hit/gothit ?

	sint32 ix,iy,iz;
	item->getLocation(ix, iy, iz);
	sint32 ixd,iyd,izd;
	item->getFootpad(ixd, iyd, izd);
	ixd *= 32; iyd *= 32; izd *= 8; //!! constants

	sint32 tx,ty,tz;
	tx = ix + xspeed;
	ty = iy + yspeed;
	tz = iz + zspeed;

	// collision detection. Move in steps half the item's height
	// (and corresponding amounts in x/y directions)
	if (izd == 0) izd = 8; //!! cheating a little to prevent 0-height
	                       //!! objects from falling through 0-height tiles

	bool valid = true;
	int curz = iz;
	int zstepsize = izd / 2;
	if (tz < iz) zstepsize = -zstepsize;
	if (tz == iz) {
		valid = cm->isValidPosition(tx,ty,tz,ixd,iyd,izd,item_num,0,0);
	} else {
		do {
			curz += zstepsize;
			int curx = ix + ((tx - ix) * (curz-iz))/(tz-iz);
			int cury = iy + ((ty - iy) * (curz-iz))/(tz-iz);
		
			if ((zstepsize > 0 && curz > tz) || (zstepsize < 0 && curz < tz))
				curz = tz;

			valid &= cm->isValidPosition(curx, cury, curz,
										 ixd, iyd, izd, item_num, 0, 0);
		} while (valid && curz != tz);
	}

	if (valid) {
		item->move(tx, ty, tz);

		// apply gravity acceleration
		zspeed -= gravity;
	} else {
		//!! bounce
		if (tz != iz) {
			curz -= zstepsize;
			if ((zstepsize > 0 && curz < iz) || (zstepsize < 0 && curz > iz))
				curz = iz;

			int curx = ix + ((tx - ix) * (curz-iz))/(tz-iz);
			int cury = iy + ((ty - iy) * (curz-iz))/(tz-iz);

			if (cm->isValidPosition(curx, cury, curz,
									ixd, iyd, izd, item_num, 0, 0))
			{
				item->move(curx, cury, curz);
			}
		}

		//!! TODO...

		terminate();
	}

	return true;
}


void GravityProcess::terminate()
{
	//signal item GravityProcess is gone
	Item* item = World::get_instance()->getItem(item_num);
	if (item) {
		item->setGravityProcess(0);
		
		// no longer bouncing
		item->clearFlag(Item::FLG_BOUNCING);
	}

	Process::terminate();
}
