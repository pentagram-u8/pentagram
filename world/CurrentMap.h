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

#ifndef CURRENTMAP_H
#define CURRENTMAP_H

#include <list>

class Map;
class Item;
class UCList;
class TeleportEgg;
class EggHatcherProcess;

class CurrentMap
{
public:
	CurrentMap();
	~CurrentMap();

	void clear();
	void writeback();
	void loadMap(Map* map);

	uint32 getNum() const;

	void addItem(Item* item);
	void removeItemFromList(Item* item, sint32 oldx, sint32 oldy);

	void areaSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* item, uint16 range, bool recurse,
					sint32 x=0, sint32 y=0);

	TeleportEgg* findDestination(uint16 id);

	// Not allowed to modify the list. Remember to use const_iterator
	const std::list<Item*>* getItemList (sint32 gx, sint32 gy)
	{
		// CONSTANTS!
		if (gx < 0 || gy < 0 || gx >= 128 || gy >= 128) return 0;
		return &items[gx][gy];
	}

private:
	void loadItems(std::list<Item*> itemlist);

	Map* current_map;

	// item lists. Lots of them :-)
	// items[x][y]
	std::list<Item*>** items;

	EggHatcherProcess* egghatcher;
};

#endif
