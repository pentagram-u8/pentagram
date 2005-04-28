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
#include "QuickAvatarMoverProcess.h"

#include "Gump.h"
#include "QuitGump.h"
#include "ConsoleGump.h"
#include "GameMapGump.h"

namespace HIDBindings {

bool avatarInStasis(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp * g = GUIApp::get_instance();
		g->toggleAvatarInStasis();
		pout << "avatarInStasis = " << g->isAvatarInStasis() << std::endl;
	} break;

	default:
		break;
	}
	return handled;
}

bool engineStats(const HID_Event& event)
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
	} break;

	default:
		break;
	}
	return handled;
}

bool itemLocator(const HID_Event& event)
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
	} break;

	default:
		break;
	}
	return handled;
}

bool paintEditorItems(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp * g = GUIApp::get_instance();
		g->togglePaintEditorItems();
		pout << "paintEditorItems = " << g->isPaintEditorItems() << std::endl;
	} break;

	default:
		break;
	}
	return handled;
}

bool showTouchingItems(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp * g = GUIApp::get_instance();
		g->toggleShowTouchingItems();
		pout << "ShowTouchingItems = " << g->isShowTouchingItems() << std::endl;
	} break;

	default:
		break;
	}
	return handled;
}

bool quickLoad(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp::get_instance()->loadGame("@save/quicksave");
	} break;

	default:
		break;
	}
	return handled;
}

bool quickSave(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		handled = true;
		GUIApp::get_instance()->saveGame("@save/quicksave", "quicksave");
	} break;

	default:
		break;
	}
	return handled;
}

bool toggleFrameByFrame(const HID_Event& event)
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
	} break;

	default:
		break;
	}
	return handled;
}

bool advanceFrameByFrame(const HID_Event& event)
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
	} break;

	default:
		break;
	}
	return handled;
}

bool quit(const HID_Event& event)
{
	switch (event.type) {
	case HID_DOWN:
		QuitGump::verifyQuit();
		return true;
	default:
		return false;
	}
}

bool toggleConsole(const HID_Event& event)
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

bool quickMoveUp(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(-64,-64,0,0);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(0);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveDown(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(+64,+64,0,1);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(1);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveLeft(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(-64,+64,0,2);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(2);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveRight(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(+64,-64,0,3);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(3);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveAscend(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(0,0,8,4);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(4);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveDescend(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GUIApp * g = GUIApp::get_instance();
		if (! g->isAvatarInStasis())
		{
			Process *p = new QuickAvatarMoverProcess(0,0,-8,5);
			Kernel::get_instance()->addProcess(p);
		} else {
			pout << "Can't: avatarInStasis" << std::endl;
		}
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::terminateMover(5);
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveQuarterSpeed(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		QuickAvatarMoverProcess::setQuarterSpeed(true);
		pout << "QuickAvatarMoverProcess::quarter = " << QuickAvatarMoverProcess::isQuarterSpeed() << std::endl;
		handled = true;
	} break;
	case HID_UP:
	{
		QuickAvatarMoverProcess::setQuarterSpeed(false);
		pout << "QuickAvatarMoverProcess::quarter = " << QuickAvatarMoverProcess::isQuarterSpeed() << std::endl;
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

bool quickMoveClipping(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		QuickAvatarMoverProcess::toggleClipping();
		pout << "QuickAvatarMoverProcess::clipping = " << QuickAvatarMoverProcess::isClipping() << std::endl;
		handled = true;
	} break;
	default:
		break;
	}
	return handled;
}

bool highlightItems(const HID_Event& event)
{
	bool handled = false;
	switch (event.type) {
	case HID_DOWN:
	{
		GameMapGump::SetHighlightItems(true);
		pout << "GameMapGump::highlightItems = " << GameMapGump::isHighlightItems() << std::endl;
		handled = true;
	} break;
	case HID_UP:
	{
		GameMapGump::SetHighlightItems(false);
		pout << "GameMapGump::highlightItems = " << GameMapGump::isHighlightItems() << std::endl;
		handled = true;
	}
	default:
		break;
	}
	return handled;
}

};
