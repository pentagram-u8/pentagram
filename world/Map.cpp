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

#include <stack>

#include "Map.h"
#include "IDataSource.h"
#include "ItemFactory.h"
#include "Item.h"
#include "Container.h"

#include "World.h"

//#define DUMP_ITEMS

Map::Map(uint32 mapnum_)
	: mapnum(mapnum_)
{

}


Map::~Map()
{
	clear();
}

void Map::clear()
{
	std::list<Item*>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();

	for (iter = dynamicitems.begin(); iter != dynamicitems.end(); ++iter) {
		delete *iter;
	}
	dynamicitems.clear();
}

void Map::loadNonFixed(IDataSource* ds)
{
	loadFixedFormatObjects(dynamicitems, ds, 0);
}

void Map::loadFixed(IDataSource* ds)
{
	loadFixedFormatObjects(fixeditems, ds, Item::EXT_FIXED);
}

void Map::unloadFixed()
{
	std::list<Item*>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();
}

void Map::loadFixedFormatObjects(std::list<Item*>& itemlist, IDataSource* ds,
								 uint32 extendedflags)
{
	uint32 size = ds->getSize();
	if (size == 0) return;

	uint32 itemcount = size / 16;

	std::stack<Container*> cont;
	int contdepth = 0;

	for (uint32 i = 0; i < itemcount; ++i)
	{
		sint32 x = static_cast<sint32>(ds->readXS(2));
		sint32 y = static_cast<sint32>(ds->readXS(2));
		sint32 z = static_cast<sint32>(ds->readXS(1));

		uint32 shape = ds->read2();
		uint32 frame = ds->read1();
		uint32 flags = ds->read2();
		uint16 quality = ds->read2();
		uint32 npcnum = ds->read1();
		uint32 mapnum = ds->read1();
		uint16 next = ds->read2(); // do we need next for anything?

		// find container this item belongs to, if any.
		// the x coordinate stores the container-depth of this item,
		// so pop items from the container stack until we reach x,
		// or, if x is too large, the item is added to the top-level list
		while (contdepth != x && contdepth > 0) {
			cont.pop();
			contdepth--;
#ifdef DUMP_ITEMS
			pout << "---- Ending container ----" << std::endl;
#endif
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Item *item = ItemFactory::createItem(shape,frame,quality,flags,
											 npcnum,mapnum,extendedflags);
		if (!item) {
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
			pout << "Couldn't create item" << std::endl;
			continue;
		}
		item->setLocation(x,y,z);

		if (contdepth > 0) {
			cont.top()->AddItem(item);	
		} else {
			itemlist.push_back(item);
		}

		Container *c = p_dynamic_cast<Container*>(item);
		if (c) {
			// container, so prepare to read contents
			contdepth++;
			cont.push(c);
#ifdef DUMP_ITEMS
			pout << "---- Starting container ----" << std::endl;
#endif
		}

#if 0
		//! big hack :-)
		// NB: this causes crashes on map-switches, somehow
		if (item->getShape() == 180) {
			static bool alreadydone = false;
//			if (!alreadydone) {
				item->assignObjId();
				item->callUsecodeEvent(0);
//			}
			alreadydone = true;
		}
#endif

	}
}
