/*
Copyright (C) 2003-2006 The Pentagram team

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
#include "MapGlob.h"
#include "GameData.h"
#include "ItemFactory.h"
#include "CurrentMap.h"
#include "CoreApp.h"
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
	uint32 coordmask = ~0x1FF;
	unsigned int coordshift = 1;
	if (GAME_IS_CRUSADER) {
		coordmask = ~0x3FF;
		coordshift = 2;
	}

	// Expand it
	if (!(flags & FLG_FASTAREA)) 
	{
		MapGlob* glob = GameData::get_instance()->getGlob(quality);
		if (!glob) return;

		std::vector<GlobItem>::iterator iter;
		for (iter = glob->contents.begin(); iter!=glob->contents.end(); ++iter)
		{
			GlobItem& globitem = *iter;
			Item* item = ItemFactory::createItem(globitem.shape,globitem.frame,
												 0,
												 FLG_DISPOSABLE|FLG_FAST_ONLY,
												 0, 0, 0, true);


			// calculate object's world position
			sint32 itemx = (x & coordmask) + (globitem.x << coordshift) + 1;
			sint32 itemy = (y & coordmask) + (globitem.y << coordshift) + 1;
			sint32 itemz = z + globitem.z;

			item->move(itemx, itemy, itemz);
		}
	}

	Item::enterFastArea();
}

void GlobEgg::saveData(ODataSource* ods)
{
	Item::saveData(ods);
}

bool GlobEgg::loadData(IDataSource* ids, uint32 version)
{
	if (!Item::loadData(ids, version)) return false;

	return true;
}
