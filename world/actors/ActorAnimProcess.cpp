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

#include "ActorAnimProcess.h"
#include "GameData.h"
#include "MainShapeFlex.h"
#include "AnimDat.h"
#include "AnimAction.h"
#include "Actor.h"
#include "Direction.h"
#include "World.h"

#include "IDataSource.h"
#include "ODataSource.h"

//#define WATCHACTOR 19

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;

#include "GUIApp.h"
#endif

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ActorAnimProcess,Process);

ActorAnimProcess::ActorAnimProcess() : Process()
{

}

ActorAnimProcess::ActorAnimProcess(Actor* actor_, uint32 action, uint32 dir_)
{
	assert(actor_);
	item_num = actor_->getObjId();
	dir = dir_;

	type = 0x00F0; // CONSTANT !

	uint32 shape = actor_->getShape();
	animaction = GameData::get_instance()->getMainShapes()->
		getAnim(shape, action);

	if (dir > 7) {
		// invalid direction
		animaction = 0;
	}

	currentindex = 0;
	firstframe = true;

	if (actor_->getActorFlags() & Actor::ACT_ANIMLOCK) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		// for now, just don't play this one.
		animaction = 0;
	}

	if (!animaction) return;

	uint32 startframe = 0;

#if 1
	if (item_num == 5)
		animaction->framerepeat = 2; // force Darion to 2 frames
#endif

	actor_->setActorFlag(Actor::ACT_ANIMLOCK);

	if (animaction->flags & AnimAction::AAF_TWOSTEP) {
		// two-step animation?
		if (actor_->getActorFlags() & Actor::ACT_FIRSTSTEP) {
			if (animaction->flags & AnimAction::AAF_LOOPING) {
				// for a looping animation, start at the end to
				// make things more fluid
				startframe = animaction->size - 1;
			} else {
				startframe = 0;
			}
		} else {
			// second step starts halfway
			startframe = animaction->size / 2;
		}
	} else {
		if (actor_->lastanim == action && actor_->direction == dir_ &&
			animaction->size > 1)
		{
			// skip first frame if repeating an animation
			startframe = 1;
		}
	}

	actor_->lastanim = action;
	actor_->direction = dir_;

	currentindex = startframe * animaction->framerepeat;

#ifdef WATCHACTOR
	if (item_num == watchactor)
		perr << "Animation [" << GUIApp::get_instance()->getFrameNum()
			 << "] ActorAnimProcess created (" <<action << "," << dir_ << ")"
			 << std::endl;
#endif
}

bool ActorAnimProcess::run(const uint32 framenum)
{
	if (!animaction) {
		// non-existant animation
		terminate();
		return false;
	}

	Actor *a = World::get_instance()->getNPC(item_num);

	if (!a) {
		// actor gone
		terminate();
		return false;
	}

	if (!firstframe) {
		currentindex++;
	} else {
		firstframe = false;
	}

	unsigned int frameindex = currentindex / animaction->framerepeat;
	unsigned int framecount = currentindex % animaction->framerepeat;

	// check if we're done
	if (framecount == 0) {
		bool done = false;

		if (animaction->flags & AnimAction::AAF_TWOSTEP) {
			// two-step animation
			if (a->getActorFlags() & Actor::ACT_FIRSTSTEP) {
				// end of first step is halfway through the animation
				//! TODO: check if this causes problem with extremely
				//        short anims
				
				if (frameindex == animaction->size/2)
				{
					a->clearActorFlag(Actor::ACT_FIRSTSTEP);
					done = true;
				} else {
					// loop if necessary
					if (frameindex >= animaction->size) {
						if (animaction->flags & AnimAction::AAF_LOOPING) {
							frameindex = 1;
						} else {
							frameindex = 0;
						}
					}
				}
			} else {
				if (animaction->flags & AnimAction::AAF_LOOPING) {
					// end of second step is one frame before the end
					if (frameindex == animaction->size - 1) {
						a->setActorFlag(Actor::ACT_FIRSTSTEP);
						done = true;
					}
				}
			}
		}

		if (!done && frameindex >= animaction->size) {
			a->setActorFlag(Actor::ACT_FIRSTSTEP);
			done = true;
		}
		
		if (done) {
#ifdef WATCHACTOR
			if (item_num == watchactor)
				perr << "Animation [" << GUIApp::get_instance()->getFrameNum()
					 << "] ActorAnimProcess terminating" << std::endl;
#endif		 
			terminate();
			return true;
		}

		currentindex = frameindex * animaction->framerepeat;
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

	int fc = static_cast<int>(framecount);

	x -= (dx*fc)/animaction->framerepeat;
	y -= (dy*fc)/animaction->framerepeat;
	z -= (dz*fc)/animaction->framerepeat;

	x += (dx*(fc+1))/animaction->framerepeat;
	y += (dy*(fc+1))/animaction->framerepeat;
	z += (dz*(fc+1))/animaction->framerepeat;

//	a->move(x,y,z);
	a->collideMove(x, y, z, false, false);
	a->setFrame(f.frame);

	if (f.is_flipped()) {
		a->setFlag(Item::FLG_FLIPPED);
	} else {
		a->clearFlag(Item::FLG_FLIPPED);
	}

	if (framecount == 0) {
#ifdef WATCHACTOR
		if (item_num == watchactor)
			perr << "Animation [" << GUIApp::get_instance()->getFrameNum()
				 << "] showing new frame (" << frameindex << "/"
				 << animaction->size << ")" << std::endl;
#endif
	}

	return true;
}


void ActorAnimProcess::terminate()
{
	Actor *a = World::get_instance()->getNPC(item_num);
	if (a) {
		if (animaction) { // if we were really animating...
			a->clearActorFlag(Actor::ACT_ANIMLOCK);
		}
	}

	Process::terminate();
}


void ActorAnimProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	ods->write4(dir);
	ods->write4(currentindex);
	if (animaction) {
		ods->write4(animaction->shapenum);
		ods->write4(animaction->action);
	} else {
		ods->write4(0);
		ods->write4(0);
	}
}

bool ActorAnimProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	firstframe = (ids->read1() != 0);
	dir = ids->read4();
	currentindex = ids->read4();

	uint32 shapenum = ids->read4();
	uint32 action = ids->read4();

	if (shapenum == 0) {
		animaction = 0;
	} else {
		animaction = GameData::get_instance()->getMainShapes()->
			getAnim(shapenum, action);
	}

	return true;
}
