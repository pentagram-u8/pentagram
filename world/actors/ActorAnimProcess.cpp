/*
Copyright (C) 2003-2004 The Pentagram team

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

#include "ActorAnimProcess.h"
#include "GameData.h"
#include "MainShapeFlex.h"
#include "Animation.h"
#include "AnimDat.h"
#include "AnimAction.h"
#include "Actor.h"
#include "Direction.h"
#include "World.h"
#include "GravityProcess.h"
#include "Kernel.h"
#include "UCList.h"
#include "LoopScript.h"
#include "CurrentMap.h"
#include "ShapeInfo.h"
#include "AnimationTracker.h"

#include "IDataSource.h"
#include "ODataSource.h"

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ActorAnimProcess,Process);

ActorAnimProcess::ActorAnimProcess() : Process(), tracker(0)
{

}

ActorAnimProcess::ActorAnimProcess(Actor* actor_, Animation::Sequence action_,
								   uint32 dir_)
{
	assert(actor_);
	item_num = actor_->getObjId();
	dir = dir_;
	action = action_;

	type = 0x00F0; // CONSTANT !
	firstframe = true;
	tracker = 0;
}

bool ActorAnimProcess::init()
{
	if (dir > 7) {
		// invalid direction
		return false;
	}

	repeatcounter = 0;
	animAborted = false;
	attackedSomething = false;

	Actor* actor = World::get_instance()->getNPC(item_num);
	assert(actor);

	if (!(actor->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Can't play an animation then.
		// (If we do, the actor will likely fall because the floor is gone.)
		return false;
	}

	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		perr << "ActorAnimProcess: ANIMLOCK set on actor "
			 << item_num << std::endl;

		// for now, just don't play this one.
		return false;
	}

	actor->setActorFlag(Actor::ACT_ANIMLOCK);

	tracker = new AnimationTracker(actor, action, dir);

	actor->lastanim = action;
	actor->direction = dir;


#ifdef WATCHACTOR
	if (item_num == watchactor)
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
			 << "] ActorAnimProcess " << getPid() << " created ("
			 << action << "," << dir << ")" << std::endl;
#endif

	return true;
}


bool ActorAnimProcess::run(const uint32 framenum)
{
	if (firstframe) {
		bool ret = init();
		if (!ret) {
			// initialization failed
			terminate();
			return false;
		}
	}

	if (animAborted) {
		terminate();
		return false;
	}

	assert(tracker);

	if (!firstframe)
		repeatcounter++;
	if (repeatcounter >= tracker->getAnimAction()->framerepeat)
		repeatcounter = 0;

	Actor *a = World::get_instance()->getNPC(item_num);
	if (!a) {
		// actor gone
		terminate();
		return false;
	}

	firstframe = false;


	bool result = true;
	if (repeatcounter == 0) {
		// next step:
		result = tracker->step();
		tracker->updateActorFlags();

		if (!result) {
			// check possible error conditions

			if (tracker->isDone()) {
				// all done
#ifdef WATCHACTOR
				if (item_num == watchactor)
					pout << "Animation ["
						 << Kernel::get_instance()->getFrameNum()
						 << "] ActorAnimProcess done; terminating"
						 << std::endl;
#endif
			terminate();
			return true;
			}


			if (tracker->isBlocked() &&
				!(tracker->getAnimAction()->flags&AnimAction::AAF_UNSTOPPABLE))
			{
#ifdef WATCHACTOR
				if (item_num == watchactor)
					pout << "Animation ["
						 << Kernel::get_instance()->getFrameNum()
						 << "] ActorAnimProcess blocked; terminating"
						 << std::endl;
#endif
				terminate();
				return true;
			}
		}
	}

	sint32 x,y,z;
	tracker->getInterpolatedPosition(x,y,z,repeatcounter+1);

#ifdef WATCHACTOR
	if (repeatcounter == 0 && item_num == watchactor)
		pout << "Animation [" << Kernel::get_instance()->getFrameNum()
			 << "] showing next frame (" << x << "," << y << "," << z << ")"
			 << std::endl;
#endif


	a->collideMove(x, y, z, false, true); // forced move
	a->setFrame(tracker->getFrame());

	if (repeatcounter == 0) {
		if (!result && tracker->isUnsupported()) {
			animAborted = true;
			GravityProcess* gp = new GravityProcess(a, 4);
			uint16 gppid = Kernel::get_instance()->addProcess(gp);
			// TODO: inertia
			
			// CHECKME: do we need to wait for the fall to finish?
			waitFor(gppid);
			return true;
		}

		// attacking?
		if ((tracker->getAnimAction()->flags & AnimAction::AAF_ATTACK) && 
			!attackedSomething && tracker->getAnimFrame()->attack_range()) {
			
			// check if there's anything in range
			ObjId hit = checkWeaponHit(dir, tracker->getAnimFrame()->
									   attack_range());
			
			if (hit) {
				attackedSomething = true;
				Item* hit_item = World::get_instance()->getItem(hit);
				assert(hit_item);
				hit_item->receiveHit(item_num, dir, 0, 0); // CHECKME: dir?
			}
		}
	}

	return true;

#if 0
	unsigned int frameindex = currentindex / animaction->framerepeat;
	int framecount = currentindex % animaction->framerepeat;

	// check if we're done
	if (framecount == 0) {
		if (frameindex == endframe) {

			// if so, toggle ACT_FIRSTSTEP flag if necessary
			if (animaction->flags & AnimAction::AAF_TWOSTEP) {
				if (a->getActorFlags() & Actor::ACT_FIRSTSTEP) {
					a->clearActorFlag(Actor::ACT_FIRSTSTEP);
				} else {
					a->setActorFlag(Actor::ACT_FIRSTSTEP);
				}
			} else {
				a->setActorFlag(Actor::ACT_FIRSTSTEP);
			}
			
#ifdef WATCHACTOR
			if (item_num == watchactor)
				pout << "Animation [" << Kernel::get_instance()->getFrameNum()
					 << "] ActorAnimProcess terminating" << std::endl;
#endif		 
			terminate();
			return true;
		}

		// loop if necessary
		if (frameindex >= animaction->size) {
			if (animaction->flags & AnimAction::AAF_LOOPING) {
				frameindex = 1;
			} else {
				frameindex = 0;
			}
		}	

		currentindex = frameindex * animaction->framerepeat;
		a->animframe = frameindex;
	}

	AnimFrame& f = animaction->frames[dir][frameindex];

	//! TODO: check if actor is still in fastarea
	//!       check if being controlled by a GravityTracker

	// Adding position works like this
	// subtract how much we've already added
	// then add how much we want. This is to get around issues
	// with accuracy

	sint32 x, y, z;
	sint32 dx, dy, dz;
	a->getLocation(x,y,z);

	dx = 4 * x_fact[dir] * f.deltadir;
	dy = 4 * y_fact[dir] * f.deltadir;
	dz = f.deltaz;

	int fc = framecount;

	x -= (dx*fc)/animaction->framerepeat;
	y -= (dy*fc)/animaction->framerepeat;
	z -= (dz*fc)/animaction->framerepeat;

	// special case:
	// if walking, and can't reach destination, see if we can step up 8
	if (framecount == 0 &&
		animaction->action == Animation::walk &&
		!a->canExistAt(x+dx, y+dy, z+dz) &&
		a->canExistAt(x+dx, y+dy, z+dz+8))
	{
		sint32 oldx, oldy, oldz;
		a->getLocation(oldx, oldy, oldz);
		oldz += 8;
		a->collideMove(oldx, oldy, oldz, false, false);
		z += 8;
	}

	x += (dx*(fc+1))/animaction->framerepeat;
	y += (dy*(fc+1))/animaction->framerepeat;
	z += (dz*(fc+1))/animaction->framerepeat;

//	a->move(x,y,z);
	sint32 dist = a->collideMove(x, y, z, false, false);
	a->setFrame(f.frame);

	if (f.is_flipped()) {
		a->setFlag(Item::FLG_FLIPPED);
	} else {
		a->clearFlag(Item::FLG_FLIPPED);
	}

	if (dist < 0x4000 && !(animaction->flags & AnimAction::AAF_UNSTOPPABLE)) {
#ifdef WATCHACTOR
			if (item_num == watchactor)
				pout << "Animation [" << Kernel::get_instance()->getFrameNum()
					 << "] blocked (dist = " << dist << ")" << std::endl;
#endif
		// didn't reach destination, so we need to abort
		// one exception: if walking, we can step up 8
		// TODO: walking up stairs

		// TODO: if jumping/running: something (fall, hit, ...)
		terminate();
	}

	if (framecount == animaction->framerepeat-1 &&
		f.flags & AnimFrame::AFF_ONGROUND) {
		// check if we're supported by something.
		// If not, check how far we're going to fall.
		// If further than some constant, abort the animation and start
		// falling (some inertia would be nice).
		// If just a little distance, collideMove() down a bit

		// FIXME: using a bit of a hack for now:
		if (a->canExistAt(x,y,z-8)) {

#ifdef WATCHACTOR
			if (item_num == watchactor)
				pout << "Animation [" << Kernel::get_instance()->getFrameNum()
					 << "] falling" << std::endl;
#endif
			if (a->canExistAt(x,y,z-16)) {
				// too far...
				animAborted = true;
				GravityProcess* gp = new GravityProcess(a, 4);
				uint16 gppid = Kernel::get_instance()->addProcess(gp);
				// TODO: inertia

				// CHECKME: do we need to wait for the fall to finish?
				waitFor(gppid);
				return true;
			} else {
				// just a bit
				a->collideMove(x,y,z-8,false,false);
			}
		}
	}


	// attacking?
	if ((animaction->flags && AnimAction::AAF_ATTACK) && 
		!hitSomething && f.attack_range()) {

		// check if there's anything in range
		ObjId hit = checkWeaponHit(dir, f.attack_range());

		if (hit) {
			hitSomething = true;
			Item* hit_item = World::get_instance()->getItem(hit);
			assert(hit_item);
			hit_item->receiveHit(item_num, dir, 0, 0); // CHECKME: dir?
		}
	}


#ifdef WATCHACTOR
//	if (framecount == 0) {
		if (item_num == watchactor)
			pout << "Animation [" << Kernel::get_instance()->getFrameNum()
				 << "] showing frame (" << frameindex << "/"
				 << animaction->size << ", " << framecount << "/"
				 << animaction->framerepeat << ")" << std::endl;
//	}
#endif

	return true;
#endif
}


void ActorAnimProcess::terminate()
{
	Actor *a = World::get_instance()->getNPC(item_num);
	if (a) {
		if (tracker) // if we were really animating...
			a->clearActorFlag(Actor::ACT_ANIMLOCK);
	}

	delete tracker;

	Process::terminate();
}

ObjId ActorAnimProcess::checkWeaponHit(int dir, int range)
{
	pout << "Checking hit (" << range << "): ";

	Actor *a = World::get_instance()->getNPC(item_num);
	assert(a);

	CurrentMap* cm = World::get_instance()->getCurrentMap();

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_END);

	// CHECKME: check range
	cm->areaSearch(&itemlist, script, sizeof(script), a, 16*range, false);

	ObjId hit = 0;
	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		ObjId itemid = itemlist.getuint16(i);
		Item* item = World::get_instance()->getItem(itemid);
		assert(item);
		sint32 ix,iy,iz;
		item->getLocationAbsolute(ix,iy,iz);
		sint32 ax,ay,az;
		a->getLocationAbsolute(ax,ay,az);
		int dirdelta = abs(a->getDirToItemCentre(*item) - dir);
		if ((dirdelta <= 1 || dirdelta >= 7) &&
			!a->getShapeInfo()->is_fixed() && itemid < 256) {
			// FIXME: should allow item to be only slightly outside of
			//        the right direction
			// FIXME: shouldn't only allow hitting NPCs
			hit = itemid;
			pout << "hit ";
			item->dumpInfo();
			break;
		}
	}

	if (!hit) {
		pout << "nothing" << std::endl;
	}

	return hit;
}


void ActorAnimProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	uint8 ab = animAborted ? 1 : 0;
	ods->write1(ab);
	uint8 attacked = attackedSomething ? 1 : 0;
	ods->write1(attacked);
	ods->write1(static_cast<uint8>(dir));
	ods->write2(static_cast<uint16>(action));
	ods->write2(static_cast<uint16>(repeatcounter));

	if (tracker) {
		ods->write1(1);
		tracker->save(ods);
	} else
		ods->write1(0);
}

bool ActorAnimProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	firstframe = (ids->read1() != 0);
	animAborted = (ids->read1() != 0);
	attackedSomething = (ids->read1() != 0);
	dir = ids->read1();
	action = static_cast<Animation::Sequence>(ids->read2());
	repeatcounter = ids->read2();

	assert(tracker == 0);
	if (ids->read1() != 0) {
		tracker = new AnimationTracker();
		if (!tracker->load(ids))
			return false;
	}

	return true;
}
