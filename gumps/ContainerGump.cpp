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
#include "MainShapeArchive.h"
#include "Mouse.h"
#include "SliderGump.h"
#include "GumpNotifyProcess.h"
#include "ItemFactory.h"
#include "SplitItemProcess.h"
#include "GameMapGump.h"
#include "MainActor.h"

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
	bool paintEditorItems = GUIApp::get_instance()->isPaintEditorItems();

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		item->setupLerp(gametick);

		if (!paintEditorItems && item->getShapeInfo()->is_editor())
			continue;

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
uint16 ContainerGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx,my);
	if (objid && objid != 65535) return objid;

	ParentToGump(mx,my);

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return 0; // Container gone!?

	bool paintEditorItems = GUIApp::get_instance()->isPaintEditorItems();

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::reverse_iterator iter;
	// iterate backwards, since we're painting from begin() to end()
	for (iter = contents.rbegin(); iter != contents.rend(); ++iter) {
		Item* item = *iter;
		if (!paintEditorItems && item->getShapeInfo()->is_editor())
			continue;

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
	Item* parent = item->getParentAsContainer();
	if (!parent) return false;
	if (parent->getObjId() != owner) return false;

	//!!! need to use lerp_factor

	item->getGumpLocation(gx,gy);
	gx += itemarea.x;
	gy += itemarea.y;

	return false;
}

// we don't want our position to depend on Gump of parent container
// so change the default ItemRelativeGump behaviour
void ContainerGump::GetItemLocation(sint32 lerp_factor)
{
	Item *it = World::get_instance()->getItem(owner);

	if (!it) {
		// This shouldn't ever happen, the GumpNotifyProcess should
		// close us before we get here
		Close();
		return;
	}

	int gx, gy;
	Item* topitem = it;

	Container *p = it->getParentAsContainer();
	if (p) {
		while (p->getParentAsContainer()) {
			p = p->getParentAsContainer();
		}

		topitem = p;
	}

	Gump *gump = GetRootGump()->FindGump(GameMapGump::ClassType);
	assert(gump);
	gump->GetLocationOfItem(topitem->getObjId(), gx, gy, lerp_factor);

	// Convert the GumpSpaceCoord relative to the world/item gump
	// into screenspace coords
	gy = gy-it->getShapeInfo()->z*8-16;
	gump->GumpToScreenSpace(gx,gy);

	// Convert the screenspace coords into the coords of us
	if (parent) parent->ScreenSpaceToGump(gx,gy);

	// Set x and y, and center us over it
	ix = gx-dims.w/2;
	iy = gy-dims.h;
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
	Gump* handled = Gump::OnMouseDown(button, mx, my);
	if (handled) return handled;

	// only interested in left clicks
	if (button == BUTTON_LEFT)
		return this;

	return 0;
}

void ContainerGump::OnMouseClick(int button, int mx, int my)
{
	if (button == BUTTON_LEFT)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			return;
		}
		
		uint16 objID = TraceObjId(mx, my);

		World *world = World::get_instance();
		Item *item = world->getItem(objID);
		if (item) {
			item->dumpInfo();
			
			// call the 'look' event
			item->callUsecodeEvent_look();
		}
	}
}

void ContainerGump::OnMouseDouble(int button, int mx, int my)
{
	if (button == BUTTON_LEFT)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			return;
		}
		
		uint16 objID = TraceObjId(mx, my);

		if (objID == getObjId()) {
			objID = owner; // use container when double click on self
		}

		World *world = World::get_instance();
		Item *item = world->getItem(objID);
		if (item) {
			item->dumpInfo();

			MainActor* avatar = World::get_instance()->getMainActor();
			if (objID == owner || avatar->canReach(item, 128)) { // CONSTANT!
				// call the 'use' event
				item->use();
			} else {
				GUIApp::get_instance()->flashCrossCursor();
			}			
		}		
	}
}


bool ContainerGump::StartDraggingItem(Item* item, int mx, int my)
{
	// probably don't need to check if item can be moved, since it shouldn't
	// be in a container otherwise

	// check if the container the item is in is in range
	MainActor* avatar = World::get_instance()->getMainActor();
	if (!avatar->canReach(item, 128)) return false;

	sint32 itemx,itemy;
	item->getGumpLocation(itemx,itemy);

	itemx += itemarea.x;
	itemy += itemarea.y;

	GUIApp::get_instance()->setDraggingOffset(mx - itemx, my - itemy);

	return true;
}

