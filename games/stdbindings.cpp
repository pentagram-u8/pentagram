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
#include "stdbindings.h"

#include "Kernel.h"
#include "ObjectManager.h"
#include "UCMachine.h"
#include "World.h"
#include "Item.h"
#include "GUIApp.h"

#include "Gump.h"
#include "QuitGump.h"
#include "ConsoleGump.h"

namespace HIDBindings {

bool avatarInStatis(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp * g = GUIApp::get_instance();
		g->toggleAvatarInStasis();
		pout << "avatarInStasis = " << g->isAvatarInStasis() << std::endl;
	}
	break;

	}
	return handled;
}

bool engineStats(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		Kernel::get_instance()->kernelStats();
		ObjectManager::get_instance()->objectStats();
		UCMachine::get_instance()->usecodeStats();
		World::get_instance()->worldStats();
	}
	break;

	}
	return handled;
}

bool itemLocator(const SDL_Event& event)
{
	bool handled = false;
	int itemNumber = 19204;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		Item* item = World::get_instance()->getItem(itemNumber);
		if (! item)
		{
			pout << "Item not found: " << itemNumber << std::endl;
			break;
		}
		sint32 x,y,z;
		item->getLocation(x,y,z);
		pout << itemNumber << ": (" << x << "," << y << "," << z << ")" << std::endl;
	}
	break;

	}
	return handled;
}

bool paintEditorItems(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp * g = GUIApp::get_instance();
		g->togglePaintEditorItems();
		pout << "paintEditorItems = " << g->isPaintEditorItems() << std::endl;
	}
	break;

	}
	return handled;
}

bool quickLoad(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp::get_instance()->loadGame("@save/quicksave");
	}
	break;

	}
	return handled;
}

bool quickSave(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp::get_instance()->saveGame("@save/quicksave");
	}
	break;

	}
	return handled;
}

bool toggleFrameByFrame(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		Kernel* kernel = Kernel::get_instance();
		bool fbf = !kernel->isFrameByFrame();
		kernel->setFrameByFrame(fbf);
		pout << "FrameByFrame = " << fbf << std::endl;
		if (fbf)
			kernel->pause();
		else
			kernel->unpause();
	}
	break;

	}
	return handled;
}

bool advanceFrameByFrame(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		Kernel* kernel = Kernel::get_instance();
		if (kernel->isFrameByFrame()) {
			handled = true;
			kernel->unpause();
			pout << "FrameByFrame: Next Frame" << std::endl;
		}
	}
	break;

	}
	return handled;
}

bool quit(const SDL_Event& event)
{
	switch (event.type) {
	case HID_DOWN:
		QuitGump::verifyQuit();
		return true;
	default:
		return false;
	}
}

bool toggleConsole(const SDL_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		ConsoleGump * consoleGump = GUIApp::get_instance()->getConsoleGump();
		consoleGump->ToggleConsole();
		handled = true;
	} break;
	default:
		break;
	}
	return handled;
}

};
