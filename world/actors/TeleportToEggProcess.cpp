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

#include "TeleportToEggProcess.h"
#include "MainActor.h"
#include "World.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(TeleportToEggProcess,Process);

TeleportToEggProcess::TeleportToEggProcess() : Process()
{

}


TeleportToEggProcess::TeleportToEggProcess(int mapnum_, int teleport_id_)
	: mapnum(mapnum_), teleport_id(teleport_id_)
{
	type = 1; // CONSTANT! (type 1 = persistent)
}


bool TeleportToEggProcess::run(const uint32 framenum)
{
	MainActor *av = World::get_instance()->getMainActor();
	
	av->teleport(mapnum, teleport_id);
	
	terminate();
	return true;
}

void TeleportToEggProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);
	
	ods->write4(static_cast<uint32>(mapnum));
	ods->write4(static_cast<uint32>(teleport_id));
}

bool TeleportToEggProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;
	
	mapnum = static_cast<int>(ids->read4());
	teleport_id = static_cast<int>(ids->read4());
	return true;
}
