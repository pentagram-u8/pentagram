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

#include "DeleteActorProcess.h"
#include "World.h"
#include "Actor.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(DeleteActorProcess,Process);

DeleteActorProcess::DeleteActorProcess() : Process()
{

}

DeleteActorProcess::DeleteActorProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0; // FIXME!!
}

bool DeleteActorProcess::run(const uint32 /*framenum*/)
{
	Actor *a = World::get_instance()->getNPC(item_num);

	if (!a) {
		// somebody did our work for us...
		terminate();
		return false;
	}

	if (!(a->getActorFlags() & Actor::ACT_DEAD)) {
		// not dead?
		terminate();
		return false;
	}

	// FIXME: should probably prevent player from opening gump in the
	// first place...
	if (a->getFlags() & Item::FLG_GUMP_OPEN) {
		// first close gump in case player is still rummaging through us
		a->closeGump();
	}

	// bye bye
	// (note that Container::destroy() calls removeContents())
	a->destroy();

	// NOTE: we're terminated here because this process belongs to the actor

	return true;
}

void DeleteActorProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);
}

bool DeleteActorProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
