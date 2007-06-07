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

#include "pent_include.h"

#include "GravityProcess.h"

#include "Actor.h"
#include "AudioProcess.h"
#include "CurrentMap.h"
#include "Kernel.h"
#include "World.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include <cmath>


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GravityProcess,Process);

GravityProcess::GravityProcess()
	: Process()
{

}

GravityProcess::GravityProcess(Item* item, int gravity_)
	: xspeed(0), yspeed(0), zspeed(0)
{
	assert(item);

	gravity = gravity_;
	item_num = item->getObjId();

	type = 0x203; // CONSTANT!
}

void GravityProcess::init()
{
	Item* item = getItem(item_num);
	assert(item);

	item->setGravityPID(getPid());

	Actor* actor = p_dynamic_cast<Actor*>(item);
	if (actor) {
		actor->setFallStart(actor->getZ());
	}
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
	Item* item = getItem(item_num);
	if (!item) {
		terminate();
		return false;
	}

	Actor* actor = p_dynamic_cast<Actor*>(item);
	if (actor && actor->getFallStart() < actor->getZ()) {
		actor->setFallStart(actor->getZ());
	}

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
	item->getFootpadWorld(ixd, iyd, izd);

	sint32 tx,ty,tz;
	tx = ix + xspeed;
	ty = iy + yspeed;
	tz = iz + zspeed;

	bool clipped = false;

	// Clip to region. This doesn't work
#if 0
	if (tx < 0 && ix >= 0) 
	{
		sint32 scale = (ix - tx)>>0x8;
		tx = 0;
		ty = iy + ((yspeed*scale)>>0x2000);
		tz = iz + ((zspeed*scale)>>0x2000);
		clipped = true;
	}
	if (ty < 0 && iy >= 0) 
	{
		sint32 scale = (iy - ty)>>0x8;
		tx = ix + ((xspeed*scale)>>0x2000);
		ty = 0;
		tz = iz + ((zspeed*scale)>>0x2000);
		clipped = true;
	}
	if (tz < 0 && iz >= 0) 
	{
		sint32 scale = (iz - tz)>>0x8;
		tx = ix + ((xspeed*scale)>>0x2000);
		ty = iy + ((yspeed*scale)>>0x2000);
		tz = 0;
		clipped = true;
	}
#endif

//#define BOUNCE_DIAG

	ObjId hititemid;
	sint32 dist = item->collideMove(tx,ty,tz, false, false, &hititemid);
	
	if (dist == 0 || clipped)
	{
		// If it landed on top of hititem and hititem is not land, the item
		// should bounce.
		bool terminate = true;
		Item* hititem = getItem(hititemid);
		if(zspeed < -2 && !p_dynamic_cast<Actor*>(item))
		{
#ifdef BOUNCE_DIAG
			pout << "item " << item_num << " bounce [" << framenum
				 << "]: hit " << hititem->getObjId() << std::endl;
#endif

			sint32 hitx,hity,hitz;
			hititem->getLocation(hitx,hity,hitz);
			sint32 hitdx,hitdy,hitdz;
			hititem->getFootpadWorld(hitdx,hitdy,hitdz);
			sint32 endx,endy,endz;
			item->getLocation(endx,endy,endz);

			// TODO: bounce off vertical surface impacts too?  This only
			// deals with hitting an item from the top.

			bool allow_land_bounce = ((0-zspeed) > 2*gravity);
			if(endz == hitz + hitdz &&	// hit an item at the end position
			   (allow_land_bounce || !hititem->getShapeInfo()->is_land()))
			{
				// Bounce!
				terminate = false;
#ifdef BOUNCE_DIAG
				int xspeedold = xspeed;
				int yspeedold = yspeed;
				int zspeedold = zspeed;
#endif
				zspeed = 0-zspeed/3;
				int approx_v = abs(xspeed) + abs(yspeed) + zspeed;

				// Apply an impulse on the x/y plane in a random direction
				// in a 180 degree pie around the orginal vector in x/y
				double heading_r = atan2(yspeed, xspeed);
				double deltah_r = static_cast<double>(rand())
								  * M_PI / RAND_MAX - M_PI/2;
#ifdef BOUNCE_DIAG
				double headingold_r = heading_r;
#endif
				heading_r += deltah_r;
				if(heading_r > M_PI) heading_r -= 2*M_PI;
				if(heading_r < -M_PI) heading_r += 2*M_PI;
				yspeed += static_cast<int>(sin(heading_r) *
										   static_cast<double>(approx_v));
				xspeed += static_cast<int>(cos(heading_r) *
										   static_cast<double>(approx_v));

				if(hititem->getShapeInfo()->is_land()) {
					// Bouncing off land; this bounce approximates what's
					// seen in the original U8 when the key thrown by
					// Kilandra's daughters ghost lands on the grass.
					xspeed /= 4;
					yspeed /= 4;
					zspeed /= 2;
					if(zspeed == 0) terminate = true;
				} else {
					// Not on land; this bounce approximates what's seen
					// in the original U8 when Kilandra's daughters ghost
					// throws a key at the Avatar's head
					if(abs(yspeed) > 2) yspeed /= 2;
					if(abs(xspeed) > 2) xspeed /= 2;
				}
#ifdef BOUNCE_DIAG
				pout << "item " << item_num << " bounce [" << framenum
					 << "]: speed was (" << xspeedold << ","
					 << yspeedold << "," << zspeedold << ") new zspeed "
					 << zspeed << " heading " << headingold_r
					 << " impulse " << heading_r << " ("
					 << (xspeed-xspeedold) << "," << (yspeed-yspeedold)
					 << "), terminate: " << terminate << std::endl;
#endif
			} else {
#ifdef BOUNCE_DIAG
				pout << "item " << item_num << " bounce [" << framenum
					 << "]: no bounce" << std::endl;
#endif
			}
		} else {
#ifdef BOUNCE_DIAG
			pout << "item " << item_num << " bounce [" << framenum
				 << "]: slow hit" << std::endl;
#endif			
		}
		if(terminate) {
			item->clearFlag(Item::FLG_BOUNCING);
			terminateDeferred();
		}
		else {
			item->setFlag(Item::FLG_BOUNCING);
		}
	}
	else 
		zspeed -= gravity;

#if 0

	CurrentMap* cm = World::get_instance()->getCurrentMap();

	// collision detection. Move in steps half the item's height
	// (and corresponding amounts in x/y directions)
	if (izd == 0) izd = 8; //!! cheating a little to prevent 0-height
	                       //!! objects from falling through 0-height tiles

	uint32 shapeflags = item->getShapeInfo()->flags;

	bool valid = true;
	int curz = iz;
	int zstepsize = izd / 2;
	if (tz < iz) zstepsize = -zstepsize;
	if (tz == iz) {
		valid = cm->isValidPosition(tx,ty,tz,ixd,iyd,izd,shapeflags,
									item_num,0,0);
	} else {
		do {
			curz += zstepsize;
			int curx = ix + ((tx - ix) * (curz-iz))/(tz-iz);
			int cury = iy + ((ty - iy) * (curz-iz))/(tz-iz);
		
			if ((zstepsize > 0 && curz > tz) || (zstepsize < 0 && curz < tz))
				curz = tz;

			valid &= cm->isValidPosition(curx, cury, curz, ixd, iyd, izd,
										 shapeflags,item_num, 0, 0);
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

			if (cm->isValidPosition(curx, cury, curz, ixd, iyd, izd,
									shapeflags, item_num, 0, 0))
			{
				item->move(curx, cury, curz);
			}
		}

		//!! TODO...

		terminate();
	}
#endif
	return true;
}


