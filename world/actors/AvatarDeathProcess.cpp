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

#include "AvatarDeathProcess.h"
#include "World.h"
#include "MainActor.h"
#include "GUIApp.h"
#include "ReadableGump.h"
#include "GameData.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarDeathProcess,Process);

AvatarDeathProcess::AvatarDeathProcess() : Process()
{
	item_num = 1;
	type = 1; // CONSTANT !
}

bool AvatarDeathProcess::run(const uint32 framenum)
{
	MainActor *av = World::get_instance()->getMainActor();

	if (!av) {
		perr << "AvatarDeathProcess: MainActor object missing" << std::endl;
		// avatar gone??
		terminate();
		return false;
	}

	if (!(av->getActorFlags() & Actor::ACT_DEAD)) {
		perr << "AvatarDeathProcess: MainActor not dead" << std::endl;
		// avatar not dead?
		terminate();
		return false;
	}

	Gump *desktop = GUIApp::get_instance()->getDesktopGump();
	Gump *gump = new ReadableGump(1, 27, 11,
								  _TL_("HERE LIES*THE AVATAR*REST IN PEACE"));
	gump->InitGump();
	desktop->AddChild(gump);
	gump->setRelativePosition(Gump::CENTER);

	// done
	terminate();

	return true;
}

void AvatarDeathProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);
}

bool AvatarDeathProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	return true;
}
