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

DEFINE_RUNTIME_CLASSTYPE_CODE(GameMapGump,Gump);

GameMapGump::GameMapGump(int X, int Y, int Width, int Height) :
	Gump(X,Y,Width,Height, 0, 0, LAYER_GAMEMAP),
	display_list(0), fastArea(0),
	last_left_state(MBS_HANDLED), last_right_state(MBS_HANDLED)
{
	// Offset us the gump. We want 0,0 to be the centre
	dims.x -= dims.w/2;
	dims.y -= dims.h/2;

	pout << "Create display_list ItemSorter object" << std::endl;
	display_list = new ItemSorter();
}

GameMapGump::~GameMapGump()
{
	World *world = World::get_instance();
	if (!world) return;	// Is it possible the world doesn't exist?

	// All items leave the fast area on close
	std::vector<uint16>::iterator it  = fastAreas[fastArea].begin();
	std::vector<uint16>::iterator end  = fastAreas[fastArea].end();

	for (;it != end; ++it)
	{
		Object *obj = world->getObject(*it);

		// No object, continue
		if (!obj) continue;

		Item *item = p_dynamic_cast<Item*>(obj);

		// Not an item, continue
		if (!item) continue;

		// leave the Fast area
		item->leavingFastArea();
	}
}

bool GameMapGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	uint32 now = SDL_GetTicks();
	// check if there's a mouse click waiting to be handled
	//! constant
	if (last_left_state == MBS_UP && now - last_left_down > 200) {
		handleMouseClick(SDL_BUTTON_LEFT, last_left_mx, last_left_my);

		last_left_state = MBS_HANDLED;
	}
	if (last_right_state == MBS_UP && now - last_right_down > 200) {
		handleMouseClick(SDL_BUTTON_RIGHT, last_right_mx, last_right_my);

		last_right_state = MBS_HANDLED;
	}

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
		Object *obj = world->getObject(*it);

		// No object, continue
		if (!obj) continue;

		Item *item = p_dynamic_cast<Item*>(obj);

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

void GameMapGump::PaintThis(RenderSurface *surf, sint32 lerp_factor)
{
	World *world = World::get_instance();
	if (!world) return;	// Is it possible the world doesn't exist?

	CurrentMap *map = world->getCurrentMap();
	if (!map) return;	// Is it possible the map doesn't exist?


	// Get the camera location
	int lx, ly, lz;
	CameraProcess *camera = CameraProcess::GetCameraProcess();
	if (!camera) {

		int map_num = map->getNum();
		Actor* av = world->getNPC(1);
		
		if (!av || av->getMapNum() != map_num)
		{
			lx = 8192;
			ly = 8192;
			lz = 64;
		}
		else
			av->getLocation(lx,ly,lz);
	}
	else
	{
		camera->GetLerped(lx, ly, lz, lerp_factor);
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
				display_list->AddItem(item);
			}
		}
	}

	// Now handle leaving the fast area
	std::vector<uint16>::iterator it  = prev_fast->begin();
	std::vector<uint16>::iterator end  = prev_fast->end();

	for (;it != end; ++it)
	{
		Object *obj = world->getObject(*it);

		// No object, continue
		if (!obj) continue;

		Item *item = p_dynamic_cast<Item*>(obj);

		// Not an item, continue
		if (!item) continue;

		// If the fast area for the item is the current one, continue
		if (item->getExtFlags() & (Item::EXT_FAST0<<fastArea)) continue;

		// Ok, we must leave te Fast area
		item->leavingFastArea();
	}



	display_list->PaintDisplayList();
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

	sint32 ix, iy, iz;
	item->getLocationAbsolute(ix,iy,iz);

	// Get the camera's location
	sint32 cx, cy, cz;
	CameraProcess *cam = CameraProcess::GetCameraProcess();
	if (!cam) CameraProcess::GetCameraLocation(cx,cy,cz);
	else cam->GetLerped(cx,cy,cz,lerp_factor);

	ix -= cx;
	iy -= cy;
	iz -= cz;

	// Screenspace bounding box bottom x coord (RNB x coord)
	gx = (ix - iy)/4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	gy = (ix + iy)/8 - iz;

	return true;
}

