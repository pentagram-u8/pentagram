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
#include "MainShapeFlex.h"
#include "ShapeInfo.h"
#include "Item.h"
#include "Container.h"

Item* ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
							  uint32 flags, uint32 npcnum, uint32 mapnum)
{
	// check what class to create
	ShapeInfo *info = MainShapeFlex::get_instance()->getShapeInfo(shape);
	if (info == 0) return 0;

	uint32 family = info->family;

	switch (family) {
	case ShapeInfo::SF_GENERIC:
	case ShapeInfo::SF_QUALITY:
	case ShapeInfo::SF_QUANTITY:
	case ShapeInfo::SF_BREAKABLE:
	case ShapeInfo::SF_REAGENT:
	{
		// 'simple' item

		Item* item = new Item();
		item->shape = shape;
		item->frame = frame;
		item->quality = quality;
		item->flags = flags;
		item->npcnum = npcnum;
		item->mapnum = mapnum;
		return item;
	}

	case ShapeInfo::SF_CONTAINER:
	{
		// container

		Container* container = new Container();
		container->shape = shape;
		container->frame = frame;
		container->quality = quality;
		container->flags = flags;
		container->npcnum = npcnum;
		container->mapnum = mapnum;
		return container;
	}

	case ShapeInfo::SF_GLOBEGG:
	{
		// glob
		return 0;
	}

	case ShapeInfo::SF_UNKEGG:
	{

		return 0;
	}

	case ShapeInfo::SF_MONSTEREGG:
	{

		return 0;
	}

	case ShapeInfo::SF_TELEPORTEGG:
	{

		return 0;
	}


	default:
		return 0;
	}
}
