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

#ifndef MAP_H
#define MAP_H

#include <list>

class Item;
class IDataSource;
class ODataSource;

class Map
{
	friend class CurrentMap;
public:
	explicit Map(uint32 mapnum);
	~Map();

	void clear();

	void loadNonFixed(IDataSource* ds);
	void loadFixed(IDataSource* ds);
	void unloadFixed();

	bool isEmpty()
		{ return fixeditems.size() == 0 && dynamicitems.size() == 0; }

	void save(ODataSource* ods);
	bool load(IDataSource* ids, uint32 version);

private:

	// load items from something formatted like 'fixed.dat'
	void loadFixedFormatObjects(std::list<Item*>& itemlist, IDataSource* ds,
								uint32 extendedflags);

	// Q: How should we store the items in a map.
	// It might make things more efficient if we order them by 'chunk'
	// (512x512). This would mean we need about 128x128 item lists.

	// It would probably be overkill to permanently maintain all these lists
	// for all maps, so we could only set them up for the current map.
	// (which makes me wonder if there should be a separate class for the
	// active map?)

	// (Note that we probably won't even have all items permanently stored,
	// since fixed items will be cached out most of the time)


	std::list<Item*> fixeditems;
	std::list<Item*> dynamicitems;

	uint32 mapnum;
};


#endif
