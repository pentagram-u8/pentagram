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
#include "HIDBinding.h"
#include "u8bindings.h"

#include "GUIApp.h"
#include "MainActor.h"
#include "World.h"
#include "UCList.h"
#include "Egg.h"
#include "ObjectManager.h"
#include "LoopScript.h"
#include "CameraProcess.h"
#include "CurrentMap.h"
#include "ShapeViewerGump.h"

namespace HIDBindings {

bool openBackpack(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		MainActor* av = World::get_instance()->getMainActor();
		Item* backpack = World::get_instance()->getItem(av->getEquip(7));
		if (backpack)
			backpack->callUsecodeEvent_use();
	}
	break;

	}
	return handled;
}

bool openInventory(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		MainActor* av = World::get_instance()->getMainActor();
		av->callUsecodeEvent_use();
	}
	break;

	}
	return handled;
}

bool recall(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		LOOPSCRIPT(script, LS_SHAPE_EQUAL(833)); // constant...
		MainActor* av = World::get_instance()->getMainActor();
		UCList uclist(2);
		av->containerSearch(&uclist, script, sizeof(script), true);
		if (uclist.getSize() < 1) {
			perr << "No recall item found!" << std::endl;
			break;
		}
		uint16 objid = uclist.getuint16(0);
		Item* item = World::get_instance()->getItem(objid);
		item->callUsecodeEvent_use();
	}
	break;

	}
	return handled;
}

bool runExecutionEgg(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		CurrentMap* currentmap = World::get_instance()->getCurrentMap();
		UCList uclist(2);
		// (shape == 73 && quality == 4)
		//const uint8* script = "@%\x49\x00=*%\x04\x00=&$";
		LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(4)));
		currentmap->areaSearch(&uclist, script, sizeof(script),
							   0, 256, false, 11732, 5844);
		if (uclist.getSize() < 1) {
			perr << "Unable to find EXCUTION egg!" << std::endl;
			break;
		}

		uint16 objid = uclist.getuint16(0);
		Egg* egg = p_dynamic_cast<Egg*>(
		ObjectManager::get_instance()->getObject(objid));
		Actor* avatar = World::get_instance()->getNPC(1);
		sint32 x,y,z;
		egg->getLocation(x,y,z);
		avatar->collideMove(x,y,z,true,true);
		egg->hatch();
	}
	break;

	}
	return handled;
}

bool runFirstEgg(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		CurrentMap* currentmap = World::get_instance()->getCurrentMap();
		UCList uclist(2);
		// (shape == 73 && quality == 36)
		//const uint8 script[] = "@%\x49\x00=*%\x24\x00=&$";
		LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(36)));
		currentmap->areaSearch(&uclist, script, sizeof(script),
						   0, 256, false, 16188, 7500);
		if (uclist.getSize() < 1) {
			perr << "Unable to find FIRST egg!" << std::endl;
			break;
		}

		uint16 objid = uclist.getuint16(0);
		Egg* egg = p_dynamic_cast<Egg*>(
			ObjectManager::get_instance()->getObject(objid));
		sint32 ix, iy, iz;
		egg->getLocation(ix,iy,iz);
		// Center on egg
		CameraProcess::SetCameraProcess(new CameraProcess(ix,iy,iz));
		egg->hatch();
	}
	break;

	}
	return handled;
}

bool toggleCombat(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		if (GUIApp::get_instance()->isAvatarInStasis())
		{
			pout << "Can't: avatarInStasis" << std::endl;
			break;
		}
		MainActor* av = World::get_instance()->getMainActor();
		av->toggleInCombat();
	}
	break;

	}
	return handled;
}

bool u8ShapeViewer(const SDL_Event& event)
{
	switch (event.type) {
	case HID_DOWN:
		ShapeViewerGump::U8ShapeViewer();
		return true;
	default:
		return false;
	}
}


}; // namespace HIDBindings
