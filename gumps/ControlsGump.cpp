/*
 *  Copyright (C) 2004  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ControlsGump.h"

#include "RenderSurface.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "QuitGump.h"
#include "BindGump.h"

#include "GUIApp.h"
#include "HIDManager.h"
#include "ObjectManager.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ControlsGump,ModalGump);

ControlsGump::ControlsGump(): ModalGump(0, 0, 5, 5)
{
	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	app->setMouseCursor(GUIApp::MOUSE_HAND);
}

ControlsGump::~ControlsGump()
{
	entries.clear();
	GUIApp::get_instance()->popMouseCursor();
}

void ControlsGump::InitGump()
{
	ModalGump::InitGump();

	dims.w = 300;
	dims.h = 200;

	Gump * widget = new TextWidget(4, 4, "Controls", 0);
	widget->InitGump();
	AddChild(widget, false);
	widget = new TextWidget(124, 4, "Keys", 0);
	widget->InitGump();
	AddChild(widget, false);

	int x = 4;
	int y = 20;
	addEntry("quickSave", "Quick Save", x, y);
	addEntry("quickLoad", "Quick Save", x, y);
	addEntry("engineStats", "Display Engine Stats", x, y);
	addEntry("paintEditorItems", "View Editor Items", x, y);
	addEntry("itemLocator", "Locate Item", x, y);
	addEntry("toggleCombat", "Toggle Combat", x, y);
	addEntry("openInventory", "Open Inventory", x, y);
	addEntry("openBackpack", "Open Backpack", x, y);
	addEntry("recall", "Recall", x, y);
	addEntry("runFirstEgg", "First Scene", x, y);
	addEntry("runExecutionEgg", "Execution Scene", x, y);
	addEntry("u8ShapeViewer", "Shape Viewer", x, y);
	addEntry("showMenu", "Menu", x, y);
	addEntry("quit", "Quit", x, y);
	addEntry("toggleConsole", "Console", x, y);

	std::vector<const char *> controls;
	std::vector<const char *>::iterator j;
	std::vector<entryPair>::iterator i;
	HIDManager * hidmanager = HIDManager::get_instance();
	ObjectManager * objectmanager = ObjectManager::get_instance();
	Pentagram::Rect rect;

	y = 20;
	for (i = entries.begin(); i != entries.end(); ++i)
	{
		x = 124;
		hidmanager->getBindings(i->second, controls);
		for (j = controls.begin(); j != controls.end(); ++j)
		{
			widget = new TextWidget(x, y, *j, 0);
			widget->InitGump();
			AddChild(widget, false);
			widget->GetDims(rect);
			x += 30;
		}
		y += rect.h;
	}	
}

void ControlsGump::addEntry(char * binding, char * name, int & x, int & y)
{
	entryPair e;
	Pentagram::Rect rect;
	Gump * widget = new ButtonWidget(x, y, name, 0);
	widget->InitGump();
	AddChild(widget);
	e.first = widget->getObjId();
	e.second = binding;
	entries.push_back(e);
	widget->GetDims(rect);
	y += rect.h;
}

void ControlsGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	// temporary - this will be a transparent gump
	uint32 rgb = 0xA0A0A0;
	surf->Fill32(rgb, 0, 0, dims.w, dims.h);
	
	Gump::PaintThis(surf, lerp_factor);
}

bool ControlsGump::OnKeyDown(int key, int mod)
{
	switch (key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

void ControlsGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
	}
}

//static
void ControlsGump::showMenu()
{
	Gump* desktopGump = GUIApp::get_instance()->getDesktopGump();
	ModalGump* gump = new ControlsGump();
	gump->InitGump();
	desktopGump->AddChild(gump);
	gump->setRelativePosition(CENTER);
}

bool ControlsGump::loadData(IDataSource* ids)
{
	return true;
}

void ControlsGump::saveData(ODataSource* ods)
{
}

