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
	Gump::InitGump();

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

	entryGumps[4] = 0;
	entryGumps[5] = 0;
}

void OptionsGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (cid == entryGumps[i])
			{
				//! Hack! Taking advantage of key ordering because
				// I'm tired of writing code twice.
				OnKeyDown(i + SDLK_1, 0);
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
	switch (key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	case SDLK_1:
	{	// Video
	} break;
	case SDLK_2:
	{	// Audio
	} break;
	case SDLK_3:
	{	// Controls
		PagedGump * gump = new PagedGump(34, -38, 3, gumpShape);
		gump->InitGump();
		gump->addPage(ControlsGump::showEngineMenu());
		gump->addPage(ControlsGump::showU8Menu());
		//! BUG: try adding to this gump instead of parent to trigger a bug.
		parent->AddChild(gump);
		gump->setRelativePosition(CENTER);
	} break;	
	case SDLK_4:
	{	// Gameplay
	} break;
	case SDLK_5:
	{
	} break;
	case SDLK_6:
	{
	} break;
	default:
		break;
	}

	return true;
}

bool OptionsGump::loadData(IDataSource* ids)
{
	return true;
}

void OptionsGump::saveData(ODataSource* ods)
{
}

