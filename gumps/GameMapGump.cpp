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

#include <SDL.h>

#include "GameMapGump.h"
#include "RenderSurface.h"

#include "CoreApp.h"
#include "Kernel.h"

#include "World.h"
#include "Map.h"
#include "CurrentMap.h"
#include "Item.h"
#include "Actor.h"
#include "MainActor.h"
#include "ItemSorter.h"
#include "CameraProcess.h"
#include "GUIApp.h"
#include "ShapeInfo.h"
#include "IDataSource.h"
#include "ODataSource.h"

#include "GravityProcess.h" // hack...

DEFINE_RUNTIME_CLASSTYPE_CODE(GameMapGump,Gump);

GameMapGump::GameMapGump() :
	Gump(), display_dragging(false)
{
	display_list = new ItemSorter();
}

GameMapGump::GameMapGump(int X, int Y, int Width, int Height) :
	Gump(X,Y,Width,Height, 0, 0, LAYER_GAMEMAP),
	display_list(0), fastArea(0), display_dragging(false)
{
	// Offset us the gump. We want 0,0 to be the centre
	dims.x -= dims.w/2;
	dims.y -= dims.h/2;

	pout << "Create display_list ItemSorter object" << std::endl;
	display_list = new ItemSorter();
}

GameMapGump::~GameMapGump()
{
#if 0
	World *world = World::get_instance();
	if (!world) return;	// Is it possible the world doesn't exist?

	// All items leave the fast area on close
	std::vector<uint16>::iterator it  = fastAreas[fastArea].begin();
	std::vector<uint16>::iterator end  = fastAreas[fastArea].end();

	for (;it != end; ++it)
	{
		Item *item = world->getItem(*it);

		// Not an item, continue
		if (!item) continue;

		// leave the Fast area
		item->leavingFastArea();
	}
#endif

	delete display_list;
}

bool GameMapGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	return true; // Always repaint, even though we really could just
	             // try to detect it
}

void GameMapGump::MapChanged()
{
	Gump::MapChanged();

	World *world = World::get_instance();
	if (!world) return;	// Is it possible the world doesn't exist?

	// All items leave the fast area on close
	std::vector<uint16>::iterator it  = fastAreas[fastArea].begin();
	std::vector<uint16>::iterator end  = fastAreas[fastArea].end();

	for (;it != end; ++it)
	{
		Item *item = world->getItem(*it);

		// Not an item, continue
		if (!item) continue;

		// set the Fast area left
		item->clearFlag(Item::FLG_FASTAREA);
		item->clearExtFlag(Item::EXT_FAST0|Item::EXT_FAST1);
	}

	// Clear the fast areas
	fastAreas[0].clear();
	fastAreas[1].clear();
}

void GameMapGump::GetCameraLocation(sint32& lx, sint32& ly, sint32& lz,
									int lerp_factor)
{
	CameraProcess *camera = CameraProcess::GetCameraProcess();
	if (!camera) 
	{
		CameraProcess::GetCameraLocation(lx,ly,lz);
	}
	else
	{
		camera->GetLerped(lx, ly, lz, lerp_factor);
	}
}


