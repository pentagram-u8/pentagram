/*
Copyright (C) 2003-2005 The Pentagram team

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

#ifndef ACTORANIMPROCESS_H
#define ACTORANIMPROCESS_H

#include "Process.h"
#include "Animation.h"

class Actor;
struct AnimAction;
class AnimationTracker;
class Item;

class ActorAnimProcess : public Process
{
public:
	ActorAnimProcess();
	//! note: this probably needs some more parameters
	ActorAnimProcess(Actor* actor, Animation::Sequence action, uint32 dir,
					 uint32 steps=0);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	virtual void terminate();

	virtual void dumpInfo();

	Animation::Sequence getAction() const { return action; }

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	virtual bool init();

	//! perform special action for an animation
	void doSpecial();

	//! perform special action when hitting an opponent
	void doHitSpecial(Item* hit);

	Animation::Sequence action;
	uint32 dir;
	uint32 steps;

	AnimationTracker* tracker;
	int repeatcounter;
	uint32 currentstep;

	bool firstframe;

	bool animAborted;

	bool attackedSomething; // attacked and hit something with this animation
};


#endif
