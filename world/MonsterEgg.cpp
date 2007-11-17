/*
Copyright (C) 2003-2007 The Pentagram team

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

#include "MonsterEgg.h"
#include "UCMachine.h"
#include "Actor.h"
#include "ItemFactory.h"
#include "CurrentMap.h"
#include "ShapeInfo.h"
#include "MonsterInfo.h"
#include "World.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MonsterEgg,Item);

MonsterEgg::MonsterEgg()
{

}


MonsterEgg::~MonsterEgg()
{

}

uint16 MonsterEgg::hatch()
{
	//!! do we need to check probability here?
	//!! monster activity? combat? anything?

	int shape = getMonsterShape();

	// CHECKME: why does this happen? (in the plane of Earth near the end)
	if (shape == 0)
		return 0;

	Actor* newactor = ItemFactory::createActor(shape, 0, 0,
								FLG_FAST_ONLY|FLG_DISPOSABLE|FLG_IN_NPC_LIST, 
								0, 0, 0, true);
	if (!newactor) {
		perr << "MonsterEgg::hatch failed to create actor (" << shape
			 <<	")." << std::endl;
		return 0;
	}
	uint16 objID = newactor->getObjId();

	// set stats
	if (!newactor->loadMonsterStats()) {
		perr << "MonsterEgg::hatch failed to set stats for actor (" << shape
			 << ")." << std::endl;
	}

	if (!newactor->canExistAt(x,y,z)) {
		newactor->destroy();
		return 0;
	}

	// mapnum has to be set to the current map. Reason: Beren teleports to
	// newactor->getMapNum() when newactor is assaulted.
	newactor->setMapNum(World::get_instance()->getCurrentMap()->getNum());
	newactor->setNpcNum(objID);
	newactor->move(x,y,z);

	newactor->cSetActivity(getActivity());

	return objID;
}

void MonsterEgg::saveData(ODataSource* ods)
{
	Item::saveData(ods);
}

bool MonsterEgg::loadData(IDataSource* ids, uint32 version)
{
	if (!Item::loadData(ids, version)) return false;

	return true;
}

uint32 MonsterEgg::I_monsterEggHatch(const uint8*args,unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(item);
	if (!megg) return 0;

	return megg->hatch();
}

uint32 MonsterEgg::I_getMonId(const uint8*args,unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(item);
	if (!megg) return 0;

	return megg->getMapNum() >> 3;
}