void GameMapGump::PaintThis(RenderSurface *surf, sint32 lerp_factor)
{
	World *world = World::get_instance();
	if (!world) return;	// Is it possible the world doesn't exist?

	CurrentMap *map = world->getCurrentMap();
	if (!map) return;	// Is it possible the map doesn't exist?


	// Get the camera location
	int lx, ly, lz;
	GetCameraLocation(lx, ly, lz, lerp_factor);

	CameraProcess *camera = CameraProcess::GetCameraProcess();

	uint16 roofid = 0;
	int zlimit = 1 << 16; // should be high enough

	if (!camera)
	{
		// Check roof
		//!! This is _not_ the right place for this...
		sint32 ax, ay, az, axd, ayd, azd;
		Actor* av = world->getNPC(1);
		av->getLocation(ax, ay, az);
		av->getFootpad(axd, ayd, azd);
		map->isValidPosition(ax, ay, az, 32, 32, 8, 1, 0, &roofid);
	}
	else
		roofid = camera->FindRoof(lerp_factor);

	Item* roof = world->getItem(roofid);
	if (roof) {
		zlimit = roof->getZ();
	}

	display_list->BeginDisplayList(surf, lx, ly, lz);


	sint32 resx = dims.w, resy = dims.h;

	/*
	at 640x480, with u8 sizes
	sint32 sx_limit = 4;
	sint32 sy_limit = 8;
	sint32 xy_limit = 6;

	at 320x240, with u8 sizes
	sint32 sx_limit = 3;
	sint32 sy_limit = 7;
	sint32 xy_limit = 4;

	for tgwds use half the resolution
	*/

	//if (tgwds)
	//{
	//	resx/=2;
	//	resy/=2;
	//}

	// By default the fastArea is the screensize plus a border of no more
	// than 256 pixels wide and 384 pixels high

	sint32 sx_limit = resx/256 + 3;
	sint32 sy_limit = resy/128 + 7;
	sint32 xy_limit = (sy_limit+sx_limit)/2;

	// Flip the fast area
	fastArea=1-fastArea;

	std::vector<uint16> *prev_fast = &fastAreas[1-fastArea];
	std::vector<uint16> *fast = &fastAreas[fastArea];
	fast->erase(fast->begin(), fast->end());

	sint32 gx = lx/512;
	sint32 gy = ly/512;

	uint32 framenum = CoreApp::get_instance()->getFrameNum();

	bool paintEditorItems = GUIApp::get_instance()->isPaintEditorItems();

	// Get all the required items
	for (int y = -xy_limit; y <= xy_limit; y++)
	{
		for (int x = -xy_limit; x <= xy_limit; x++)
		{
			sint32 sx = x - y;
			sint32 sy = x + y;

			if (sx < -sx_limit || sx > sx_limit ||
				sy < -sy_limit || sy > sy_limit)
				continue;

			const std::list<Item*>* items = map->getItemList(gx+x,gy+y);

			if (!items) continue;

			std::list<Item*>::const_iterator it = items->begin();
			std::list<Item*>::const_iterator end = items->end();
			for (; it != end; ++it)
			{
				Item *item = *it;
				if (!item) continue;

				item->inFastArea(fastArea, framenum);
				fast->push_back(item->getObjId());
				item->doLerp(lerp_factor);

				if (item->getZ() >= zlimit && !item->getShapeInfo()->is_draw())
					continue;
				if (paintEditorItems && item->getShapeInfo()->is_editor())
					continue;
				display_list->AddItem(item);
			}
		}
	}

	// Now handle leaving the fast area
	std::vector<uint16>::iterator it  = prev_fast->begin();
	std::vector<uint16>::iterator end  = prev_fast->end();

	for (;it != end; ++it)
	{
		Item *item = world->getItem(*it);

		// Not an item, continue
		if (!item) continue;

		// If the fast area for the item is the current one, continue
		if (item->getExtFlags() & (Item::EXT_FAST0<<fastArea)) continue;

		// Ok, we must leave te Fast area
		item->leavingFastArea();
	}


	// Dragging:

	if (display_dragging) {
		display_list->AddItem(dragging_x, dragging_y, dragging_z,
							  dragging_shape, dragging_frame,
							  Item::FLG_INVISIBLE, //!! change this to transp?
							  0);
	}


	display_list->PaintDisplayList();
}

void GameMapGump::FlushFastArea()
{
	World *world = World::get_instance();
	std::vector<uint16> *prev_fast = &fastAreas[fastArea];

	// Now handle leaving the fast area
	std::vector<uint16>::iterator it  = prev_fast->begin();
	std::vector<uint16>::iterator end  = prev_fast->end();

	for (;it != end; ++it)
	{
		Item *item = world->getItem(*it);

		// Not an item, continue
		if (!item) continue;

		// Ok, we must leave te Fast area
		item->leavingFastArea();
	}
	prev_fast->clear();
}

// Trace a click, and return ObjID
uint16 GameMapGump::TraceObjID(int mx, int my)
{
	uint16 objid = Gump::TraceObjID(mx,my);
	if (objid && objid != 65535) return objid;

	ParentToGump(mx,my);
	return display_list->Trace(mx,my);
}

bool GameMapGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
									sint32 lerp_factor)
{
	Item *item = World::get_instance()->getItem(itemid);

	if (!item) return false;

	while (item->getParent()) item = item->getParent();

	sint32 ix, iy, iz;

	// Hacks be us. Force the item into the fast area
	if (!(item->getExtFlags() & (Item::EXT_FAST0<<fastArea))) 
	{
		item->inFastArea(fastArea, framenum);
		fastAreas[fastArea].push_back(item->getObjId());
	}
	item->doLerp(lerp_factor);
	item->getLerped(ix,iy,iz);

	// Get the camera's location
	sint32 cx, cy, cz;
		CameraProcess *cam = CameraProcess::GetCameraProcess();
	if (!cam) CameraProcess::GetCameraLocation(cx,cy,cz);
	else cam->GetLerped(cx,cy,cz,lerp_factor);

	// Screenspace bounding box bottom x coord (RNB x coord)
	gx = (ix - iy)/4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	gy = (ix + iy)/8 - iz;

	// Screenspace bounding box bottom x coord (RNB x coord)
	gx -= (cx - cy)/4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	gy -= (cx + cy)/8 - cz;

	return true;
}

Gump* GameMapGump::OnMouseDown(int button, int mx, int my)
{
//	if (button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT)
		return this;

	return 0;
}

