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

#include "ItemMoveProcess.h"
#include "Item.h"
#include "World.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(ItemMoveProcess,Process);

ItemMoveProcess::ItemMoveProcess(Item* item_, sint32 to_x_, sint32 to_y_,
								 sint32 to_z_, int speed_)
{
	assert(item_);
	item = item_->getObjId();
	
	to_x = to_x_;
	to_y = to_y_;
	to_z = to_z_;

	item_->getLocation(from_x, from_y, from_z);

	speed = speed_;

	currentpos = 0;
}

bool ItemMoveProcess::run(const uint32 framenum)
{
	// some values to make items use a nice arc :-)
	// (No, this isn't permanent, but it makes the fish fly nicely ;-) )
	static const int sinvals[21] = 
		{0, 156, 309, 454, 588, 705, 809, 891, 952, 987, 1000,
		 987, 952, 891, 809, 705, 588, 454, 309, 156, 0};

	//!! this needs timing
	// without main timing I can't really determine how many 'ticks' 
	// each frame should last.

	Item *it = p_dynamic_cast<Item*>(World::get_instance()->getObject(item));

	if (!it) {
		// item gone
		terminate();
		return false;
	}

	sint32 x, y, z;

	currentpos += 1;
	if (currentpos > speed) currentpos = speed;

	// trajectory is rather boring currently
	// also speed doesn't work the way it (probably) should this way
	x = from_x + ((to_x - from_x) * currentpos)/speed;
	y = from_y + ((to_y - from_y) * currentpos)/speed;
	z = from_z + ((to_z - from_z) * currentpos)/speed + sinvals[(20*currentpos)/speed]/25;

	it->move(x,y,z);

	if (currentpos >= speed)
		terminate();

	return true;
}
