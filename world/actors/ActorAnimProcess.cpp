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

//#define WATCHACTOR 5

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

	if (actor_->getActorFlags() & Actor::ACT_ANIMLOCK) {
		//! What do we do if actor was already animating?
		//! don't do this animation or kill the previous one?
		//! Or maybe wait until the previous one finishes?

		// for now, just don't play this one.
		animaction = 0;
	}

#ifndef ANIMATIONINTERRUPT
	if (item_num == 5)
		animaction->framerepeat = 2; // force Darion to 2 frames
#endif

	if (animaction) {
		actor_->setActorFlag(Actor::ACT_ANIMLOCK);

#ifdef ANIMATIONINTTERUPT
		if (animaction->unk1 == 5) {
			if (actor_->lastanim == action && actor_->direction == dir_) {
				currentindex = (actor_->lastframe+1) * animaction->framerepeat;
				// resume previous animation

				if (currentindex >= animaction->size * animaction->framerepeat)
					currentindex = 0;
			}
		} else {
#endif
			if (actor_->lastanim == action && actor_->direction == dir_ &&
				animaction->size > 1)
			{
				currentindex+=animaction->framerepeat; // skip first frame if 
				                                      // repeating an animation
			}
#ifdef ANIMATIONINTERRUPT
		}
#endif
		actor_->lastanim = action;
		actor_->direction = dir_;
	}

#ifdef WATCHACTOR
	if (item_num == watchactor)
		perr << "Animation [" << GUIApp::get_instance()->getFrameNum()
			 << "] ActorAnimProcess created" << std::endl;
#endif
}

bool ActorAnimProcess::run(const uint32 framenum)
{
	if (!animaction) {
		// non-existant animation
		terminate();
		return false;
	}
	//!! this needs timing
	// without main timing I can't really determine how many 'ticks' 
	// each frame should last.

	int frameindex = currentindex / animaction->framerepeat;
	int framecount = currentindex % animaction->framerepeat;

	AnimFrame& f = animaction->frames[dir][frameindex];
	Actor *a = World::get_instance()->getNPC(item_num);

	if (!a) {
		// actor gone
		terminate();
		return false;
	}

	// Adding position works like this
	// subtract how much we've already added
	// then add how much we want. This is to get around issues
	// with accuracy

	sint32 x, y, z;
	sint32 dx, dy, dz;
	a->getLocation(x,y,z);

#ifdef ANIMATIONINTERRUPT
	dx = 4 * x_fact[dir] * f.deltadir;
	dy = 4 * y_fact[dir] * f.deltadir;
	dz = f.deltaz;
#else
	dx = 2 * x_fact[dir] * f.deltadir;
	dy = 2 * y_fact[dir] * f.deltadir;
	dz = f.deltaz;
#endif

	x -= (dx*framecount)/animaction->framerepeat;
	y -= (dy*framecount)/animaction->framerepeat;
	z -= (dz*framecount)/animaction->framerepeat;

	x += (dx*(framecount+1))/animaction->framerepeat;
	y += (dy*(framecount+1))/animaction->framerepeat;
	z += (dz*(framecount+1))/animaction->framerepeat;

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
				 << "] showing new frame" << std::endl;
#endif
	}

#ifdef ANIMATIONINTTERUPT
	if (framecount == animaction->framerepeat - 1) {
		if (animaction->unk1 == 5) {
			a->lastframe = frameindex;
//			perr << std::hex << f.flags << std::dec << std::endl;
			if (f.flags & 0x800) {
				terminate();
			}
		}
	}
#endif

	currentindex++;

	if (currentindex >= animaction->size * animaction->framerepeat) {
#ifdef ANIMATIONINTTERUPT
		if (animaction->unk1 == 5) {
			currentindex = 0;
		} else {
#endif
#ifdef WATCHACTOR
			if (item_num == watchactor)
				perr << "Animation [" << GUIApp::get_instance()->getFrameNum()
					 << "] ActorAnimProcess terminating" << std::endl;
#endif		 
			terminate();
#ifdef ANIMATIONINTTERUPT
		}
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
