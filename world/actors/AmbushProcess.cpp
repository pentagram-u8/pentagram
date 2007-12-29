/*
Copyright (C) 2006 The Pentagram team

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

#include "AmbushProcess.h"
#include "MainActor.h"
#include "CombatProcess.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AmbushProcess,Process);

AmbushProcess::AmbushProcess() : Process()
{

}

AmbushProcess::AmbushProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();
	type = 0x21E; // CONSTANT !

	delaycount = 0;
}

void AmbushProcess::run()
{
	if (delaycount > 0) {
		delaycount--;
		return;
	}
	delaycount = 10;

	Actor *a = getActor(item_num);
	CombatProcess* cp = a->getCombatProcess();
	if (!cp) {
		// this shouldn't have happened
		terminate();
		return;
	}

	ObjId targetid = cp->seekTarget();
	Item* target = getItem(targetid);

	// no target in range yet, continue waiting
	if (!target || a->getRange(*target) > 192)
		return;

	// target in range, so terminate and let parent take over
	terminate();
}

void AmbushProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(delaycount);
}

bool AmbushProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	delaycount = ids->read4();

	return true;
}
