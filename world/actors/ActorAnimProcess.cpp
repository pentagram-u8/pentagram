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

ActorAnimProcess::ActorAnimProcess(Actor* actor_, uint32 action, uint32 dir_)
{
	assert(actor_);
	actor = actor_;
	dir = dir_;

	//! We probably want to mark the actor as being animated somewhere
	//! possibly add a pointer to this process to the actor to allow
	//! for terminating the anim


	uint32 shape = actor->getShape();
	animaction = GameData::get_instance()->getMainShapes()->
		getAnim(shape, action);

	currentindex = 0;
}

bool ActorAnimProcess::run(const uint32 framenum)
{
	if (!animaction) {
		terminate();
		return false;
	}

	//!! this needs timing
	// without main timing I can't really determine how many 'ticks' 
	// each frame should last.

	AnimFrame& f = animaction->frames[dir][currentindex];

	sint32 x, y, z;
	actor->getLocation(x,y,z);

	x += 4 * x_fact[dir] * f.deltadir;
	y += 4 * y_fact[dir] * f.deltadir;
	z += f.deltaz;

	actor->setLocation(x,y,z);
	actor->setFrame(f.frame);

	if (f.is_flipped()) {
		actor->setFlag(Item::FLG_FLIPPED);
	} else {
		actor->clearFlag(Item::FLG_FLIPPED);
	}

	currentindex++;

	if (currentindex >= animaction->size) {
		//? do we need to terminate now or when we're about to show the next
		// frame?
		terminate();
	}



	return true;
}
