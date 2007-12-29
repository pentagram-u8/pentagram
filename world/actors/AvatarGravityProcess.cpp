/*
Copyright (C) 2007 The Pentagram team

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

#include "AvatarGravityProcess.h"
#include "MainActor.h"
#include "World.h"
#include "GUIApp.h"
#include "CurrentMap.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include <cmath>


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarGravityProcess,GravityProcess);

AvatarGravityProcess::AvatarGravityProcess()
	: GravityProcess()
{

}

AvatarGravityProcess::AvatarGravityProcess(MainActor* avatar, int gravity)
	: GravityProcess(avatar, gravity)
{

}

void AvatarGravityProcess::run()
{
	if (!GUIApp::get_instance()->isMouseDown(BUTTON_RIGHT)) {
		// right mouse button not down, so fall normally

		GravityProcess::run();
		return;
	}

	// right mouse button down, so see if we can cling to a ledge
	MainActor* avatar = getMainActor();
	sint32 direction = avatar->getDir();
	if (avatar->tryAnim(Animation::climb40, direction) == Animation::SUCCESS) {

		// we can, so perform a hang animation
		// CHECKME: do we need to perform any other checks?

		if (avatar->getLastAnim() != Animation::hang)
			avatar->doAnim(Animation::hang, 8);

		return;
	} else {

		// fall normally
		GravityProcess::run();
		return;
	}
}


void AvatarGravityProcess::saveData(ODataSource* ods)
{
	GravityProcess::saveData(ods);
}

bool AvatarGravityProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!GravityProcess::loadData(ids, version)) return false;

	return true;
}
