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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "PaperdollGump.h"

#include "Shape.h"
#include "ShapeFrame.h"
#include "ShapeInfo.h"
#include "Actor.h"
#include "World.h"
#include "RenderSurface.h"
#include "GameData.h"
#include "MainShapeFlex.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(PaperdollGump,ContainerGump);

PaperdollGump::PaperdollGump()
	: ContainerGump()
{

}

PaperdollGump::PaperdollGump(Shape* shape_, uint32 framenum_, uint16 owner,
							 uint32 Flags_, sint32 layer)
	: ContainerGump(shape_, framenum_, owner, Flags_, layer)
{
}

PaperdollGump::~PaperdollGump()
{

}


void PaperdollGump::InitGump()
{
	ContainerGump::InitGump();

}

// lots of CONSTANTS...
struct EquipCoords {
	int x, y;
} equipcoords[] = {
	{ 0, 0 },
	{ 23, 64 },
	{ 37, 50 },
	{ 40, 25 },
	{ 41, 63 },
	{ 40, 92 },
	{ 16, 18 }
};

const Pentagram::Rect backpack_rect(49, 25, 10, 25);

void PaperdollGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	// paint self
	ItemRelativeGump::PaintThis(surf, lerp_factor);

	Actor* a = World::get_instance()->getNPC(owner);

	if (!a) {
		// Actor gone!?
		Close();
		return;
	}

	for (int i = 6; i >= 1; --i) { // constants
		Item* item = World::get_instance()->getItem(a->getEquip(i));
		if (!item) continue;
		sint32 itemx,itemy;
		uint32 frame = item->getFrame() + 1;

		itemx = equipcoords[i].x;
		itemy = equipcoords[i].y;
		itemx += itemarea.x;
		itemy += itemarea.y;
		Shape* s = item->getShapeObject();
		assert(s);
		surf->Paint(s, frame, itemx, itemy);
	}

	if (display_dragging) {
		sint32 itemx, itemy;
		itemx = dragging_x + itemarea.x;
		itemy = dragging_y + itemarea.y;
		Shape* s = GameData::get_instance()->getMainShapes()->
			getShape(dragging_shape);
		assert(s);
		surf->PaintInvisible(s, dragging_frame, itemx, itemy, false, (dragging_flags&Item::FLG_FLIPPED)!=0);
	}
}

// Find object (if any) at (mx,my)
// (mx,my) are relative to parent
uint16 PaperdollGump::TraceObjID(int mx, int my)
{
	uint16 objid = Gump::TraceObjID(mx,my);
	if (objid && objid != 65535) return objid;

	ParentToGump(mx,my);

	Actor* a = World::get_instance()->getNPC(owner);

	if (!a) return 0; // Container gone!?

	for (int i = 1; i <= 6; ++i) {
		Item* item = World::get_instance()->getItem(a->getEquip(i));
		if (!item) continue;
		sint32 itemx,itemy;

		itemx = equipcoords[i].x;
		itemy = equipcoords[i].y;
		itemx += itemarea.x;
		itemy += itemarea.y;
		Shape* s = item->getShapeObject();
		assert(s);
		ShapeFrame* frame = s->getFrame(item->getFrame() + 1);

		if (frame->hasPoint(mx - itemx, my - itemy))
		{
			// found it
			return item->getObjId();
		}
	}

	// try backpack
	if (backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		if (a->getEquip(7)) // constants
			return a->getEquip(7);
	}

	// didn't find anything, so return self
	return getObjId();
}

// get item coords relative to self
bool PaperdollGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
									  sint32 lerp_factor)
{

	Item* item = World::get_instance()->getItem(itemid);
	Item* parent = item->getParent();
	if (!parent) return false;
	if (parent->getObjId() != owner) return false;

	//!!! need to use lerp_factor

	if (item->getShape() == 529) //!! constant
	{
		gx = backpack_rect.x;
		gy = backpack_rect.y;
	} else {
		int equiptype = item->getZ();
		assert(equiptype >= 0 && equiptype <= 6); //!! constants
		gx = equipcoords[equiptype].x;
		gy = equipcoords[equiptype].y;
	}
	gx += itemarea.x;
	gy += itemarea.y;

	return true;
}

bool PaperdollGump::StartDraggingItem(Item* item, int mx, int my)
{
	// can't drag backpack
	if (item->getShape() == 529) { //!! constant
		return false;
	}

	return ContainerGump::StartDraggingItem(item, mx, my);
}


bool PaperdollGump::DraggingItem(Item* item, int mx, int my)
{
	if (!itemarea.InRect(mx, my)) {
		display_dragging = false;
		return false;
	}

	Actor* a = World::get_instance()->getNPC(owner);
	assert(a);

	bool over_backpack = false;
	Container* backpack = p_dynamic_cast<Container*>(
		World::get_instance()->getItem(a->getEquip(7))); // constant!
			
	if (backpack && backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		over_backpack = true;
	}

	display_dragging = true;

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();

	int equiptype = item->getShapeInfo()->equiptype;
	// determine target location and set dragging_x/y
	if (!over_backpack && equiptype) {
		// check if item will fit (weight/volume/etc...)
		if (!a->CanAddItem(item, true)) {
			display_dragging = false;
			return false;
		}

		dragging_frame++;
		dragging_x = equipcoords[equiptype].x;
		dragging_y = equipcoords[equiptype].y;
	} else {
		// drop in backpack

		if (!backpack->CanAddItem(item, true)) {
			display_dragging = false;
			return false;
		}

		dragging_x = backpack_rect.x + backpack_rect.w/2;
		dragging_y = backpack_rect.y + backpack_rect.h/2;
	}

	return true;
}

void PaperdollGump::DropItem(Item* item, int mx, int my)
{
	display_dragging = false;

	Actor* a = World::get_instance()->getNPC(owner);
	assert(a);

	bool over_backpack = false;
	Container* backpack = p_dynamic_cast<Container*>(
		World::get_instance()->getItem(a->getEquip(7))); // constant!
			
	if (backpack && backpack_rect.InRect(mx - itemarea.x, my - itemarea.y)) {
		over_backpack = true;
	}

	int equiptype = item->getShapeInfo()->equiptype;
	if (!over_backpack && equiptype) {
		item->moveToContainer(a);
	} else {
		item->moveToContainer(backpack);

		// TODO: find a better place
		item->setGumpLocation(0, 0);
	}
}

void PaperdollGump::saveData(ODataSource* ods)
{
	ods->write2(1);
	ContainerGump::saveData(ods);
}

bool PaperdollGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ContainerGump::loadData(ids)) return false;

	return true;
}
