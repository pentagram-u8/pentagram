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

#ifndef WORLD_H
#define WORLD_H

// the main world class.

// Current ideas on how to store the game world: (-wjp)

// World contains Map objects. All Map objects are initialized when
//  starting/loading a game.

// Each Map permanently contains the nonfixed objects in that map,
//  ( These objects don't have to be assigned object IDs here, I think )

// When a Map is loaded, it loads the fixed objects from disk.
// All objects in the active map will also have to be assigned object IDs.
//  NB: This has to happen in such a way that when a game is loaded the IDs can
//  be reproduced exactly. (Since running usecode scripts can contain objIDs)
// The first N objIDs will probably be reserved from NPCs (and inventories?)
// After that, for example, the fixed objects could be loaded (in disk-order),
// followed by assigning IDs to the (already loaded) dynamic items.
//  (in basic depth-first order, I would guess)

// NPCs will also have to be stored somewhere. We could keep them in Map #0
// like the original.
// Q: Is the avatar's inventory stored in npcdata?
// Q: Is the number of objIDs reserved for npcdata fixed?

// World also has to have the necessary save/load functions. (Which will
//  mostly consist of calls to the save/load functions of the Map objects.)

// Fixed objects could be kept cached in for better performance, or
// swapped out for memory.

// A clear/reset function would also be useful. (All singletons that store
//  game data need this, actually.)

#include <vector>
#include <stack>

class Map;
class CurrentMap;
class IDataSource;
class Actor;
class Flex;
class Item;
class Actor;


class World
{
public:
	World();
	~World();

	static World* get_instance() { return world; }

	void clear();

	void initMaps();
	void loadNonFixed(IDataSource* ds); // delete ds afterwards
	void loadItemCachNPCData(IDataSource* itemcach, IDataSource* npcdata);

	CurrentMap* getCurrentMap() const { return currentmap; }

	bool switchMap(uint32 newmap);

	Item* getItem(uint16 itemid) const;
	Actor* getNPC(uint16 npcid) const;

	void etherealPush(uint16 objid) { ethereal.push(objid); }
	bool etherealEmpty() { return ethereal.empty(); }
	uint16 etherealPop()
		{ uint16 id = ethereal.top(); ethereal.pop(); return id; }
	uint16 etherealPeek() { return ethereal.top(); }

	void worldStats();

private:
	static World *world;

	std::vector<Map*> maps;
	CurrentMap* currentmap;

	std::stack<uint16> ethereal;
};

#endif
