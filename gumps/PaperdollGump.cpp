/*
 *  Copyright (C) 2003-2004  The Pentagram Team
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
#include "ShapeFont.h"
#include "FontShapeFlex.h"
#include "RenderedText.h"
#include "GumpShapeFlex.h"
#include "ButtonWidget.h"
#include "MiniStatsGump.h"
#include "GUIApp.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(PaperdollGump,ContainerGump);



// lots of CONSTANTS...
struct {
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

struct {
    int xd, x, y;
} statcoords[] = {
	{ 90, 130, 24 },
	{ 90, 130, 33 },
	{ 90, 130, 42 },
	{ 90, 130, 51 },
	{ 90, 130, 60 },
	{ 90, 130, 69 },
	{ 90, 130, 78 }
};

static const int statdescwidth = 29;
static const int statwidth = 15;
static const int statheight = 8;
static const int statfont = 7;
static const int statdescfont = 0;

const Pentagram::Rect backpack_rect(49, 25, 10, 25);

static const int statbuttonshape = 38;
static const int statbuttonx = 81;
static const int statbuttony = 84;


PaperdollGump::PaperdollGump()
	: ContainerGump()
{

}

PaperdollGump::PaperdollGump(Shape* shape_, uint32 framenum_, uint16 owner,
							 uint32 Flags_, sint32 layer)
	: ContainerGump(shape_, framenum_, owner, Flags_, layer)
{
	statbuttongid = 0;
}

PaperdollGump::~PaperdollGump()
{

}

void PaperdollGump::InitGump()
{
	ContainerGump::InitGump();

	Shape* childshape = GameData::get_instance()->getGumps()->
		getShape(statbuttonshape);

	Gump *widget = new ButtonWidget(statbuttonx, statbuttony,
									childshape, 0, childshape, 1);
	statbuttongid = widget->getObjId();
	widget->InitGump();
	AddChild(widget);
}


void PaperdollGump::PaintStats(RenderSurface* surf, sint32 lerp_factor)
{
	Actor* a = World::get_instance()->getNPC(owner);
	assert(a);

	// text: English: STR,INT,DEX,ARMR,HITS,MANA,WGHT
	//       French : FORCE,INTEL,DEXT,ARMR,COUPS,MAGIE,POIDS
    //       German : KRAFT,INTELL.,GESCH.,R\"UST.,TREFF.,MANA,LAST
	//       Spanish: FUE,INT,DES,ARMR,PNTS,MANA,PESO

	ShapeFont* font = GameData::get_instance()->getFonts()->getFont(statfont);
	ShapeFont* descfont = GameData::get_instance()->
		getFonts()->getFont(statdescfont);
	char buf[16]; // enough for uint32
	RenderedText* rendtext;
	unsigned int remaining;

	sprintf(buf, "%d", a->getStr());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[0].x, statcoords[0].y);
	delete rendtext;

#if 0
	// TODO: hardcoded text!! (needs different languages)
	rendtext = descfont->renderText("STR", remaining, statdescwidth,statheight,
									Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[0].xd, statcoords[0].y);
	delete rendtext;
#endif

	sprintf(buf, "%d", a->getInt());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[1].x, statcoords[1].y);
	delete rendtext;

	sprintf(buf, "%d", a->getDex());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[2].x, statcoords[2].y);
	delete rendtext;

	sprintf(buf, "%d", a->getArmourClass());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[3].x, statcoords[3].y);
	delete rendtext;

	sprintf(buf, "%d", a->getHP());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[4].x, statcoords[4].y);
	delete rendtext;

	sprintf(buf, "%d", a->getMana());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[5].x, statcoords[5].y);
	delete rendtext;

	sprintf(buf, "%d", a->getTotalWeight());
	rendtext = font->renderText(buf, remaining, statwidth, statheight,
							   Pentagram::Font::TEXT_RIGHT);
	rendtext->draw(surf, statcoords[6].x, statcoords[6].y);
	delete rendtext;
}

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

	PaintStats(surf, lerp_factor);

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
uint16 PaperdollGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx,my);
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
	Item* parent = item->getParentAsContainer();
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

void PaperdollGump::ChildNotify(Gump *child, uint32 message)
{
	if (child->getObjId() == statbuttongid &&
		message == ButtonWidget::BUTTON_CLICK)
	{
		// check if there already is an open MiniStatsGump
		Gump* desktop = GUIApp::get_instance()->getDesktopGump();
		if (!desktop->FindGump(MiniStatsGump::ClassType)) {
			Gump* gump = new MiniStatsGump(0, 0);
			gump->InitGump();
			desktop->AddChild(gump);
			gump->setRelativePosition(BOTTOM_RIGHT, -5, -5);
		}
	}
}


void PaperdollGump::saveData(ODataSource* ods)
{
	ods->write2(1);
	ContainerGump::saveData(ods);

	ods->write2(statbuttongid);
}

bool PaperdollGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ContainerGump::loadData(ids)) return false;

	statbuttongid = ids->read2();

	return true;
}
