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

#include "GameMapGump.h"
#include "RenderSurface.h"

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
#include "Mouse.h"

#include "GravityProcess.h" // hack...
#include "PathfinderProcess.h"
#include "AvatarMoverProcess.h"
#include "UCList.h"
#include "LoopScript.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(GameMapGump,Gump);

GameMapGump::GameMapGump() :
	Gump(), display_dragging(false), leftDownToAvatarMover(false)
{
	display_list = new ItemSorter();
}

GameMapGump::GameMapGump(int X, int Y, int Width, int Height) :
	Gump(X,Y,Width,Height, 0, 0, LAYER_GAMEMAP),
	display_list(0), display_dragging(false)
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
		av->getFootpadWorld(axd, ayd, azd);
		map->isValidPosition(ax, ay, az, 32, 32, 8, 1, 0, &roofid);
	}
	else
		roofid = camera->FindRoof(lerp_factor);

	Item* roof = world->getItem(roofid);
	if (roof) {
		zlimit = roof->getZ();
	}

	display_list->BeginDisplayList(surf, lx, ly, lz);

	uint32 gametick = Kernel::get_instance()->getFrameNum();

	bool paintEditorItems = GUIApp::get_instance()->isPaintEditorItems();

	// Get all the required items
	for (int cy = 0; cy < MAP_NUM_CHUNKS; cy++)
	{
		for (int cx = 0; cx < MAP_NUM_CHUNKS; cx++)
		{
			sint32 sx = x - y;
			sint32 sy = x + y;

			// Not fast, ignore
			if (!map->isChunkFast(cx,cy)) continue;

			const std::list<Item*>* items = map->getItemList(cx,cy);

			if (!items) continue;

			std::list<Item*>::const_iterator it = items->begin();
			std::list<Item*>::const_iterator end = items->end();
			for (; it != end; ++it)
			{
				Item *item = *it;
				if (!item) continue;

				item->setupLerp(gametick);
				item->doLerp(lerp_factor);

				if (item->getZ() >= zlimit && !item->getShapeInfo()->is_draw())
					continue;
				if (paintEditorItems && item->getShapeInfo()->is_editor())
					continue;
				display_list->AddItem(item);
			}
		}
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

// Trace a click, and return ObjId
uint16 GameMapGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx,my);
	if (objid && objid != 65535) return objid;

	ParentToGump(mx,my);
	return display_list->Trace(mx,my);
}

bool GameMapGump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
									sint32 lerp_factor)
{
	Item *item = World::get_instance()->getItem(itemid);

	if (!item) return false;

	while (item->getParentAsContainer()) item = item->getParentAsContainer();

	sint32 ix, iy, iz;

	// Hacks be us. Force the item into the fast area
	item->setupLerp(Kernel::get_instance()->getFrameNum());
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
	int sx = mx, sy = my;
	GumpToScreenSpace(sx, sy);

	AvatarMoverProcess* amp = GUIApp::get_instance()->getAvatarMoverProcess();
	if (button == BUTTON_RIGHT) {
		amp->OnMouseDown(button, sx, sy);
	}

	if (button == BUTTON_LEFT &&
		GUIApp::get_instance()->isMouseDown(BUTTON_RIGHT))
	{
		// if right button is down, AvatarMoverProcess wants left clicks too
		amp->OnMouseDown(button, sx, sy);
		leftDownToAvatarMover = true;
	} else if (button == BUTTON_LEFT) {
		leftDownToAvatarMover = false;
	}

	if (button == BUTTON_LEFT || button == BUTTON_RIGHT ||
		button == BUTTON_MIDDLE)
	{
		// we take all clicks
		return this;
	}

	return 0;
}

void GameMapGump::OnMouseUp(int button, int mx, int my)
{
	AvatarMoverProcess* amp = GUIApp::get_instance()->getAvatarMoverProcess();
	if (button == BUTTON_RIGHT) {
		amp->OnMouseUp(button);
	}

	if (button == BUTTON_LEFT && leftDownToAvatarMover) {
		amp->OnMouseUp(button);
	}
}

void GameMapGump::OnMouseClick(int button, int mx, int my)
{
	extern uint16 targetObject; // major hack number 2

	switch (button) {
	case BUTTON_LEFT:
	{
		if (leftDownToAvatarMover) break;

		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl;
			uint16 objID = TraceObjId(mx, my); //!! hack
			targetObject = objID;
			break;
		}

		uint16 objID = TraceObjId(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			targetObject = objID; //!! hack

			sint32 x,y,z;
			item->getLocation(x,y,z);
			item->dumpInfo();
			
			// call the 'look' event
			item->callUsecodeEvent_look();
		}
		break;
	}
	case BUTTON_MIDDLE:
	{
		uint16 objID = TraceObjId(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			sint32 x,y,z;
			item->getLocation(x,y,z);
			item->dumpInfo();

#if 0
			Actor* devon = World::get_instance()->getNPC(1);
			PathfinderProcess* pfp = new PathfinderProcess(devon, x, y, z);
			Kernel::get_instance()->addProcess(pfp);
#elif 0
			item->explode();
#else
			UCList uclist(2);
			LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
			World* world= World::get_instance();
			world->getCurrentMap()->surfaceSearch(&uclist, script,
												  sizeof(script),
												  item, true, false, true);
			for (uint32 i = 0; i < uclist.getSize(); i++)
			{
				Item *item2 = world->getItem(uclist.getuint16(i));
				if (!item2) continue;
				item2->setExtFlag(Item::EXT_HIGHLIGHT);
			}
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
	case BUTTON_LEFT:
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			break;
		}

		uint16 objID = TraceObjId(mx, my);
		Item *item = World::get_instance()->getItem(objID);
		if (item) {
			sint32 x,y,z;
			item->getLocation(x,y,z);
			item->dumpInfo();
			
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
		
	item->move(dragging_x,dragging_y,dragging_z); // move
	item->fall();
}

void GameMapGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);
}

bool GameMapGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	return true;
}


// Colourless Protection
