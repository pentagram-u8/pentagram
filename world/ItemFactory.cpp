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
#include "GameData.h"
#include "MainShapeFlex.h"
#include "ShapeInfo.h"
#include "Item.h"
#include "Container.h"
#include "Actor.h"
#include "MainActor.h"
#include "GlobEgg.h"

Item* ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
							  uint32 flags, uint32 npcnum, uint32 mapnum,
							  uint32 extendedflags)
{
	// check what class to create
	ShapeInfo *info = GameData::get_instance()->getMainShapes()->
		getShapeInfo(shape);
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
		item->extendedflags = extendedflags;
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
		container->extendedflags = extendedflags;
		return container;
	}

	case ShapeInfo::SF_GLOBEGG:
	{
		// glob egg

		GlobEgg* globegg = new GlobEgg();
		globegg->shape = shape;
		globegg->frame = frame;
		globegg->quality = quality;
		globegg->flags = flags;
		globegg->npcnum = npcnum;
		globegg->mapnum = mapnum;
		globegg->extendedflags = extendedflags;
		return globegg;
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



Actor* ItemFactory::createActor(uint32 shape, uint32 frame, uint16 quality,
								uint32 flags, uint32 npcnum, uint32 mapnum,
								uint32 extendedflags)
{
	// this function should probably differentiate between the Avatar,
	// NPCs, monsters?

	if (npcnum == 0) // or do monsters have npcnum 0? we'll see...
		return 0;

	if (npcnum == 1) {
		// Main Actor

		MainActor* actor = new MainActor();
		actor->shape = shape;
		actor->frame = frame;
		actor->quality = quality;
		actor->flags = flags;
		actor->npcnum = npcnum;
		actor->mapnum = mapnum;
		actor->objid = npcnum;
		actor->extendedflags = extendedflags;
		return actor;
	}

	// 'normal' Actor/NPC
	Actor* actor = new Actor();
	actor->shape = shape;
	actor->frame = frame;
	actor->quality = quality;
	actor->flags = flags;
	actor->npcnum = npcnum;
	actor->mapnum = mapnum;
	actor->objid = npcnum;
	actor->extendedflags = extendedflags;

	return actor;
}
