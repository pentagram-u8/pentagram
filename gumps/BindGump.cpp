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

#include "Font.h"
#include "RenderSurface.h"
#include "GUIApp.h"
#include "TextWidget.h"
#include "Mouse.h"

#include "HIDManager.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BindGump,ModalGump);

BindGump::BindGump(Pentagram::istring * b, Gump * g): ModalGump(0, 0, 160, 80), binding(b), invoker(g)
{
}

BindGump::~BindGump()
{
}

void BindGump::InitGump()
{
	ModalGump::InitGump();

	//! English ! - TODO: Externalize string.
	std::string bindtext = "Press any key or button.\n\nPress ESCAPE to cancel\nor BACKSPACE to clear";
	Gump * widget = new TextWidget(0, 0, bindtext, 6, 0, 0,
								   Pentagram::Font::TEXT_CENTER);
	widget->InitGump();
	AddChild(widget);
	widget->setRelativePosition(TOP_CENTER, 0, 8);
}


void BindGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	uint32 rgb = 0x000000;
	surf->Fill32(rgb, 0, 0, dims.w, dims.h);

	Gump::PaintThis(surf, lerp_factor);
}

bool BindGump::OnKeyDown(int key, int mod)
{
	if (key != SDLK_ESCAPE && binding)
	{
		HIDManager * hidmanager = HIDManager::get_instance();
		if (key == SDLK_BACKSPACE) {
			hidmanager->unbind(*binding);
		}
		else {
			Pentagram::istring control = SDL_GetKeyName((SDLKey) key);
			hidmanager->bind(control, *binding);
		}
		if (invoker)
			invoker->ChildNotify(this, UPDATE);
	}
	Close();	
	return true;
}

Gump * BindGump::OnMouseDown(int button, int mx, int my)
{
	Pentagram::istring control = GetMouseButtonName((MouseButton) button);
	HIDManager * hidmanager = HIDManager::get_instance();
	if (binding)
		hidmanager->bind(control, *binding);
	if (invoker)
		invoker->ChildNotify(this, UPDATE);
	Close();
	return this;
}

bool BindGump::loadData(IDataSource* ids)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void BindGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}
