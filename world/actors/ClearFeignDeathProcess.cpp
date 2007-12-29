/*
Copyright (C) 2004-2005 The Pentagram team

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
#include "Actor.h"
#include "AudioProcess.h"
#include "getObject.h"

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

void ClearFeignDeathProcess::run()
{
	Actor *a = getActor(item_num);

	if (!a) {
		// actor gone?
		terminate();
		return;
	}

	a->clearActorFlag(Actor::ACT_FEIGNDEATH);

	AudioProcess* audioproc = AudioProcess::get_instance();
	if (audioproc) audioproc->playSFX(59, 0x60, item_num, 0);

	// done
	terminate();
}

void ClearFeignDeathProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);
}

bool ClearFeignDeathProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
