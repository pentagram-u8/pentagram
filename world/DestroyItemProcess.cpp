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

#include "DestroyItemProcess.h"
#include "World.h"
#include "Actor.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(DestroyItemProcess,Process);

DestroyItemProcess::DestroyItemProcess() : Process()
{

}

DestroyItemProcess::DestroyItemProcess(Item* item_)
{
	assert(item_);
	item_num = item_->getObjId();

	type = 0; // FIXME!!
}

bool DestroyItemProcess::run(const uint32 /*framenum*/)
{
	Item *it = World::get_instance()->getItem(item_num);

	if (!it) {
		// somebody did our work for us...
		terminate();
		return false;
	}

	// FIXME: should probably prevent player from opening gump in the
	// first place...
	if (it->getFlags() & Item::FLG_GUMP_OPEN) {
		// first close gump in case player is still rummaging through us
		it->closeGump();
	}

	// bye bye
	// (note that Container::destroy() calls removeContents())
	it->destroy(true);

	// NOTE: we're terminated here because this process belongs to the item

	return true;
}

void DestroyItemProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);
}

bool DestroyItemProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
