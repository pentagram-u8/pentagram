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

#include "pent_include.h"

#include "LoiterProcess.h"
#include "Actor.h"
#include "PathfinderProcess.h"
#include "Kernel.h"
#include "DelayProcess.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(LoiterProcess,Process);

LoiterProcess::LoiterProcess() : Process()
{

}

LoiterProcess::LoiterProcess(Actor* actor_, sint32 c)
{
	assert(actor_);
	item_num = actor_->getObjId();
	count = c;

	type = 0x205; // CONSTANT!
}

void LoiterProcess::run()
{
	if (!count) {
		terminate();
		return;
	}
	if (count > 0)
		count--;

	Actor *a = getActor(item_num);

	if (!a || a->isDead()) {
		// dead?
		terminate();
		return;
	}

	sint32 x,y,z;
	a->getLocation(x,y,z);

	x += 32 * ((std::rand() % 20) - 10);
	y += 32 * ((std::rand() % 20) - 10);

	PathfinderProcess* pfp = new PathfinderProcess(a,x,y,z);
	Kernel::get_instance()->addProcess(pfp);

	bool hasidle1 = a->hasAnim(Animation::idle1);
	bool hasidle2 = a->hasAnim(Animation::idle2);

	if ((hasidle1 || hasidle2) && ((std::rand()%3) == 0))
	{
		Animation::Sequence idleanim;

		if (!hasidle1) {
			idleanim = Animation::idle2;
		} else if (!hasidle2) {
			idleanim = Animation::idle1;
		} else {
			if (std::rand()%2)
				idleanim = Animation::idle1;
			else
				idleanim = Animation::idle2;
		}
		uint16 idlepid = a->doAnim(idleanim, 8);
		Process* idlep = Kernel::get_instance()->getProcess(idlepid);
		idlep->waitFor(pfp);

		waitFor(idlep);

	} else {
		// wait 4-7 sec
		DelayProcess* dp = new DelayProcess(30 * (4 + (std::rand()%3)));
		Kernel::get_instance()->addProcess(dp);
		dp->waitFor(pfp);

		waitFor(dp);
	}
}

void LoiterProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(count);
}

bool LoiterProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	if (version >= 3)
		count = ids->read4();
	else
		count = 0; // default to loitering indefinitely

	return true;
}
