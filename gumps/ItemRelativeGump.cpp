/*
 *  Copyright (C) 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ItemRelativeGump.h"
#include "GameMapGump.h"
#include "Item.h"
#include "World.h"
#include "Container.h"
#include "ShapeInfo.h"
#include "GUIApp.h"
#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ItemRelativeGump,Gump);

ItemRelativeGump::ItemRelativeGump()
	: Gump(), ix(0), iy(0)
{
}

ItemRelativeGump::ItemRelativeGump(int x, int y, int width, int height,
								   uint16 owner, uint32 _Flags, sint32 _Layer)
	: Gump(x, y, width, height, owner, _Flags, _Layer), ix(0), iy(0)
{
}

ItemRelativeGump::~ItemRelativeGump(void)
{
}

// Paint the Gump (RenderSurface is relative to parent).
// Calls PaintThis and PaintChildren
void ItemRelativeGump::Paint(RenderSurface*surf, sint32 lerp_factor)
{
	GetItemLocation(lerp_factor);
	Gump::Paint(surf,lerp_factor);
}


// Convert a parent relative point to a gump point
void ItemRelativeGump::ParentToGump(int &px, int &py)
{
	px -= ix; 
	py -= iy;
	Gump::ParentToGump(px,py);
}

// Convert a gump point to parent relative point
void ItemRelativeGump::GumpToParent(int &gx, int &gy)
{
	Gump::GumpToParent(gx,gy);
	gx += ix;
	gy += iy;
}

void ItemRelativeGump::GetItemLocation(sint32 lerp_factor)
{
	Item *it = 0;
	Item *next = 0;
	Item *prev = 0;
	Gump *gump = 0;

	it = World::get_instance()->getItem(owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	while ((next = it->getParent()) != 0)
	{
		prev = it;
		it = next;
		gump = GUIApp::get_instance()->getGump(it->getGump());
		if (gump) break;
	}

	int gx, gy;

	if (!gump)
	{
		gump = GetRootGump()->FindGump<GameMapGump>();

		if (!gump) {
			perr << "ItemRelativeGump::GetItemLocation(): "
				 << "Unable to find GameMapGump!?!?" << std::endl;
			return;
		}

		gump->GetLocationOfItem(owner, gx, gy, lerp_factor);
	}
	else
	{
		gump->GetLocationOfItem(prev->getObjId(), gx, gy, lerp_factor);
	}

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gump->GumpToScreenSpace(gx,gy);

	// Convert the screenspace coords into the coords of us
	if (parent) parent->ScreenSpaceToGump(gx,gy);

	// Set x and y, and center us over it
	ix = gx-dims.w/2;
	iy = gy-dims.h-it->getShapeInfo()->z*8-16;
}

void ItemRelativeGump::Move(int x_, int y_)
{
	ParentToGump(x_, y_);
	x += x_;
	y += y_;

	x -= moveOffsetX;
	y -= moveOffsetY;
}

void ItemRelativeGump::saveData(ODataSource* ods)
{
	ods->write2(1); // version
	Gump::saveData(ods);
}

bool ItemRelativeGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	return true;
}

// Colourless Protection
