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

Map::Map()
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
	loadFixedFormatObjects(dynamicitems, ds);
}


void Map::loadFixedFormatObjects(std::list<Item*>& itemlist, IDataSource* ds)
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
			pout << "---- Ending container ----" << std::endl;
		}

		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;

		// Question: how do we handle Globs?

		Item *item = ItemFactory::createItem(shape,frame,flags,quality,
											 npcnum,mapnum);
		if (!item) {
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
			pout << "---- Starting container ----" << std::endl;
		}
	}
}


// read items from start (inclusive) to end (exclusive)
// NB: Untested
void Map::loadItemCachObjects(std::list<Item*>& itemlist,
							  IDataSource* itemcach,
							  IDataSource* npcdata,
							  unsigned int start, unsigned int end)
{
	if (end <= start) return;

	std::stack<Container*> cont;
	int contdepth = 0;

	for (uint32 i = start; i < end; ++i)
	{
		itemcach->seek(0x00000+i*2);
		sint32 x = static_cast<sint32>(itemcach->readXS(2));
		itemcach->seek(0x04800+i*2);
		sint32 y = static_cast<sint32>(itemcach->readXS(2));
		itemcach->seek(0x09000+i*1);
		sint32 z = static_cast<sint32>(itemcach->readXS(1));

		itemcach->seek(0x0B400+i*2);
		uint32 shape = itemcach->read2();
		itemcach->seek(0x0FC00+i*1);
		uint32 frame = itemcach->read1();
		itemcach->seek(0x12000+i*2);
		uint32 flags = itemcach->read2();
		itemcach->seek(0x16800+i*2);
		uint16 quality = itemcach->read2();
		itemcach->seek(0x1B000+i*1);
		uint32 npcnum = itemcach->read1();
		itemcach->seek(0x1D400+i*1);
		uint32 mapnum = itemcach->read1();
		itemcach->seek(0x1F800+i*2);
		uint32 next = itemcach->read2();

		if (npcdata != 0 && i < 256) {
			npcdata->seek(7 + i * 0x31);
			frame += npcdata->read1() << 8;
		}

		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;

		// Question: how do we handle Globs?

		Item *item = ItemFactory::createItem(shape,frame,flags,quality,
											 npcnum,mapnum);
		if (!item) {
			pout << "Couldn't create item" << std::endl;
			continue;
		}
		item->setLocation(x,y,z);

		// find container this item belongs to, if any.
		// the x coordinate stores the container-depth of this item,
		// so pop items from the container stack until we reach x,
		// or, if x is too large, the item is added to the top-level list
		while (contdepth != x && contdepth > 0) {
			cont.pop();
			contdepth--;
		}

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
		}
	}
	
}
