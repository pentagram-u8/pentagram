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
#include "PathfinderProcess.h"

#include "Actor.h"
#include "World.h"
#include "Pathfinder.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(PathfinderProcess,Process);

PathfinderProcess::PathfinderProcess() : Process()
{

}

PathfinderProcess::PathfinderProcess(Actor* actor_,
									 sint32 x, sint32 y, sint32 z)
{
	assert(actor_);
	item_num = actor_->getObjId();

	targetx = x;
	targety = y;
	targetz = z;

	currentstep = 0;

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(targetx, targety, targetz);

	bool ok = pf.pathfind(path);

	if (!ok) {
		perr << "PathfinderProcess: failed to find path" << std::endl;
		// can't get there...
		result = 1;
		terminate();
		return;
	}
}

PathfinderProcess::~PathfinderProcess()
{

}

bool PathfinderProcess::run(const uint32 framenum)
{
	if (currentstep >= path.size()) {
		// done
		perr << "PathfinderProcess: done" << std::endl;
		result = 0;
		terminate();
		return false;
	}

	// try to take the next step

	perr << "PathfinderProcess: trying step" << std::endl;

	Actor* actor = World::get_instance()->getNPC(item_num);
	bool ok = actor->tryAnim(path[currentstep].action,
							 path[currentstep].direction);

	if (!ok) {
		perr << "PathfinderProcess: recalculating path" << std::endl;

		// need to redetermine path
		Pathfinder pf;
		pf.init(actor);
		pf.setTarget(targetx, targety, targetz);
		ok = pf.pathfind(path);
		currentstep = 0;
		if (!ok) {
			perr << "PathfinderProcess: failed to find path" << std::endl;
			// can't get there anymore
			result = 1;
			terminate();
			return false;
		}
	}

	if (currentstep >= path.size()) {
		perr << "PathfinderProcess: done" << std::endl;
		// done
		result = 0;
		terminate();
		return false;
	}

	uint16 animpid = actor->doAnim(path[currentstep].action,
								   path[currentstep].direction);
	currentstep++;
	perr << "PathfinderProcess(" << getPid() << "): taking step (pid="
		 << animpid << ")" << std::endl;

	waitFor(animpid);
	return true;
}

void PathfinderProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write2(static_cast<uint16>(targetx));
	ods->write2(static_cast<uint16>(targety));
	ods->write2(static_cast<uint16>(targetz));
	ods->write2(static_cast<uint16>(currentstep));

	ods->write2(static_cast<uint16>(path.size()));
	for (unsigned int i = 0; i < path.size(); ++i) {
		ods->write2(static_cast<uint16>(path[i].action));
		ods->write2(static_cast<uint16>(path[i].direction));
	}
}

bool PathfinderProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	targetx = ids->read2();
	targety = ids->read2();
	targetz = ids->read2();
	currentstep = ids->read2();

	unsigned int pathsize = ids->read2();
	path.resize(pathsize);
	for (unsigned int i = 0; i < pathsize; ++i) {
		path[i].action = ids->read2();
		path[i].direction = ids->read2();
	}

	return true;
}
