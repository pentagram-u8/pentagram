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
#include "OptionsGump.h"

#include "RenderSurface.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "ControlsGump.h"
#include "PagedGump.h"

#include "GUIApp.h"
#include "ObjectManager.h"

#include "IDataSource.h"
#include "ODataSource.h"

static const int font = 0;

DEFINE_RUNTIME_CLASSTYPE_CODE(OptionsGump,Gump);

OptionsGump::OptionsGump(): Gump(0, 0, 5, 5)
{
}

OptionsGump::~OptionsGump()
{
}

void OptionsGump::InitGump()
{
	int i;
	Gump::InitGump();
	for (i=0; i < 9; ++i)
	{
		entryGumps[i] = 0;
	}

	dims.w = 220;
	dims.h = 120;

	int x = dims.w / 2 + 14;
	int y = 4;
	Gump * widget;

	widget = new ButtonWidget(x, y, "1. Video", font);
	widget->InitGump();
	AddChild(widget);
	entryGumps[0] = widget->getObjId();
	y+= 14;

	widget = new ButtonWidget(x, y, "2. Audio", font);
	widget->InitGump();
	AddChild(widget);
	entryGumps[1] = widget->getObjId();
	y+= 14;

	widget = new ButtonWidget(x, y, "3. Controls", font);
	widget->InitGump();
	AddChild(widget);
	entryGumps[2] = widget->getObjId();
	y+= 14;

	widget = new ButtonWidget(x, y, "4. Gameplay", font);
	widget->InitGump();
	AddChild(widget);
	entryGumps[3] = widget->getObjId();
	y+= 14;
}

void OptionsGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		for (int i = 0; i < 9; ++i)
		{
			if (cid == entryGumps[i])
			{
				selectEntry(i + 1);
			}
		}
	}
}

void OptionsGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);
}

static const int gumpShape = 35;

bool OptionsGump::OnKeyDown(int key, int mod)
{
	if (key == SDLK_ESCAPE)
	{
		Close();
	}
	else if (key >= SDLK_1 && key <=SDLK_9)
	{
		// Minor hack.
		selectEntry(key - SDLK_1 + 1);
	}
	return true;
}

void OptionsGump::selectEntry(int entry)
{
	switch (entry)
	{
	case 1:
	{	// Video
	} break;
	case 2:
	{	// Audio
	} break;
	case 3:
	{	// Controls
		PagedGump * gump = new PagedGump(34, -38, 3, gumpShape);
		gump->InitGump();
		gump->addPage(ControlsGump::showEngineMenu());
		gump->addPage(ControlsGump::showU8Menu());
		//! BUG: try adding to this gump instead of parent to trigger a bug.
		parent->AddChild(gump);
		gump->setRelativePosition(CENTER);
	} break;	
	case 4:
	{	// Gameplay
	} break;
	case 5:
	{
	} break;
	case 6:
	{
	} break;
	default:
		break;
	}
}

bool OptionsGump::loadData(IDataSource* ids)
{
	return true;
}

void OptionsGump::saveData(ODataSource* ods)
{
}

