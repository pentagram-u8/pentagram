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

	if (!(a->getFlags() & Item::FLG_FASTAREA)) {
		// not in the fast area? Kill the animation then.
		//! TODO: Decide if this is the right move.
		//  Animation could do one of three things: pause, move
		//  without allowing actor to fall, or pretend to move and
		//  complete the entire movement as the actor reappears
		//  in fast area (still may need to pause when
		//  AnimationTracker is done.)
#ifdef WATCHACTOR
		if (item_num == watchactor)
			pout << "Animation ["
				 << Kernel::get_instance()->getFrameNum()
				 << "] ActorAnimProcess left fastarea; terminating"
				 << std::endl;
#endif
		terminate();
		return true;
	}

	bool result = true;
	if (repeatcounter == 0) {
		// next step:
		sint32 x,y,z;
		a->getLocation(x,y,z);
		result = tracker->stepFrom(x,y,z);
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
