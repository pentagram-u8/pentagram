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
#include "ContainerGump.h"

#include "Shape.h"
#include "ShapeFrame.h"
#include "ShapeInfo.h"
#include "Container.h"
#include "World.h"
#include "RenderSurface.h"
#include "GUIApp.h"
#include "Kernel.h"
#include "GameData.h"
#include "MainShapeFlex.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ContainerGump,ItemRelativeGump);

ContainerGump::ContainerGump()
	: ItemRelativeGump(), display_dragging(false)
{

}

ContainerGump::ContainerGump(Shape* shape_, uint32 framenum_, uint16 owner,
							 uint32 Flags_, sint32 layer)
	: ItemRelativeGump(0, 0, 5, 5, owner, Flags_, layer),
	  display_dragging(false)
{
	shape = shape_;
	framenum = framenum_;
}

ContainerGump::~ContainerGump()
{

}

void ContainerGump::InitGump()
{
	ItemRelativeGump::InitGump();

	ShapeFrame* sf = shape->getFrame(framenum);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	// make every item enter the fast area
	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return; // Container gone!?

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->enterFastArea();
	}

	// Position isn't like in the original
	// U8 puts a container gump slightly to the left of an object
}

void ContainerGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	// paint self
	ItemRelativeGump::PaintThis(surf, lerp_factor);

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) {
		// Container gone!?
		Close();
		return;
	}

	std::list<Item*>& contents = c->contents;
	sint32 gametick = Kernel::get_instance()->getFrameNum();

	//!! TODO: check these painting commands (flipped? translucent?)

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		item->setupLerp(gametick);
		sint32 itemx,itemy;
		item->getGumpLocation(itemx,itemy);

		itemx += itemarea.x;
		itemy += itemarea.y;
		Shape* s = item->getShapeObject();
		assert(s);
		surf->Paint(s, item->getFrame(), itemx, itemy);
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
uint16 ContainerGump::TraceObjID(int mx, int my)
{
	uint16 objid = Gump::TraceObjID(mx,my);
	if (objid && objid != 65535) return objid;

	ParentToGump(mx,my);

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return 0; // Container gone!?

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::reverse_iterator iter;
	// iterate backwards, since we're painting from begin() to end()
	for (iter = contents.rbegin(); iter != contents.rend(); ++iter) {
		Item* item = *iter;
		sint32 itemx,itemy;
		item->getGumpLocation(itemx,itemy);
		Shape* s = item->getShapeObject();
		assert(s);
		ShapeFrame* frame = s->getFrame(item->getFrame());

		if (frame->hasPoint(mx - (itemx + itemarea.x),
							my - (itemy + itemarea.y)))
		{
			// found it
			return item->getObjId();
		}
	}

	// didn't find anything, so return self
	return getObjId();
}

// get item coords relative to self
bool ContainerGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
									  sint32 lerp_factor)
{
	Item* item = World::get_instance()->getItem(itemid);
	Item* parent = item->getParent();
	if (!parent) return false;
	if (parent->getObjId() != owner) return false;

	//!!! need to use lerp_factor

	item->getGumpLocation(gx,gy);
	gx += itemarea.x;
	gy += itemarea.y;

	return false;
}

void ContainerGump::Close(bool no_del)
{
	// close any gumps belonging to contents
	// and make every item leave the fast area
	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return; // Container gone!?

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::iterator iter = contents.begin();
	while(iter != contents.end()) {
		Item* item = *iter;
		++iter;
		Gump* g = GUIApp::get_instance()->getGump(item->getGump());
		if (g) {
			g->Close(); //!! what about no_del?
		}
		item->leaveFastArea();	// Can destroy the item
	}

	Item* o = World::get_instance()->getItem(owner);
	if (o)
		o->clearGump(); //!! is this the appropriate place?

	ItemRelativeGump::Close(no_del);
}

Gump* ContainerGump::OnMouseDown(int button, int mx, int my)
{
	// only interested in left clicks
	if (button == GUIApp::BUTTON_LEFT)
		return this;

	return 0;
}

