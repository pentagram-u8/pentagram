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

//#define DUMP_ITEMS

World* World::world = 0;

World::World()
	: current_map(0), fixed(0), fixedds(0)
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
	for (unsigned int i = 0; i < maps.size(); ++i) {
		delete maps[i];
	}
	maps.clear();

	for (unsigned int i = 0; i < npcs.size(); ++i) {
		delete npcs[i];
	}
	npcs.clear();

	if (current_map)
		delete current_map;

	if (fixed)
		delete fixed;
	if (fixedds)
		delete fixedds;
}

void World::initMaps()
{
	// Q: How do we determine which Maps to create? Only create those
	// with non-zero size in fixed.dat?

	maps.resize(256);
	for (unsigned int i = 0; i < 256; ++i) {
		maps[i] = new Map(i);
	}

	current_map = new CurrentMap();
}

void World::initNPCs()
{
	// Q: How do we determine which NPCs to create?

	// automatically initialized to 0
	npcs.resize(256);
}

bool World::switchMap(uint32 newmap)
{
	assert(current_map);

	if (current_map->getNum() == newmap)
		return true;

	if (newmap >= maps.size() || maps[newmap] == 0)
		return false; // no such map

	// Map switching procedure:
	// write back CurrentMap to the old map
	// clear all objIDs
	// swap out fixed items in old map?
	// make sure fixed items in the new map are loaded
	// assign objIDs to fixed items
	// assign objIDs to nonfixed items
	// load new map into CurrentMap

	// NB: not only World has to perform actions on a map switch
	// other things that should/could also happen:
	// - usecode processes have to be terminated (forcefully?)
	// - autosave?
	// - ...?

	uint32 oldmap = current_map->getNum();

	if (oldmap != 0) {
		assert(oldmap < maps.size() && maps[oldmap] != 0);

		current_map->writeback();
		maps[oldmap]->unloadFixed();

		// TODO: clear objIDs
	}

	pout << "Loading Fixed items in map " << newmap << std::endl;
	pout << "-----------------------------------------" << std::endl;
	IDataSource *items = fixed->get_datasource(newmap);
	maps[newmap]->loadFixed(items);
	delete items;

	// TODO: assign objIDs

	current_map->loadMap(maps[newmap]);

	return true;
}

void World::loadNonFixed(IDataSource* ds)
{
	Flex* f = new Flex(ds);

	for (unsigned int i = 0; i < f->get_count(); ++i) {

		// items in this map?
		if (f->get_size(i) > 0) {
			pout << "Loading NonFixed items in map " << i << std::endl;
			pout << "-----------------------------------------" << std::endl;

			assert(maps.size() > i);
			assert(maps[i] != 0);

			IDataSource *items = f->get_datasource(i);

			maps[i]->loadNonFixed(items);

			delete items;

		}
	}

	delete f;
}


void World::loadFixed(IDataSource* ds)
{
	fixed = new Flex(ds);
	fixedds = ds;
}

void World::loadItemCachNPCData(IDataSource* itemcach, IDataSource* npcdata)
{
	Flex* itemcachflex = new Flex(itemcach);
	Flex* npcdataflex = new Flex(npcdata);

	IDataSource* itemds = itemcachflex->get_datasource(0);
	IDataSource* npcds = npcdataflex->get_datasource(0);

	pout << "Loading NPCs" << std::endl;
	pout << "----------------" << std::endl;

	for (uint32 i = 1; i < 256; ++i) // Get rid of constants?
	{
		itemds->seek(0x00000+i*2);
		sint32 x = static_cast<sint32>(itemds->readXS(2));
		itemds->seek(0x04800+i*2);
		sint32 y = static_cast<sint32>(itemds->readXS(2));
		itemds->seek(0x09000+i*1);
		sint32 z = static_cast<sint32>(itemds->readXS(1));

		itemds->seek(0x0B400+i*2);
		uint32 shape = itemds->read2();
		itemds->seek(0x0FC00+i*1);
		uint32 frame = itemds->read1();
		itemds->seek(0x12000+i*2);
		uint32 flags = itemds->read2();
		itemds->seek(0x16800+i*2);
		uint16 quality = itemds->read2();
		itemds->seek(0x1B000+i*1);
		uint32 npcnum = itemds->read1();
		itemds->seek(0x1D400+i*1);
		uint32 mapnum = itemds->read1();
		itemds->seek(0x1F800+i*2);
		uint32 next = itemds->read2();

		// half the frame number is stored in npcdata.dat
		npcds->seek(7 + i * 0x31);
		frame += npcds->read1() << 8;

		// TODO: (decode and) read rest of npcdata.dat...
		// TODO: locate inventory

		if (shape == 0) {
			// U8's itemcach has a lot of garbage in it.
			// Ignore it.
			x = 0; y = 0; z = 0;
			frame = 0;
			flags = 0; quality = 0;
			npcnum = 0; mapnum = 0;
			next = 0;
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Actor *actor = ItemFactory::createActor(shape,frame,flags,quality,
												npcnum,mapnum);
		if (!actor) {
#ifdef DUMP_ITEMS
			// this 'error' message is supposed to occur rather a lot
			pout << "Couldn't create actor" << std::endl;
#endif
			continue;
		}
		actor->setLocation(x,y,z);

		npcs[i] = actor;
	}

	delete itemcachflex;
	delete npcdataflex;
	delete itemds;
	delete npcds;
}