bool ContainerGump::DraggingItem(Item* item, int mx, int my)
{
	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));
	assert(c);

	// check if the container the item is in is in range
	MainActor* avatar = World::get_instance()->getMainActor();
	if (!avatar->canReach(c, 128)) {
		display_dragging = false;
		return false;
	}

	int dox, doy;
	GUIApp::get_instance()->getDraggingOffset(dox, doy);	
	GUIApp::get_instance()->setMouseCursor(GUIApp::MOUSE_TARGET);
	display_dragging = true;

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();

	// determine target location and set dragging_x/y

	dragging_x = mx - itemarea.x - dox;
	dragging_y = my - itemarea.y - doy;

	if (dragging_x < 0 || dragging_x >= itemarea.w ||
		dragging_y < 0 || dragging_y >= itemarea.h) {
		display_dragging = false;
		return false;
	}

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
	Item* targetitem = World::get_instance()->getItem(TraceObjId(px, py));
	Container* targetcontainer = p_dynamic_cast<Container*>(targetitem);


	if (item->getShapeInfo()->hasQuantity() &&
		item->getQuality() > 1)
	{
		// more than one, so see if we should ask if we should split it up

		Item* splittarget = 0;

		// also try to combine
		int family = item->getShapeInfo()->family;
		// Note: can only combine SF_REAGENTS when frame is equal
		// see U8's usecode FREE::2767
		if (targetitem && targetitem->getShape() == item->getShape() &&
			(targetitem->getFrame() == item->getFrame() ||
			 family == ShapeInfo::SF_QUANTITY))
		{
			splittarget = targetitem;
		}

		if (!splittarget) {
			// create new item
			splittarget = ItemFactory::createItem(
				item->getShape(), item->getFrame(), 0,
				item->getFlags() & (Item::FLG_DISPOSABLE | Item::FLG_OWNED | Item::FLG_INVISIBLE | Item::FLG_FLIPPED | Item::FLG_FAST_ONLY | Item::FLG_LOW_FRICTION), item->getNpcNum(), item->getMapNum(),
				item->getExtFlags() & (Item::EXT_SPRITE | Item::EXT_HIGHLIGHT | Item::EXT_TRANSPARENT));
			if (!splittarget) {
				perr << "ContainerGump failed to create item ("
					 << item->getShape() << "," << item->getFrame()
					 << ") while splitting" << std::endl;
				return;
			}
			splittarget->assignObjId();


			if (targetcontainer) {
				splittarget->moveToContainer(targetcontainer);
				splittarget->setGumpLocation(0, 0); //TODO: randomize!
			} else {
				splittarget->moveToContainer(p_dynamic_cast<Container*>(
									World::get_instance()->getItem(owner)));
				splittarget->setGumpLocation(dragging_x, dragging_y);
			}
		}			

		SliderGump* slidergump = new SliderGump(100, 100,
												0, item->getQuality(),
												item->getQuality());
		slidergump->InitGump();
		GUIApp::get_instance()->addGump(slidergump);
		slidergump->CreateNotifier(); // manually create notifier
		Process* notifier = slidergump->GetNotifyProcess();
		SplitItemProcess* splitproc = new SplitItemProcess(item, splittarget);
		Kernel::get_instance()->addProcess(splitproc);
		splitproc->waitFor(notifier);

		return;
	}

	if (targetitem && item->getShapeInfo()->hasQuantity())
	{
		// try to combine items
		int family = item->getShapeInfo()->family;
		if (targetitem->getShape() == item->getShape() &&
			(targetitem->getFrame() == item->getFrame() ||
			 family == ShapeInfo::SF_QUANTITY))
		{
			// Note: can only combine SF_REAGENTS when frame is equal
			// see U8's usecode FREE::2767
			targetitem->setQuality(targetitem->getQuality() +
								   item->getQuality());
			targetitem->callUsecodeEvent_combine();
			
			// combined, so delete item
			item->destroy(); item = 0;
			return;
		}
	}


	if (targetcontainer) {
		// do not move to containers marked as land or is an NPC,
		// just in case you decided to start collecting bodies in trunks
		
		// Note: this is taking advantage of keyring being marked as land
		ShapeInfo * targetinfo = targetcontainer->getShapeInfo();
		if (! targetinfo->is_land() &&
			! (targetcontainer->getFlags() & Item::FLG_IN_NPC_LIST))
		{
			item->moveToContainer(targetcontainer);
			item->setGumpLocation(0, 0); //TODO: randomize!
			return;
		}
	}

	targetcontainer = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	// add item to container
	assert(targetcontainer);

	if (item->getParent() == owner) {
		// already in this container, so move item to let it be drawn
		// on top of all other items
		targetcontainer->moveItemToEnd(item);
	} else {
		item->moveToContainer(targetcontainer);
	}

	//!! TODO: this is nonsense when not adding to this container

	int dox, doy;
	GUIApp::get_instance()->getDraggingOffset(dox, doy);
	dragging_x = mx - itemarea.x - dox;
	dragging_y = my - itemarea.y - doy;
	item->setGumpLocation(dragging_x, dragging_y);
}

void ContainerGump::saveData(ODataSource* ods)
{
	ItemRelativeGump::saveData(ods);

	ods->write4(static_cast<uint32>(itemarea.x));
	ods->write4(static_cast<uint32>(itemarea.y));
	ods->write4(static_cast<uint32>(itemarea.w));
	ods->write4(static_cast<uint32>(itemarea.h));
}

bool ContainerGump::loadData(IDataSource* ids, uint32 version)
{
	if (!ItemRelativeGump::loadData(ids, version)) return false;

	sint32 iax = static_cast<sint32>(ids->read4());
	sint32 iay = static_cast<sint32>(ids->read4());
	sint32 iaw = static_cast<sint32>(ids->read4());
	sint32 iah = static_cast<sint32>(ids->read4());
	itemarea.Set(iax, iay, iaw, iah);

	return true;
}
