/*
Copyright (C) 2004-2010 The Pentagram team

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
#include "MainActor.h"
#include "ReadableGump.h"
#include "GameData.h"
#include "Kernel.h"
#include "MainMenuProcess.h"
#include "GumpNotifyProcess.h"
#include "MusicProcess.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarDeathProcess,Process);

AvatarDeathProcess::AvatarDeathProcess() : Process()
{
	item_num = 1;
	type = 1; // CONSTANT !
}

void AvatarDeathProcess::run()
{
	MainActor *av = getMainActor();

	if (!av) {
		perr << "AvatarDeathProcess: MainActor object missing" << std::endl;
		// avatar gone??
		terminate();
		return;
	}

	if (!(av->getActorFlags() & Actor::ACT_DEAD)) {
		perr << "AvatarDeathProcess: MainActor not dead" << std::endl;
		// avatar not dead?
		terminate();
		return;
	}

	ReadableGump *gump = new ReadableGump(1, 27, 11,
								  _TL_("HERE LIES*THE AVATAR*REST IN PEACE"));
	gump->InitGump(0);
	gump->setRelativePosition(Gump::CENTER);
	Process* gumpproc = gump->GetNotifyProcess();

	Process* menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);
	menuproc->waitFor(gumpproc);

	// done
	terminate();
}

void AvatarDeathProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);
}

bool AvatarDeathProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
