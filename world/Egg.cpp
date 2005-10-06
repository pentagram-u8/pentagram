/*
Copyright (C) 2003-2004 The Pentagram team

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

#include "Egg.h"
#include "GUIApp.h"
#include "getObject.h"
#include "UCMachine.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(Egg,Item);

Egg::Egg() : hatched(false)
{

}


Egg::~Egg()
{

}

uint16 Egg::hatch()
{
	if (hatched) return 0;
	hatched = true;
	return callUsecodeEvent_hatch();
}

void Egg::dumpInfo()
{
	Item::dumpInfo();
	pout << "range: " << getXRange() << "," << getYRange()
		 << ", hatched=" << hatched << std::endl;
}

void Egg::leaveFastArea()
{
	reset();
	Item::leaveFastArea();
}

void Egg::saveData(ODataSource* ods)
{
	Item::saveData(ods);

	uint8 h = hatched ? 1 :  0;
	ods->write1(h);
}

bool Egg::loadData(IDataSource* ids, uint32 version)
{
	if (!Item::loadData(ids, version)) return false;

	hatched = (ids->read1() != 0);

	return true;
}

uint32 Egg::I_getEggXRange(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return static_cast<uint32>(egg->getXRange());
}

uint32 Egg::I_getEggYRange(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return static_cast<uint32>(egg->getYRange());
}

uint32 Egg::I_setEggXRange(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(xr);
	if (!egg) return 0;

	egg->setXRange(xr);
	return 0;
}

uint32 Egg::I_setEggYRange(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(yr);
	if (!egg) return 0;

	egg->setYRange(yr);
	return 0;
}

uint32 Egg::I_getEggId(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	if (!egg) return 0;

	return egg->getMapNum();
}

uint32 Egg::I_setEggId(const uint8* args, unsigned int /*argsize*/)
{
	ARG_EGG_FROM_PTR(egg);
	ARG_UINT16(eggid);
	if (!egg) return 0;

	egg->setMapNum(eggid);
	
	return 0;
}

