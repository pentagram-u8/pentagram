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
#include "ContainerGump.h"

#include "Shape.h"
#include "ShapeFrame.h"
#include "Container.h"
#include "World.h"
#include "RenderSurface.h"
#include "GUIApp.h"

//! see comments in Paint()
//! (these need to be read from gumpage.dat)
static const int itemx_offset = 20;
static const int itemy_offset = 18; 

DEFINE_RUNTIME_CLASSTYPE_CODE(ContainerGump,ItemRelativeGump);

ContainerGump::ContainerGump(Shape* shape_, uint32 framenum_, uint16 owner,
							 uint32 Flags_, sint32 layer)
	: ItemRelativeGump(0, 0, 5, 5, owner, Flags_, layer)
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

	// Position isn't like in the original
	// U8 puts a container gump slightly to the left of an object
}

void ContainerGump::Paint(RenderSurface* surf, sint32 lerp_factor)
{
	// paint self
	ItemRelativeGump::Paint(surf, lerp_factor);

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) {
		// Container gone!?
		Close();
		return;
	}

	std::list<Item*>& contents = c->contents;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		sint32 itemx,itemy;
		item->getGumpLocation(itemx,itemy);
		// Where do we need to paint his item?
		// It looks like(itemx,itemy) isn't entirely correct;
		// it seems to need an extra offset.
		//   (20,14) seems to be correct for a basket, but a barrel
        //   needs (20,18)
		// Using constant 'itemx_offset', 'itemy_offset' currently.
		// Need to change these to variables.
		itemx += itemx_offset;
		itemy += itemy_offset;
		GumpToParent(itemx,itemy);
		Shape* s = item->getShapeObject();
		assert(s);
		surf->Paint(s, item->getFrame(), itemx, itemy);
	}
	
}

// Find object (if any) at (mx,my)
// (mx,my) are relative to parent
uint16 ContainerGump::TraceObjID(int mx, int my)
{
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

		if (frame->hasPoint(mx - (itemx + itemx_offset),
							my - (itemy + itemy_offset)))
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
	//!!! need to use lerp_factor

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return 0; // Container gone!?

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		if (item->getObjId() == itemid) {
			// found it
			item->getGumpLocation(gx,gy);
			gx += itemx_offset;
			gy += itemy_offset;
			return true;
		}
	}

	return false;
}

void ContainerGump::Close(bool no_del)
{
	// close any gumps belonging to contents
	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) return; // Container gone!?

	std::list<Item*>& contents = c->contents;
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		if (item->getGump()) {
			item->getGump()->Close(); //!! what about no_del?
		}
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

			pout << "Found item " << objID << " (shape " << item->getShape() << ", " << item->getFrame() << ", q:" << item->getQuality() << ", m:" << item->getMapNum() << ", n:" << item->getNpcNum() << ")" << std::endl;
			
			// call the 'look' event
			item->callUsecodeEvent(0);	// CONSTANT
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
			pout << "Found item " << objID << " (shape " << item->getShape() << ", " << item->getFrame() << ", q:" << item->getQuality() << ", m:" << item->getMapNum() << ", n:" << item->getNpcNum() << ")" << std::endl;
			
			// call the 'use' event
			item->callUsecodeEvent(1);	// CONSTANT
		}		
	}
}
