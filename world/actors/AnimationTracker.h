/*
Copyright (C) 2004-2007 The Pentagram team

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

#ifndef ANIMATIONTRACKER_H
#define ANIMATIONTRACKER_H

#include "Animation.h"
#include "Pathfinder.h"

class Actor;
class IDataSource;
class ODataSource;
struct AnimAction;
struct AnimFrame;

class AnimationTracker
{
public:
	AnimationTracker();
	~AnimationTracker();

	//! initialize the AnimationTracker for the given actor, action, dir
	//! if state is non-zero, start from that state instead of the Actor's
	//! current state
	bool init(Actor* actor, Animation::Sequence action, uint32 dir,
			  PathfindingState* state=0);

	//! evaluate the maximum distance the actor will travel if the current
	//! animation runs to completion by incremental calls to step
	void evaluateMaxAnimTravel(sint32& max_endx, sint32& max_endy, uint32 dir);

	//! do a single step of the animation
	//! returns true if everything ok, false if not
	//! caller must decide if animation should continue after a 'false'
	bool step();

	//! do a single step of the animation, starting at (x,y,z)
	//! returns true if everything ok, false if not
	//! caller must decide if animation should continue after a 'false'
	bool stepFrom(sint32 x, sint32 y, sint32 z);

	//! update the PathfindingState with latest coordinates and flags
	void updateState(PathfindingState& state);

	//! update the Actor with latest flags and animframe
	void updateActorFlags();

	//! get the current position
	void getPosition(sint32& x_, sint32& y_, sint32& z_)
		{ x_ = x; y_ = y; z_ = z; }

	void getInterpolatedPosition(sint32& x_, sint32& y_, sint32& z_, int fc);

	//! get the difference between current position and previous position
	void getSpeed(sint32& dx, sint32& dy, sint32& dz);

	//! get the current (shape)frame
	uint32 getFrame() { return shapeframe; }

	//! get the current AnimAction
	AnimAction* getAnimAction() { return animaction; }

	//! get the current AnimFrame
	AnimFrame* getAnimFrame();

	void setTargetedMode(sint32 x_, sint32 y_, sint32 z_);

	bool isDone() const { return done; }
	bool isBlocked() const { return blocked; }
	bool isUnsupported() const { return unsupported; }
	ObjId hitSomething() const { return hitobject; }

	bool load(IDataSource* ids, uint32 version);
	void save(ODataSource* ods);

private:
	enum Mode
	{
		NormalMode = 0,
		TargetMode
	};

	unsigned int getNextFrame(unsigned int frame);
	void checkWeaponHit();

	unsigned int startframe, endframe;
	bool firstframe;
	unsigned int currentframe;

	ObjId actor;
	unsigned int dir;

	AnimAction* animaction;

	// actor state
	sint32 prevx,prevy,prevz;
	sint32 x,y,z;
	sint32 startx,starty,startz;
	sint32 target_dx,target_dy,target_dz;
	sint32 target_offground_left;
	bool firststep, flipped;
	uint32 shapeframe;

	// status flags
	bool done;
	bool blocked;
	bool unsupported;
	ObjId hitobject;

	Mode mode;
};


#endif
