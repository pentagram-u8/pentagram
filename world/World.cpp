/*
Copyright (C) 2003 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "World.h"
#include "Map.h"
#include "CurrentMap.h"
#include "IDataSource.h"
#include "Flex.h"
#include "ItemFactory.h"
#include "Actor.h"
#include "idMan.h"
#include "GameData.h"
#include "Kernel.h"
#include "GUIApp.h"
#include "CameraProcess.h" // for resetting the camera
#include "Gump.h" // For MapChanged notification

//#define DUMP_ITEMS

World* World::world = 0;

World::World()
	: currentmap(0)
{
	assert(world == 0);
	world = this;

	objects.resize(65536);

	//!CONSTANTS
	objIDs = new idMan(256,65534);	// Want range of 256 to 65534
	actorIDs = new idMan(1,255);
}


World::~World()
{
	clear();
	delete objIDs;
	delete actorIDs;

	world = 0;
}


void World::clear()
{
	unsigned int i;

	for (i = 0; i < maps.size(); ++i) {
		delete maps[i];
	}
	maps.clear();

	while (!ethereal.empty())
		ethereal.pop();

	if (currentmap)
		delete currentmap;

	//! Need to check the object delete policy
	// If everything works out, there shouldn't be any objects left
	// (maybe NPCs? Ethereals?)
	for (i = 0; i < objects.size(); ++i) {
		objects[i] = 0;
	}

	// Clear all the objIDs
	objIDs->clearAll();
	actorIDs->clearAll();
}

void World::initMaps()
{
	// Q: How do we determine which Maps to create? Only create those
	// with non-zero size in fixed.dat?

	maps.resize(256);
	for (unsigned int i = 0; i < 256; ++i) {
		maps[i] = new Map(i);
	}

	currentmap = new CurrentMap();
}

bool World::switchMap(uint32 newmap)
{
	assert(currentmap);

	if (currentmap->getNum() == newmap)
		return true;

	if (newmap >= maps.size() || maps[newmap] == 0)
		return false; // no such map

	// Map switching procedure:
	// write back CurrentMap to the old map, which
	//   un-expands GlobEggs (delete EXT_INGLOB items, reset contents fields)
	// clear all objIDs
	// swap out fixed items in old map?
	// make sure fixed items in the new map are loaded
	// load new map into CurrentMap, which also
	//   expands GlobEggs
	//   assigns objIDs to fixed items
	//   assigns objIDs to nonfixed items

	// NB: not only World has to perform actions on a map switch
	// other things that should/could also happen:
	// - processes have to be terminated (all processes?)
	// - autosave?
	// - camera reset?
	// - ...?

	// TODO:
	// - check ethereal items
	// - temporary items/actors?

	// kill camera
	CameraProcess::ResetCameraProcess();

	// kill ALL processes:
	Kernel::get_instance()->killProcesses(0,6); // !constant

	// Notify all the gumps of the mapchange
	GUIApp *gui = getGUIInstance();
	if (gui) 
	{
		Gump *desktop = gui->getDesktopGump();
		if (desktop) desktop->MapChanged();
	}

#if 0
	// get rid of any remaining ethereal items
	while (!etherealEmpty()) {
		uint16 eth = etherealPop();
		Object* o = getObject(eth);
		if (o) {
			o->clearObjId();
			delete o;
		}
	}
#endif

	uint32 oldmap = currentmap->getNum();
	if (oldmap != 0) {
		perr << "Unloading map " << oldmap << std::endl;

		assert(oldmap < maps.size() && maps[oldmap] != 0);

		currentmap->writeback();

		perr << "Unloading Fixed items from map " << oldmap << std::endl;

		maps[oldmap]->unloadFixed();

		//! constant
		for (unsigned int i = 256; i < objects.size(); ++i) {
//			assert(objects[i] == 0);
			if (objects[i] != 0)
				objects[i]->clearObjId();
		}
	}

	// Clear all the objIDs
	objIDs->clearAll();

	pout << "Loading Fixed items in map " << newmap << std::endl;
	IDataSource *items = GameData::get_instance()->getFixed()
		->get_datasource(newmap);
	maps[newmap]->loadFixed(items);
	delete items;

	currentmap->loadMap(maps[newmap]);

	// reset camera
	CameraProcess::SetCameraProcess(new CameraProcess(1));

	return true;
}

void World::loadNonFixed(IDataSource* ds)
{
	Flex* f = new Flex(ds);

	for (unsigned int i = 0; i < f->get_count(); ++i) {

		// items in this map?
		if (f->get_size(i) > 0) {
			pout << "Loading NonFixed items in map " << i << std::endl;

			assert(maps.size() > i);
			assert(maps[i] != 0);

			IDataSource *items = f->get_datasource(i);

			maps[i]->loadNonFixed(items);

			delete items;

		}
	}

	delete f;
}

void World::loadItemCachNPCData(IDataSource* itemcach, IDataSource* npcdata)
{
	Flex* itemcachflex = new Flex(itemcach);
	Flex* npcdataflex = new Flex(npcdata);

	IDataSource* itemds = itemcachflex->get_datasource(0);
	IDataSource* npcds = npcdataflex->get_datasource(0);

	pout << "Loading NPCs" << std::endl;

	for (uint32 i = 1; i < 256; ++i) // Get rid of constants?
	{
		// These are ALL unsigned on disk
		itemds->seek(0x00000+i*2);
		sint32 x = static_cast<sint32>(itemds->readX(2));
		itemds->seek(0x04800+i*2);
		sint32 y = static_cast<sint32>(itemds->readX(2));
		itemds->seek(0x09000+i*1);
		sint32 z = static_cast<sint32>(itemds->readX(1));

		itemds->seek(0x0B400+i*2);
		uint32 shape = itemds->read2();
		itemds->seek(0x0FC00+i*1);
		uint32 frame = itemds->read1();
		itemds->seek(0x12000+i*2);
		uint16 flags = itemds->read2();
		itemds->seek(0x16800+i*2);
		uint16 quality = itemds->read2();
		itemds->seek(0x1B000+i*1);
		uint16 npcnum = static_cast<uint8>(itemds->read1());
		itemds->seek(0x1D400+i*1);
		uint16 mapnum = static_cast<uint8>(itemds->read1());
		itemds->seek(0x1F800+i*2);
		uint16 next = itemds->read2();

		// half the frame number is stored in npcdata.dat
		npcds->seek(7 + i * 0x31);
		frame += npcds->read1() << 8;

		// TODO: locate inventory

		if (shape == 0) {
			// U8's itemcach has a lot of garbage in it.
			// Ignore it.
			continue;
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Actor *actor = ItemFactory::createActor(shape,frame,quality,flags,
												npcnum,mapnum,
												Item::EXT_NOTINMAP);
		if (!actor) {
#ifdef DUMP_ITEMS
			pout << "Couldn't create actor" << std::endl;
#endif
			continue;
		}
		actor->setLocation(x,y,z);

		// read npcdata:
		npcds->seek(i * 0x31);
		actor->setStr(npcds->read1());
		actor->setDex(npcds->read1());
		actor->setInt(npcds->read1());
		actor->setHP(npcds->read1());
		actor->setDir(npcds->read1()); //! is this correct?
		actor->setLastAnim(npcds->read1());
		// TODO: (decode and) read rest of npcdata.dat...

		objects[i] = actor;
		actorIDs->reserveID(i);
	}

	delete itemcachflex;
	delete npcdataflex;
	delete itemds;
	delete npcds;
}


uint16 World::assignObjId(Object* obj)
{
	uint16 new_objid = objIDs->getNewID();
	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = obj;
	}
	return new_objid;
}

uint16 World::assignActorObjId(Actor* actor)
{
	uint16 new_objid = actorIDs->getNewID();
	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = actor;
	}
	return new_objid;
}

void World::clearObjId(uint16 objid)
{
	// need to make this assert check only permanent NPCs
//	assert(objid >= 256); // !constant
	if (objid >= 256) // !constant
		objIDs->clearID(objid);
	else
		actorIDs->clearID(objid);

	objects[objid] = 0;
}

void World::worldStats()
{
	unsigned int i, npccount = 0, objcount = 0, mapcount = 0;

	//!constants
	for (i = 1; i < 256; i++) {
		if (objects[i] != 0)
			npccount++;
	}
	for (i = 256; i < objects.size(); i++) {
		if (objects[i] != 0)
			objcount++;
	}

	for (i = 0; i < maps.size(); i++) {
		if (maps[i] != 0)
			mapcount++;
	}

	pout << "World memory stats:" << std::endl;
	pout << "NPCs    : " << npccount << "/255" << std::endl;
	pout << "Objects : " << objcount << "/65279" << std::endl;
	pout << "Maps    : " << mapcount << "/256" << std::endl;
}

Object* World::getObject(uint16 objid) const
{
	return objects[objid];
}

Item* World::getItem(uint16 itemid) const
{
	return p_dynamic_cast<Item*>(objects[itemid]);
}

Actor* World::getNPC(uint16 npcid) const
{
	return p_dynamic_cast<Actor*>(objects[npcid]);
}

