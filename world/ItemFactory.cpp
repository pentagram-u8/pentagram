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

#include "ItemFactory.h"
#include "Item.h"
#include "Container.h"

Item* ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
							  uint32 flags, uint32 mapNum)
{
	// TODO: check what class to create... (need to read typeflag.dat)

	Item* item = new Item();
	item->shape = shape;
	item->frame = frame;
	item->quality = quality;
	item->flags = flags;
	item->mapNum = mapNum;

	return item;
}