void ContainerGump::OnMouseClick(int button, int mx, int my)
{
	if (button == GUIApp::BUTTON_LEFT)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			return;
		}
		
		uint16 objID = TraceObjID(mx, my);

		World *world = World::get_instance();
		Item *item = world->getItem(objID);
		if (item) {
			extern uint16 targetObject; // major hack number 2
			targetObject = objID;
			item->dumpInfo();
			
			// call the 'look' event
			item->callUsecodeEvent_look();
		}
	}
}

void ContainerGump::OnMouseDouble(int button, int mx, int my)
{
	if (button == GUIApp::BUTTON_LEFT)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			return;
		}
		
		uint16 objID = TraceObjID(mx, my);

		if (objID == getObjId()) {
			objID = owner; // use container when double click on self
		}

		World *world = World::get_instance();
		Item *item = world->getItem(objID);
		if (item) {
			item->dumpInfo();
			
			// call the 'use' event
			item->callUsecodeEvent_use();
		}		
	}
}


bool ContainerGump::StartDraggingItem(Item* item, int mx, int my)
{
	// probably don't need to check if item can be moved, since it shouldn't
	// be in a container otherwise

	//TODO: do need to check if the container the item is in, is in range
	return true;
}

bool ContainerGump::DraggingItem(Item* item, int mx, int my)
{
	display_dragging = true;

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();

	// determine target location and set dragging_x/y

	dragging_x = mx - itemarea.x;
	dragging_y = my - itemarea.y;

	if (dragging_x < 0 || dragging_x >= itemarea.w ||
		dragging_y < 0 || dragging_y >= itemarea.h) {
		display_dragging = false;
		return false;
	}


	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));
	assert(c);

	// check if item will fit (weight/volume/adding container to itself)
	if (!c->CanAddItem(item, true)) {
		display_dragging = false;
		return false;
	}

	return true;
}

void ContainerGump::DraggingItemLeftGump(Item* item)
{
	display_dragging = false;
}


void ContainerGump::StopDraggingItem(Item* item, bool moved)
{
	if (!moved) return; // nothing to do
}

void ContainerGump::DropItem(Item* item, int mx, int my)
{
	display_dragging = false;

	int px = mx, py = my;
	GumpToParent(px, py);
	// see what the item is being dropped on
	Item* targetitem = World::get_instance()->getItem(TraceObjID(px, py));
	Container* targetcontainer = 0;
	if (targetitem) {
		targetcontainer = p_dynamic_cast<Container*>(targetitem);
		if (targetcontainer) {

		} else {
			// try to combine items
			int family = item->getShapeInfo()->family;

			// item with quantity?
			if (family == ShapeInfo::SF_QUANTITY ||
				family == ShapeInfo::SF_REAGENT)
			{
				if (targetitem->getShape() == item->getShape() &&
					(targetitem->getFrame() == item->getFrame() ||
					 family == ShapeInfo::SF_QUANTITY))
				{
					// can only combine SF_REAGENTS when frame is equal
					// see U8's usecode FREE::2767
					targetitem->setQuality(targetitem->getQuality() +
										   item->getQuality());
					targetitem->callUsecodeEvent_combine();

					// combined, so delete item
					item->destroy(); item = 0;
					return;
				}
						
			}
		}
	}

	if (!targetcontainer) {
		targetcontainer = p_dynamic_cast<Container*>
			(World::get_instance()->getItem(owner));
	}
	// add item to container
	assert(targetcontainer);
	item->moveToContainer(targetcontainer);

	//!! TODO: this is nonsense when not adding to this container
	dragging_x = mx - itemarea.x;
	dragging_y = my - itemarea.y;
	item->setGumpLocation(dragging_x, dragging_y);
}

void ContainerGump::saveData(ODataSource* ods)
{
	ods->write2(1);
	ItemRelativeGump::saveData(ods);

	ods->write4(static_cast<uint32>(itemarea.x));
	ods->write4(static_cast<uint32>(itemarea.y));
	ods->write4(static_cast<uint32>(itemarea.w));
	ods->write4(static_cast<uint32>(itemarea.h));
}

bool ContainerGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ItemRelativeGump::loadData(ids)) return false;

	sint32 iax = static_cast<sint32>(ids->read4());
	sint32 iay = static_cast<sint32>(ids->read4());
	sint32 iaw = static_cast<sint32>(ids->read4());
	sint32 iah = static_cast<sint32>(ids->read4());
	itemarea.Set(iax, iay, iaw, iah);

	return true;
}
