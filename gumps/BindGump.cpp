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
#include "BindGump.h"

#include "RenderSurface.h"
#include "GUIApp.h"
#include "TextWidget.h"
#include "Mouse.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BindGump,ModalGump);

BindGump::BindGump(): ModalGump(0, 0, 160, 80)
{
}

BindGump::~BindGump()
{
}

void BindGump::InitGump()
{
	ModalGump::InitGump();

	int y = 8;
	Pentagram::Rect rect;
	Gump * widget = new TextWidget(0, 0, "Press any key or button.", 6);
	widget->InitGump();
	AddChild(widget);
	widget->setRelativePosition(TOP_CENTER, 0, y);
	widget->GetDims(rect);

	y += rect.h * 2;
	widget = new TextWidget(0, 0, "Press ESCAPE to cancel", 6);
	widget->InitGump();
	AddChild(widget);
	widget->setRelativePosition(TOP_CENTER, 0, y);
	widget->GetDims(rect);

	y += rect.h;
	widget = new TextWidget(0, 0, "or BACKSPACE to clear.", 6);
	widget->InitGump();
	AddChild(widget);
	widget->setRelativePosition(TOP_CENTER, 0, y);
}


void BindGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	uint32 rgb = 0x000000;
	surf->Fill32(rgb, 0, 0, dims.w, dims.h);

	Gump::PaintThis(surf, lerp_factor);
}

bool BindGump::OnKeyDown(int key, int mod)
{
	if (key != SDLK_ESCAPE)
	{
		pout << "Key pressed - " << SDL_GetKeyName((SDLKey) key) << std::endl;
	}
	Close();	
	return true;
}

Gump * BindGump::OnMouseDown(int button, int mx, int my)
{
	pout << "Mouse Button pressed - " << GetMouseButtonName((MouseButton) button) << std::endl;
	Close();
	return this;
}

//static
void BindGump::askBinding(Gump * parent)
{
	ModalGump* gump = new BindGump();
	gump->InitGump();
	parent->AddChild(gump);
	gump->setRelativePosition(CENTER);
}

bool BindGump::loadData(IDataSource* ids)
{
	return true;
}

void BindGump::saveData(ODataSource* ods)
{
}