void GameMapGump::OnMouseClick(int button, int mx, int my)
{
	extern uint16 targetObject; // major hack number 2

	switch (button) {
	case SDL_BUTTON_LEFT:
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl;
			uint16 objID = TraceObjID(mx, my); //!! hack
			targetObject = objID;
			break;
		}

		uint16 objID = TraceObjID(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			targetObject = objID; //!! hack

			sint32 x,y,z;
			item->getLocation(x,y,z);
			pout << "Found item " << objID << " (shape " << item->getShape()
				 << ", " << item->getFrame() << ", (" << x << "," << y << ","
				 << z << ") q:" << item->getQuality() << ", m:"
				 << item->getMapNum() << ", n:" << item->getNpcNum()
				 << ")" << std::endl;
			
			// call the 'look' event
			item->callUsecodeEvent_look();
		}
		break;
	}
	case SDL_BUTTON_MIDDLE:
	{
		uint16 objID = TraceObjID(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			sint32 x,y,z;
			item->getLocation(x,y,z);
			pout << "Found item " << objID << " (shape " << item->getShape()
				 << ", " << item->getFrame() << ", (" << x << "," << y << ","
				 << z << ") q:" << item->getQuality() << ", m:"
				 << item->getMapNum() << ", n:" << item->getNpcNum()
				 << ")" << std::endl;
#if 0
			item->setFlag(Item::FLG_INVISIBLE);
#else
			GravityProcess* p = new GravityProcess(item, 4);
			Kernel::get_instance()->addProcess(p);
			p->init();
			p->move(0, -32, 32);
#endif
		}
	}
	default:
		break;
	}
}

void GameMapGump::OnMouseDouble(int button, int mx, int my)
{
	switch (button) {
	case SDL_BUTTON_LEFT:
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			break;
		}

		uint16 objID = TraceObjID(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			sint32 x,y,z;
			item->getLocation(x,y,z);
			pout << "Found item " << objID << " (shape " << item->getShape()
				 << ", " << item->getFrame() << ", (" << x << "," << y << ","
				 << z << ") q:" << item->getQuality() << ", m:"
				 << item->getMapNum() << ", n:" << item->getNpcNum()
				 << ")" << std::endl;
			
			//!! need to check range
			
			// call the 'use' event
			item->callUsecodeEvent_use();
		}
		break;
	}
	default:
		break;
	}
}

void GameMapGump::IncSortOrder(int count)
{
	if (count>0) display_list->IncSortLimit();
	else display_list->DecSortLimit();
}

bool GameMapGump::StartDraggingItem(Item* item, int mx, int my)
{
//	ParentToGump(mx, my);

	// check if item can be moved
	ShapeInfo* si = item->getShapeInfo();
	if (si->is_fixed()) return false;
	//!! need more checks here

	// check if item is in range

	return true;
}

bool GameMapGump::DraggingItem(Item* item, int mx, int my)
{
	display_dragging = true;
	// determine target location and set dragging_x/y/z

	dragging_shape = item->getShape();
	dragging_frame = item->getFrame();
	dragging_flags = item->getFlags();
	display_dragging = true;

	sint32 cx, cy, cz;
	GetCameraLocation(cx, cy, cz);

	//!! hack...
	dragging_z = 128;
	dragging_x = 2*mx + 4*(my+128) + cx - 4*cz;
	dragging_y = -2*mx + 4*(my+128) + cy - 4*cz;

	//TODO: determine if item can be dropped here

	return true;
}

void GameMapGump::DraggingItemLeftGump(Item* item)
{
	display_dragging = false;
}


void GameMapGump::StopDraggingItem(Item* item, bool moved)
{
	display_dragging = false;

	if (!moved) return; // nothing to do

	// make items on top of item fall
	item->grab();
	// remove item from world
	CurrentMap* cm = World::get_instance()->getCurrentMap();
	cm->removeItem(item);
}

void GameMapGump::DropItem(Item* item, int mx, int my)
{
	display_dragging = false;

	// add item to world 

	//!! TODO: throw item if too far, etc...

	// hackety-hack
	sint32 cx, cy, cz;
	GetCameraLocation(cx, cy, cz);
	dragging_z = 128;
	dragging_x = 2*mx + 4*(my+128) + cx - 4*cz;
	dragging_y = -2*mx + 4*(my+128) + cy - 4*cz;

	perr << "Dropping item at (" << dragging_x << "," << dragging_y 
		 << "," << dragging_z << ")" << std::endl;
		

	item->move(dragging_x,dragging_y,dragging_z, true); // move, and force
	                                                    // map update
	item->fall();
}

void GameMapGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);

	ods->write4(static_cast<uint32>(fastArea));
	ods->write4(fastAreas[0].size());
	for (unsigned int i = 0; i < fastAreas[0].size(); ++i) {
		ods->write2(fastAreas[0][i]);
	}
	ods->write4(fastAreas[1].size());
	for (unsigned int i = 0; i < fastAreas[1].size(); ++i) {
		ods->write2(fastAreas[1][i]);
	}
}

bool GameMapGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	fastArea = static_cast<int>(ids->read4());
	unsigned int fastcount = ids->read4();
	fastAreas[0].resize(fastcount);
	for (unsigned int i = 0; i < fastcount; ++i) {
		fastAreas[0][i] = ids->read2();
	}
	fastcount = ids->read4();
	fastAreas[1].resize(fastcount);
	for (unsigned int i = 0; i < fastcount; ++i) {
		fastAreas[1][i] = ids->read2();
	}

	return true;
}


// Colourless Protection
