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

#include "ClearFeignDeathProcess.h"
#include "World.h"
#include "Actor.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ClearFeignDeathProcess,Process);

ClearFeignDeathProcess::ClearFeignDeathProcess() : Process()
{

}

ClearFeignDeathProcess::ClearFeignDeathProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x243; // constant !
}

bool ClearFeignDeathProcess::run(const uint32 /*framenum*/)
{
	Actor *a = World::get_instance()->getNPC(item_num);

	if (!a) {
		// actor gone?
		terminate();
		return false;
	}

	a->clearActorFlag(Actor::ACT_FEIGNDEATH);

	// TODO: play sound


	// done
	terminate();

	return true;
}

void ClearFeignDeathProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);
}

bool ClearFeignDeathProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	return true;
}
