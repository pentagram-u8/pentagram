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
#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GlobEgg,Item);

GlobEgg::GlobEgg()
{

}

GlobEgg::~GlobEgg()
{

}


// Called when an item has entered the fast area
void GlobEgg::enterFastArea()
{
	// Expand it
	if (!(flags & FLG_FASTAREA)) 
	{
		Glob* glob = GameData::get_instance()->getGlob(quality);
		if (!glob) return;

		std::vector<GlobItem>::iterator iter;
		for (iter = glob->contents.begin(); iter != glob->contents.end(); ++iter)
		{
			GlobItem& globitem = *iter;
			Item* item = ItemFactory::createItem(globitem.shape, globitem.frame, 
								0, FLG_DISPOSABLE|FLG_FAST_ONLY, 0, 0, 0);

			item->assignObjId();

			// calculate object's world position
			sint32 itemx = (x & ~0x1FF) + 2 * globitem.x;
			sint32 itemy = (y & ~0x1FF) + 2 * globitem.y;
			sint32 itemz = z + globitem.z;

			item->move(itemx, itemy, itemz);
		}
	}

	Item::enterFastArea();
}
