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
#include "ODataSource.h"
#include "Flex.h"
#include "ItemFactory.h"
#include "Actor.h"
#include "MainActor.h"
#include "idMan.h"
#include "GameData.h"
#include "Kernel.h"
#include "ObjectManager.h"
#include "GUIApp.h"
#include "CameraProcess.h" // for resetting the camera
#include "Gump.h" // For MapChanged notification
#include "Animation.h"

//#define DUMP_ITEMS

World* World::world = 0;

World::World()
	: currentmap(0)
{
	assert(world == 0);
	world = this;
}


World::~World()
{
	clear();

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
		ethereal.pop_front();

	if (currentmap)
		delete currentmap;
	currentmap = 0;
}

void World::reset()
{
	clear();

	initMaps();
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

	// get rid of camera
	// notify all gumps of a map change
	// delete any ethereal objects
	// write back CurrentMap to the old map, which
	//   deletes all disposable items
	//   deletes the EggHatcher
	//   resets all eggs
	// swap out fixed items in old map
	// kill all processes (except those of type 1)
	// load fixed items in new map
	// load new map into CurrentMap, which also
	//   assigns objIDs to fixed items
	//   assigns objIDs to nonfixed items
	//   creates an EggHatcher and notifies it of all eggs
	//   sets up all NPCs in the new map
	// reset camera


	// kill camera
	CameraProcess::ResetCameraProcess();

	// Notify all the gumps of the mapchange
	GUIApp *gui = GUIApp::get_instance();
	if (gui) 
	{
		Gump *desktop = gui->getDesktopGump();
		if (desktop) desktop->MapChanged();
	}

	// get rid of any remaining ethereal items
	while (!ethereal.empty()) {
		uint16 eth = ethereal.front();
		ethereal.pop_front();
		Item* i = getItem(eth);
		if(i) i->destroy();
	}

	uint32 oldmap = currentmap->getNum();
	if (oldmap != 0) {
		perr << "Unloading map " << oldmap << std::endl;

		assert(oldmap < maps.size() && maps[oldmap] != 0);

		currentmap->writeback();

		perr << "Unloading Fixed items from map " << oldmap << std::endl;

		maps[oldmap]->unloadFixed();
	}

	// Kill any processes that need killing (those with type != 1)
	Kernel::get_instance()->killProcessesNotOfType(0, 1);

	pout << "Loading Fixed items in map " << newmap << std::endl;
	IDataSource *items = GameData::get_instance()->getFixed()
		->get_datasource(newmap);
	maps[newmap]->loadFixed(items);
	delete items;

	currentmap->loadMap(maps[newmap]);

	// reset camera
	CameraProcess::SetCameraProcess(new CameraProcess(1));
	CameraProcess::SetEarthquake(0);

	return true;
}

