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

#include "Map.h"
#include "IDataSource.h"
#include "ItemFactory.h"

Map::Map()
{

}


Map::~Map()
{

}

void Map::loadObjects(IDataSource* ds)
{
	uint32 size = ds->getSize();
	if (size == 0) return;

	uint32 itemcount = size / 16;

	for (uint32 i = 0; i < itemcount; ++i)
	{
		sint32 x = static_cast<sint32>(ds->read2());
		sint32 y = static_cast<sint32>(ds->read2());
		sint32 z = static_cast<sint32>(ds->read1());

		uint32 shape = ds->read2();
		uint32 frame = ds->read1();
		uint32 flags = ds->read2();
		uint16 quality = ds->read2();
		uint32 npcnum = ds->read1();
		uint32 mapnum = ds->read1();
		uint16 next = ds->read2();

		// Question: how do we handle NPCs, Globs?

		Item *item = ItemFactory::createItem(shape,frame,flags,quality,mapnum);

		items.push_back(item);
	}
}
