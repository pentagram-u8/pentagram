/*
Copyright (C) 2004 The Pentagram team

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
#include "AnimationTracker.h"

#include "GameData.h"
#include "Actor.h"
#include "World.h"
#include "CurrentMap.h"
#include "MainShapeFlex.h"
#include "AnimAction.h"
#include "Direction.h"
#include "ShapeInfo.h"

#include "IDataSource.h"
#include "ODataSource.h"

AnimationTracker::AnimationTracker()
{

}

AnimationTracker::AnimationTracker(Actor* actor_, Animation::Sequence action_,
								   uint32 dir_, PathfindingState* state_)
{
	init(actor_, action_, dir_, state_);
}

AnimationTracker::~AnimationTracker()
{

}

void AnimationTracker::init(Actor* actor_, Animation::Sequence action_,
							uint32 dir_, PathfindingState* state_)
{
	assert(actor_);
	actor = actor_->getObjId();
	uint32 shape = actor_->getShape();
	animaction = GameData::get_instance()->getMainShapes()->
		getAnim(shape, action_);

	dir = dir_;

	if (state_ == 0) {
		animaction->getAnimRange(actor_, dir, startframe, endframe);
		actor_->getLocation(x, y, z);
		flipped = (actor_->getFlags() & Item::FLG_FLIPPED);
		firststep = (actor_->getActorFlags() & Actor::ACT_FIRSTSTEP);
	} else {
		animaction->getAnimRange(state_->lastanim, state_->direction,
								 state_->firststep, dir, startframe, endframe);
		flipped = state_->flipped;
		firststep = state_->firststep;
		x = state_->x;
		y = state_->y;
		z = state_->z;
	}

	firstframe = true;

	done = false;
	blocked = false;
	unsupported = false;
}

bool AnimationTracker::step()
{
	if (done) return false;

	prevx = x;
	prevy = y;
	prevz = z;

	// reset status flags
	unsupported = false;
	blocked = false;

	Actor* a = World::get_instance()->getNPC(actor);
	assert(a);

	if (firstframe)
		currentframe = startframe;
	else
		currentframe++;

	if (currentframe == endframe) {
		done = true;

		// toggle ACT_FIRSTSTEP flag if necessary
		if (animaction->flags & AnimAction::AAF_TWOSTEP)
			firststep = !firststep;
		else
			firststep = true;

		return false;
	}

	firstframe = false;

	// loop if necessary
	if (currentframe >= animaction->size) {
		if (animaction->flags & AnimAction::AAF_LOOPING) {
			currentframe = 1;
		} else {
			currentframe = 0;
		}
	}

	AnimFrame& f = animaction->frames[dir][currentframe];

	shapeframe = f.frame;
	flipped = f.is_flipped();

	// determine movement for this frame
	sint32 dx = 4 * x_fact[dir] * f.deltadir;
	sint32 dy = 4 * y_fact[dir] * f.deltadir;
	sint32 dz = f.deltaz;

	// determine footpad
	bool actorflipped = (a->getFlags() & Item::FLG_FLIPPED);
	sint32 xd, yd, zd;
	a->getFootpadWorld(xd, yd, zd);
	if (actorflipped != flipped) {
		sint32 t = xd;
		xd = yd;
		yd = t;
	}
	CurrentMap* cm = World::get_instance()->getCurrentMap();

	// TODO: check if this step is allowed
	// * can move?
	//   if not:
	//     - try to step up a bit
	//     - try to shift left/right a bit
	//     CHECKME: how often can we do these minor adjustments?
	//     CHECKME: for which animation types can we do them?
	//   if still fails: blocked
	// * if ONGROUND
	//     - is supported if ONGROUND?
	//       if not:
	//         * try to step down a bit
	//         * try to shift left/right a bit
	//       if still fails: unsupported
	//     - if supported by non-land item: unsupported

	// It might be worth it creating a 'scanForValidPosition' function
	// (in CurrentMap maybe) that scans a small area around the given
	// coordinates for a valid position (with 'must be supported' as a flag).

	// I seem to recall that the teleporter from the Upper Catacombs teleporter
	// to the Upper Catacombs places you inside the floor. Using this
	// scanForValidPosition after a teleport would work around that problem.

	ObjId support;
	bool targetok = cm->isValidPosition(x+dx,y+dy,z+dz, xd,yd,zd,
										actor, &support, 0);

	if (!targetok) {
		// TODO: try to adjust properly...
		// for now just a hack to try to step up 8

		targetok = cm->isValidPosition(x+dx,y+dy,z+dz+8, xd,yd,zd,
									   actor, 0, 0);

		if (targetok) {
			dz+=8;
		} else {
			blocked = true;
			return false;
		}
	}

	x += dx;
	y += dy;
	z += dz;

	if (f.flags & AnimFrame::AFF_ONGROUND) {
		// needs support

		if (!support) {
			// TODO: try to adjust...
			// for now just a hack to try to step down 8

			targetok = cm->isValidPosition(x,y,z-8, xd,yd,zd,
										   actor, &support, 0);

			if (targetok && support) {
				z -= 8;
			} else {
				unsupported = true;
				return false;
			}
		} else {
			Item* supportitem = World::get_instance()->getItem(support);
			assert(supportitem);
			if (!supportitem->getShapeInfo()->is_land()) {
				// invalid support
				unsupported = true;
				return false;
			}
		}
	}

	return true;
}

AnimFrame* AnimationTracker::getAnimFrame()
{
	return &animaction->frames[dir][currentframe];
}

void AnimationTracker::updateState(PathfindingState& state)
{
	state.x = x;
	state.y = y;
	state.z = z;
	state.flipped = flipped;
	state.firststep = firststep;
}


void AnimationTracker::updateActorFlags()
{
	Actor* a = World::get_instance()->getNPC(actor);
	assert(a);

	if (flipped)
		a->setFlag(Item::FLG_FLIPPED);
	else
		a->clearFlag(Item::FLG_FLIPPED);

	if (firststep)
		a->setActorFlag(Actor::ACT_FIRSTSTEP);
	else
		a->clearActorFlag(Actor::ACT_FIRSTSTEP);

	a->animframe = currentframe;
}

void AnimationTracker::getInterpolatedPosition(sint32& x_, sint32& y_,
											   sint32& z_, int fc)
{
	sint32 dx = x - prevx;
	sint32 dy = y - prevy;
	sint32 dz = z - prevz;

	x_ = prevx + (dx*fc)/animaction->framerepeat;
	y_ = prevy + (dy*fc)/animaction->framerepeat;
	z_ = prevz + (dz*fc)/animaction->framerepeat;
}

void AnimationTracker::save(ODataSource* ods)
{
	ods->write2(1); //version

	ods->write4(startframe);
	ods->write4(endframe);
	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	ods->write4(currentframe);

	ods->write2(actor);
	ods->write1(static_cast<uint8>(dir));

	if (animaction) {
		ods->write4(animaction->shapenum);
		ods->write4(animaction->action);
	} else {
		ods->write4(0);
		ods->write4(0);
	}

	ods->write4(static_cast<uint32>(prevx));
	ods->write4(static_cast<uint32>(prevy));
	ods->write4(static_cast<uint32>(prevz));
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));
	uint8 fs = firststep ? 1 : 0;
	ods->write1(fs);
	uint8 fl = flipped ? 1 : 0;
	ods->write1(fl);
	ods->write4(shapeframe);

	uint8 flag = done ? 1 : 0;
	ods->write1(flag);
	flag = blocked ? 1 : 0;
	ods->write1(flag);
	flag = unsupported ? 1 : 0;
	ods->write1(flag);
}

bool AnimationTracker::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	startframe = ids->read4();
	endframe = ids->read4();
	firstframe = (ids->read1() != 0);
	currentframe = ids->read4();

	actor = ids->read2();
	dir = ids->read1();

	uint32 shapenum = ids->read4();
	uint32 action = ids->read4();
	if (shapenum == 0) {
		animaction = 0;
	} else {
		animaction = GameData::get_instance()->getMainShapes()->
			getAnim(shapenum, action);
		assert(animaction);
	}

	prevx = ids->read4();
	prevy = ids->read4();
	prevz = ids->read4();
	x = ids->read4();
	y = ids->read4();
	z = ids->read4();
	firststep = (ids->read1() != 0);
	flipped = (ids->read1() != 0);
	shapeframe = ids->read4();

	done = (ids->read1() != 0);
	blocked = (ids->read1() != 0);
	unsupported = (ids->read1() != 0);

	return true;
}