void World::loadNonFixed(IDataSource* ds)
{
	Flex* f = new Flex(ds);

	pout << "Loading NonFixed items" << std::endl;
			
	for (unsigned int i = 0; i < f->get_count(); ++i) {

		// items in this map?
		if (f->get_size(i) > 0) {
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
		uint16 next;
		next = itemds->read2();

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

		Actor *actor = ItemFactory::createActor(shape,frame,quality,
												flags|Item::FLG_IN_NPC_LIST,
												npcnum,mapnum, 0);
		if (!actor) {
#ifdef DUMP_ITEMS
			pout << "Couldn't create actor" << std::endl;
#endif
			continue;
		}
		actor->setLocation(x,y,z);

		// read npcdata:
		npcds->seek(i * 0x31);
		actor->setStr(npcds->read1()); // 0x00: strength
		actor->setDex(npcds->read1()); // 0x01: dexterity
		actor->setInt(npcds->read1()); // 0x02: intelligence
		actor->setHP(npcds->read1());  // 0x03: hitpoints
		actor->setDir(npcds->read1()); // 0x04: direction
		actor->setLastAnim((Animation::Sequence) npcds->read2()); // 0x05: last anim
		npcds->skip(1); // 0x07: unknown 
		npcds->skip(1); // 0x08: current anim frame
		npcds->skip(1); // 0x09: start Z of current fall
		npcds->skip(1); // 0x0A: unknown
		uint8 align = npcds->read1(); // 0x0B: alignments
		actor->setAlignment(align & 0x0F);
		actor->setEnemyAlignment(align & 0xF0);
		npcds->skip(15); // 0x0C-0x1A: unknown
		actor->clearActorFlag(0xFF);
		actor->setActorFlag(npcds->read1()); // 0x1B: flags
		npcds->skip(1); // 0x1C: unknown
		npcds->skip(16); // 0x1D-0x2C: equipment
		sint16 mana = static_cast<sint16>(npcds->read2()); // 0x2D: mana
		actor->setMana(mana);
		actor->clearActorFlag(0xFFFF00);
		uint32 flags2F = npcds->read1(); // 0x2F: flags
		actor->setActorFlag(flags2F << 8);
		uint32 flags30 = npcds->read1(); // 0x30: flags
		actor->setActorFlag(flags30 << 16);

		// TODO: decode and use rest of npcdata.dat

		ObjectManager::get_instance()->assignActorObjId(actor, i);
	}

	delete itemcachflex;
	delete npcdataflex;
	delete itemds;
	delete npcds;
}


void World::worldStats()
{
	unsigned int i, mapcount = 0;

	for (i = 0; i < maps.size(); i++) {
		if (maps[i] != 0 && !maps[i]->isEmpty())
			mapcount++;
	}

	pout << "World memory stats:" << std::endl;
	pout << "Maps       : " << mapcount << "/256" << std::endl;
	Actor* av = getNPC(1);
	pout << "Avatar pos.: ";
	if (av) {
		pout << "map " << av->getMapNum() << ", (";
		sint32 x,y,z;
		av->getLocation(x,y,z);
		pout << x << "," << y << "," << z << ")" << std::endl;
	} else {
		pout << "missing (null)" << std::endl;
	}
}

Item* World::getItem(uint16 itemid) const
{
	return p_dynamic_cast<Item*>(ObjectManager::get_instance()->
								 getObject(itemid));
}

Actor* World::getNPC(uint16 npcid) const
{
	return p_dynamic_cast<Actor*>(ObjectManager::get_instance()->
								  getObject(npcid));
}

MainActor* World::getMainActor() const
{
	return p_dynamic_cast<MainActor*>(ObjectManager::get_instance()->
									  getObject(1));
}




void World::save(ODataSource* ods)
{
	ods->write2(1); // version

	ods->write4(currentmap->getNum());

	ods->write2(currentmap->egghatcher);

	uint16 es = ethereal.size();
	ods->write4(es);

	// empty stack and refill it again
	uint16* e = new uint16[es];
	std::list<ObjId>::iterator it = ethereal.begin();
	unsigned int i;
	for (i = 0; i < es; ++i) {
		e[es-i] = *it;
		++it;
	}

	for (i = 0; i < es; ++i) {
		ods->write2(e[i]);
	}
	delete[] e;
}

// load items
bool World::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	uint16 curmapnum = ids->read4();
	currentmap->setMap(maps[curmapnum]);

	currentmap->egghatcher = ids->read2();

	uint32 etherealcount = ids->read4();
	for (unsigned int i = 0; i < etherealcount; ++i) {
		ethereal.push_front(ids->read2());
	}

	return true;
}

void World::saveMaps(ODataSource* ods)
{
	ods->write2(1); //version
	ods->write4(maps.size());
	for (unsigned int i = 0; i < maps.size(); ++i) {
		maps[i]->save(ods);
	}
}


bool World::loadMaps(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;	

	uint32 mapcount = ids->read4();

	// Map objects have already been created by reset()
	for (unsigned int i = 0; i < mapcount; ++i) {
		bool res = maps[i]->load(ids);
		if (!res) return false;
	}

	return true;
}
