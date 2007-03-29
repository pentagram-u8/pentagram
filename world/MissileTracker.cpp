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

#include "pent_include.h"

#include "MissileTracker.h"
#include "Item.h"
#include "getObject.h"

MissileTracker::MissileTracker(Item* item, sint32 tx, sint32 ty, sint32 tz,
							   sint32 speed, sint32 gravity_)
{
	assert(item->getParent() == 0);

	objid = item->getObjId();
	destx = tx;
	desty = ty;
	destz = tz;
	gravity = gravity_;

	sint32 x,y,z;
	item->getLocation(x,y,z);

	int range = abs(x - tx) + abs(y - ty);

	// rounded division: range/speed
	int frames = (range + (speed/2)) / speed;

	/*

	Item's vertical trajectory:

	z_{i+1} = z_i + s_i
	s_{i+1} = s_i - g

	(z_i = vertical position after i frames,
	 s_i = vertical speed after i frames, g = gravity)

	So:

	z_i = z + sum_{j=0}^{i-1} ( s_0 - jg)
	    = z + is_0 - 1/2 i(i-1)g

	Conclusion: if we want to reach the destination vertical level in i frames,
	we need to set

	s_0 = ((1/2 gi(i-1)) + z_i-z) / i

	*/

	if (frames > 0) {
		speedz = ((gravity * frames * (frames-1) / 2) + destz - z) / frames;

		// check if vertical speed isn't too high
		if (speedz > speed/4) {
			if (gravity == 0 || (speed / (4*gravity)) <= frames) {
				if (speed >= 4 && (destz-z)/(speed/4) > frames)
					frames = (destz-z)/(speed/4);
			} else {
				frames = speed/(4*gravity);
			}
		}

		speedz = ((gravity * frames * (frames-1) / 2) + destz - z) / frames;

		// horizontal speed is easier: just divide distance by frames
		speedx = ((destx - x) + (frames/2)) / frames;
		speedy = ((desty - y) + (frames/2)) / frames;

		pout.printf("MissileTracker: from (%d,%d,%d) to (%d,%d,%d)\n", x, y, z, tx, ty, tz);
		pout.printf("speed: %d, gravity: %d, frames: %d\n", speed, gravity, frames);
		pout.printf("resulting speed: (%d,%d,%d)\n", speedx, speedy, speedz);
	} else {

		// no significant horizontal movement
		if (destz > z)
			speedz = speed/4;
		else
			speedz = -speed/4;

	}
}

MissileTracker::~MissileTracker()
{

}

void MissileTracker::launchItem()
{
	Item* item = getItem(objid);
	if (!item) return;

	item->hurl(speedx, speedy, speedz, gravity);
}
