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

#include "LoiterProcess.h"
#include "World.h"
#include "Actor.h"
#include "PathfinderProcess.h"
#include "Kernel.h"
#include "DelayProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(LoiterProcess,Process);

LoiterProcess::LoiterProcess() : Process()
{

}

LoiterProcess::LoiterProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x205; // CONSTANT!
}

bool LoiterProcess::run(const uint32 framenum)
{
	Actor *a = World::get_instance()->getNPC(item_num);

	if (!a || a->getActorFlags() & Actor::ACT_DEAD) {
		// dead?
		terminate();
		return false;
	}

	sint32 x,y,z;
	a->getLocation(x,y,z);

	x += 32 * ((std::rand() % 20) - 10);
	y += 32 * ((std::rand() % 20) - 10);

	PathfinderProcess* pfp = new PathfinderProcess(a,x,y,z);
	Kernel::get_instance()->addProcess(pfp);

	DelayProcess* dp = new DelayProcess(30);
	Kernel::get_instance()->addProcess(dp);
	dp->waitFor(pfp);

	waitFor(dp);
	

	return false;
}

void LoiterProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);
}

bool LoiterProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	return true;
}
