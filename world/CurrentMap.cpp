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

#include "CurrentMap.h"
#include "Map.h"
#include "Item.h"
#include "GlobEgg.h"
#include "Actor.h"
#include "World.h"

using std::list; // too messy otherwise
typedef list<Item*> item_list;

CurrentMap::CurrentMap()
	: current_map(0)
{
	items = new list<Item*>*[128]; //! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		items[i] = new list<Item*>[128];
	}
}


CurrentMap::~CurrentMap()
{
	clear();

	//! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		delete[] items[i];
	}
	delete[] items;
}

void CurrentMap::clear()
{
	// We need to be careful about who exactly deletes the items in a map
	// (CurrentMap or Map)
	// It should probably be CurrentMap, which means a Map has to be
	// emptied when it's loaded into CurrentMap

	//! get rid of constants
	for (unsigned int i = 0; i < 128; i++) {
		for (unsigned int j = 0; j < 128; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
				delete *iter;
			items[i][j].clear();
		}
	}

	current_map = 0;
}

uint32 CurrentMap::getNum() const
{
	if (current_map == 0)
		return 0;

	return current_map->mapnum;
}

void CurrentMap::writeback()
{
	if (!current_map)
		return;

	//! constants

	for (unsigned int i = 0; i < 128; i++) {
		for (unsigned int j = 0; j < 128; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
			{
				Item* item = *iter;

				// delete items inside globs
				if (item->getExtFlags() | Item::EXT_INGLOB) {
					item->clearObjId();
					delete item;
					continue;
				}

				// unexpand all globeggs (note that this doesn't do much)
				GlobEgg* globegg = p_dynamic_cast<GlobEgg*>(item);
				if (globegg) {
					globegg->unexpand();
				}

				// this item isn't from the Map. (like NPCs)
				if (item->getExtFlags() | Item::EXT_NOTINMAP)
					continue;

				if (item->getExtFlags() | Item::EXT_FIXED) {
					// item came from fixed
					current_map->fixeditems.push_back(item);
				} else {
					current_map->dynamicitems.push_back(item);
				}
			}
			items[i][j].clear();
		}
	}
}

void CurrentMap::loadItems(list<Item*> itemlist)
{
	item_list::iterator iter;
	for (iter = itemlist.begin(); iter != itemlist.end(); ++iter)
	{
		Item* item = *iter;

		// add item to internal object list
		addItem(item);

		item->assignObjId();


		GlobEgg* globegg = p_dynamic_cast<GlobEgg*>(item);
		if (globegg) {
			globegg->expand();
		}
	}
}

void CurrentMap::loadMap(Map* map)
{
	current_map = map;

	loadItems(map->fixeditems);
	loadItems(map->dynamicitems);

	// we take control of the items in map, so clear the pointers
	map->fixeditems.clear();
	map->dynamicitems.clear();


	// load relevant NPCs to the item lists
	// !constant
	for (uint16 i = 0; i < 256; ++i) {
		Actor* actor = World::get_instance()->getNPC(i);
		if (actor && actor->getMapNum() == getNum()) {
			addItem(actor);
		}
	}
}

void CurrentMap::addItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	//! constants
	if (ix < 0 || iy < 0 || ix >= 512*128 || iy >= 512*128) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / 512;
	sint32 cy = iy / 512;

	items[cx][cy].push_back(item);
}
