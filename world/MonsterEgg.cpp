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

#include "MonsterEgg.h"
#include "Kernel.h"
#include "World.h"
#include "UCMachine.h"
#include "Actor.h"
#include "ItemFactory.h"
#include "CurrentMap.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MonsterEgg,Egg);

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
	//!! maybe mark actor as temporary

	int shape = getMonsterShape();

	Actor* newactor = ItemFactory::createActor(shape, 0, 0,
											   Item::FLG_MONSTER_NPC, 0, 0,
											   Item::EXT_NOTINMAP);
	if (!newactor) {
		perr << "MonsterEgg::hatch failed to create actor (" << shape
			 <<	")." << std::endl;
		return 0;
	}
	uint16 objID = newactor->assignObjId();

	newactor->setMapNum(getMapNum());
	newactor->setNpcNum(objID);
	newactor->setLocation(x,y,z);
	World::get_instance()->getCurrentMap()->addItem(newactor);

	return objID;
}

uint32 MonsterEgg::I_monsterEggHatch(const uint8*args,unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(egg);
	if (!megg) return 0;

	return megg->hatch();
}

uint32 MonsterEgg::I_getMonId(const uint8*args,unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(egg);
	if (!megg) return 0;

	return megg->getMapNum() >> 3;
}
