/*
 *  Copyright (C) 2003-2007  The Pentagram Team
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
#include "DesktopGump.h"
#include "RenderSurface.h"
#include "GUIApp.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ConsoleGump.h"
#include "ModalGump.h"
#include "TargetGump.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(DesktopGump,Gump);

bool DesktopGump::faded_modal = true;

DesktopGump::DesktopGump()
	: Gump()
{
}

DesktopGump::DesktopGump(sint32 _x, sint32 _y, sint32 _width, sint32 _height) :
	Gump(_x, _y, _width, _height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP,
		 LAYER_DESKTOP)
{
}

DesktopGump::~DesktopGump(void)
{
}

void DesktopGump::PaintThis(RenderSurface *surf, sint32 lerp_factor, bool scaled)
{
	// Just fill it (only if console showing, or in debug mode)

#ifndef DEBUG
	ConsoleGump *console = GUIApp::get_instance()->getConsoleGump();
	if (console->ConsoleIsVisible() || (children.size() && children.front()->IsOfType<ConsoleGump>()))
#endif
		surf->Fill32(0x000000, 0, 0, dims.w, dims.h);
		//surf->Fill32(0x3f3f3f, 0, 0, dims.w, dims.h);
}

void DesktopGump::PaintChildren(RenderSurface *surf, sint32 lerp_factor, bool scaled)
{
	// Iterate all children
	std::list<Gump*>::iterator it = children.begin();
	std::list<Gump*>::iterator end = children.end();

	while (it != end)
	{
		Gump *g = *it;

		// Paint if not closing
		if (!g->IsClosing()) 
		{
			// If background blanking on modal is enabled...
			// Background is partially transparent
			if (faded_modal && g->IsOfType<ModalGump>() &&
					!g->IsOfType<TargetGump>() && !g->IsHidden())
				surf->FillBlended(0x7F000000,0,0,dims.w,dims.h);

			g->Paint(surf, lerp_factor, scaled);
		}

		++it;
	}	
}

bool DesktopGump::StartDraggingChild(Gump* gump, int mx, int my)
{
	gump->ParentToGump(mx, my);
	GUIApp::get_instance()->setDraggingOffset(mx, my);
	MoveChildToFront(gump);
	return true;
}

void DesktopGump::DraggingChild(Gump* gump, int mx, int my)
{
	int dx, dy;
	GUIApp::get_instance()->getDraggingOffset(dx, dy);
	gump->Move(mx - dx, my - dy);
}

void DesktopGump::StopDraggingChild(Gump* gump)
{

}

void DesktopGump::RenderSurfaceChanged(RenderSurface *surf)
{
	// Resize the desktop gump to match the RenderSurface 
	Pentagram::Rect new_dims;
	surf->GetSurfaceDims(new_dims);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	Gump::RenderSurfaceChanged();
}

void DesktopGump::RenderSurfaceChanged()
{
	// Resize the desktop gump to match the parent
	Pentagram::Rect new_dims;
	parent->GetDims(new_dims);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	Gump::RenderSurfaceChanged();
}

void DesktopGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save DesktopGump");
}

bool DesktopGump::loadData(IDataSource* ids, uint32 version)
{
	CANT_HAPPEN_MSG("Trying to save DesktopGump");

	return false;
}

// Colourless Protection