void GravityProcess::terminate()
{
	//signal item GravityProcess is gone
	Item* item = getItem(item_num);
	if (item) {
		item->setGravityPID(0);
		
		// no longer bouncing
		item->clearFlag(Item::FLG_BOUNCING);
	}

	Process::terminate();

	// Note: we need to terminate before calling receiveHit on actor.
	// If we don't, receiveHit will try to terminate this GravityProcess,
	// which will cause infinite recursion.

	Actor* actor = p_dynamic_cast<Actor*>(item);
	if (actor && !actor->isDead()) {
		// actors take a hit if they fall
		// CHECKME: might need to do a 'die' animation even if actor is dead

		int height = actor->getFallStart() - actor->getZ();

		if (height >= 80) {
			int damage = 0;

			if (height < 104) {
				// medium fall: take some damage
				damage = (height - 72)/4;
			} else {
				// high fall: die
				damage = actor->getHP();
			}

			actor->receiveHit(0, actor->getDir(), damage,
							  WeaponInfo::DMG_FALLING|WeaponInfo::DMG_PIERCE);

			// 'ooof'
			AudioProcess* audioproc = AudioProcess::get_instance();
			if (audioproc) audioproc->playSFX(51, 250, item_num, 0); // CONSTANT!
		}

		if (!actor->isDead() && actor->getLastAnim() != Animation::die) {

			// play land animation, overriding other animations
			Kernel::get_instance()->killProcesses(item_num, 0xF0, false); // CONSTANT!
			ProcId lpid = actor->doAnim(Animation::land, 8);

			if (actor->isInCombat()) {
				// need to get back to a combat stance to prevent weapon from
				// being drawn again
				ProcId spid = actor->doAnim(Animation::combatStand, 8);
				Process* sp = Kernel::get_instance()->getProcess(spid);
				sp->waitFor(lpid);
			}
		}
	}
}

void GravityProcess::dumpInfo()
{
	Process::dumpInfo();

	pout << "gravity: " << gravity << ", speed: (" << xspeed << ","
		 << yspeed << "," << zspeed << ")" << std::endl;
}


void GravityProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(gravity));
	ods->write4(static_cast<uint32>(xspeed));
	ods->write4(static_cast<uint32>(yspeed));
	ods->write4(static_cast<uint32>(zspeed));
}

bool GravityProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	gravity = static_cast<int>(ids->read4());
	xspeed = static_cast<int>(ids->read4());
	yspeed = static_cast<int>(ids->read4());
	zspeed = static_cast<int>(ids->read4());

	return true;
}