Gump* GameMapGump::OnMouseDown(int button, int mx, int my)
{
	uint32 now = SDL_GetTicks();
    Gump* handled = Gump::OnMouseDown(button, mx, my);

	if (!handled) {
		handled = this;

		switch(button) {
		case SDL_BUTTON_LEFT:
			// setup delayed left click
			last_left_down = now;
			last_left_state = MBS_DOWN;
			break;
		case SDL_BUTTON_RIGHT:
			// setup delayed right click
			last_right_down = now;
			last_right_state = MBS_DOWN;
			break;
		default:
			break;
		}
	}

	return handled;
}

void GameMapGump::OnMouseUp(int  button, int mx, int my)
{
	uint32 now = SDL_GetTicks();
	switch (button) {
	case SDL_BUTTON_LEFT:
		if (last_left_state == MBS_HANDLED) break;
		//! constant
		if (now - last_left_down > 200) {
			handleMouseClick(button, mx, my);
			last_left_state = MBS_HANDLED;
		} else {
			// setup delayed left click
			last_left_state = MBS_UP;
			last_left_mx = mx;
			last_left_my = my;
		}
		break;
	case SDL_BUTTON_RIGHT:
		if (last_right_state == MBS_HANDLED) break;
		//! constant
		if (now - last_right_down > 200) {
			handleMouseClick(button, mx, my);
			last_right_state = MBS_HANDLED;
		} else {
			// setup delayed right click
			last_right_state = MBS_UP;
			last_right_mx = mx;
			last_right_my = my;
		}
		break;
	default:
		// no delayed clicks for other buttons
		handleMouseClick(button, mx, my);
		break;
	}
}


void GameMapGump::OnMouseDouble(int button, int mx, int my)
{
	switch(button) {
	case SDL_BUTTON_LEFT:
		last_left_state = MBS_HANDLED;
		break;
	case SDL_BUTTON_RIGHT:
		last_right_state = MBS_HANDLED;
		break;
	default:
		break;
	}
	handleMouseDoubleClick(button, mx, my);
}

void GameMapGump::handleMouseClick(int button, int mx, int my)
{
	World* world = World::get_instance();
	perr << "Click! (" << button << ")" << std::endl;

	switch (button) {
	case SDL_BUTTON_LEFT:
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			break;
		}

		uint16 objID = TraceObjID(mx, my);

		Item *item = world->getItem(objID);
		Gump *gump = p_dynamic_cast<Gump*>(world->getObject(objID));
		if (item) {
			extern uint16 targetObject; // major hack number 2
			targetObject = objID;

			pout << "Found item " << objID << " (shape " << item->getShape() << ", " << item->getFrame() << ", q:" << item->getQuality() << ", m:" << item->getMapNum() << ", n:" << item->getNpcNum() << ")" << std::endl;
			
			if (item) {
				// call the 'look' event
				item->callUsecodeEvent(0);	// CONSTANT
			}
		} else if (gump) {
			pout << "Found gump " << objID << std::endl;
		}
		break;
	}
	default:
		break;
	}
}

void GameMapGump::handleMouseDoubleClick(int button, int mx, int my)
{
	World* world = World::get_instance();
	perr << "DoubleClick! (" << button << ")" << std::endl;

	switch (button) {
	case SDL_BUTTON_LEFT:
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) {
			pout << "Can't: avatarInStasis" << std::endl; 
			break;
		}

		uint16 objID = TraceObjID(mx, my);

		Item *item = world->getItem(objID);
		Gump *gump = p_dynamic_cast<Gump*>(world->getObject(objID));
		if (item) {
			pout << "Found item " << objID << " (shape " << item->getShape() << ", " << item->getFrame() << ", q:" << item->getQuality() << ", m:" << item->getMapNum() << ", n:" << item->getNpcNum() << ")" << std::endl;
			
			if (item) {
				//!! need to check range

				// call the 'use' event
				item->callUsecodeEvent(1);	// CONSTANT
			}
		} else if (gump) {
			pout << "Found gump " << objID << std::endl;

			//!! No, this isn't correct
			gump->Close();
		}
		break;
	}
	default:
		break;
	}
}

// Colourless Protection
