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
#include "World.h"
#include "UCMachine.h"

DEFINE_DYNAMIC_CAST_CODE(MonsterEgg,Egg);

MonsterEgg::MonsterEgg()
{

}


MonsterEgg::~MonsterEgg()
{

}

uint16 MonsterEgg::hatch()
{
	// create monster...
	return 0;
}

uint32 MonsterEgg::I_monsterEggHatch(const uint8*args,unsigned int /*argsize*/)
{
	ARG_EGG(egg);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(egg);
	if (!megg) return 0;

	return megg->hatch();
}

uint32 MonsterEgg::I_getMonId(const uint8*args,unsigned int /*argsize*/)
{
	ARG_EGG(egg);
	MonsterEgg* megg = p_dynamic_cast<MonsterEgg*>(egg);
	if (!megg) return 0;

	return megg->getMapNum() >> 3;
}
