/*
Copyright (C) 2003-2005 The Pentagram team

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

#include "TeleportEgg.h"
#include "MainActor.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"


DEFINE_RUNTIME_CLASSTYPE_CODE(TeleportEgg,Egg);

TeleportEgg::TeleportEgg()
{

}


TeleportEgg::~TeleportEgg()
{

}

uint16 TeleportEgg::hatch()
{
	if (!isTeleporter()) return 0; // teleport target

	// teleport to destination egg
	perr << "Teleport!!!!!!!!" << std::endl;

	MainActor* av = getMainActor();
	av->teleport(mapnum, getTeleportId());

	return 0;
}

void TeleportEgg::saveData(ODataSource* ods)
{
	Egg::saveData(ods);
}

bool TeleportEgg::loadData(IDataSource* ids, uint32 version)
{
	if (!Egg::loadData(ids, version)) return false;

	return true;
}
