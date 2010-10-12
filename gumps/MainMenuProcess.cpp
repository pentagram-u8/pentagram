/*
Copyright (C) 2005-2010 The Pentagram team

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

#include "MainMenuProcess.h"
#include "MenuGump.h"
#include "getObject.h"
#include "MusicProcess.h"
#include "MainActor.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainMenuProcess,Process);

MainMenuProcess::MainMenuProcess() : Process()
{
	init = false;
}


void MainMenuProcess::run()
{
	MainActor* avatar = getMainActor();
	if (avatar && avatar->isDead()) {
		// stop death music
		MusicProcess::get_instance()->playCombatMusic(0);
	}

	MenuGump::showMenu();

	terminate();
}

void MainMenuProcess::saveData(ODataSource* ods)
{
	CANT_HAPPEN();

	Process::saveData(ods);
}

bool MainMenuProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
