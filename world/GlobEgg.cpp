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

#include "GlobEgg.h"
#include "Glob.h"
#include "GameData.h"
#include "ItemFactory.h"
#include "World.h"
#include "CurrentMap.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(GlobEgg,Item);

GlobEgg::GlobEgg()
	: contents(0)
{

}

GlobEgg::~GlobEgg()
{

}


// expand contents of glob.
// needs to assign an objID, and also needs to add them to the CurrentMap
void GlobEgg::expand()
{
	Glob* glob = GameData::get_instance()->getGlob(quality);
	if (!glob) return;

	Item* previous = 0;

	std::vector<GlobItem>::iterator iter;
	for (iter = glob->contents.begin(); iter != glob->contents.end(); ++iter)
	{
		GlobItem& globitem = *iter;
		Item* item = ItemFactory::createItem(globitem.shape, globitem.frame, 
											 0, 0, 0, 0, Item::EXT_INGLOB);
		// Q: do we need to set any flags?



		// calculate object's world position
		sint32 itemx = (x & 0x1FF) + 2 * globitem.x;
		sint32 itemy = (y & 0x1FF) + 2 * globitem.y;
		sint32 itemz = z + globitem.z;

		item->setLocation(itemx, itemy, itemz);

		item->assignObjId();

		// add item to map
		World::get_instance()->getCurrentMap()->addItem(item);


		// link item to glob's linked list
		if (previous)
			previous->setGlobNext(item);
		else
			contents = item;
		previous = item;
	}
}

// delete expanded contents.
// needs to clear objIDs, but doesn't need to remove pointer from CurrentMap
// (since we're probably iterating over object lists in CurrentMap when called)
void GlobEgg::unexpand()
{
	contents = 0;

	// New idea: just leave all objects be.
	// They'll be deleted in CurrentMap::writeBack()

#if 0
	Item* item = contents;

	while (item) {
		Item* next = item->getGlobNext();

		// Q: Need to think this through sometime. We can't really delete the
		// item here (since it's still in currentmap's object list), so maybe
		// we need to mark this item 'to be deleted' somehow

		//! currently a memory leak here...

		item->clearObjId();

		item = next;
	}
	contents = 0;
#endif
}
