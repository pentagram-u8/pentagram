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

#ifndef ITEMFACTORY_H
#define ITEMFACTORY_H

class Item;
class Actor;
class Object;
class IDataSource;

class ItemFactory
{
public:

	//! create an item.
	//! look up (where?) what type of item the given shape is, and
	//! create an instance of the corresponding Item class.
	//! If objid is set, assign item an objectid
	static Item* createItem(uint32 shape, uint32 frame, uint16 quality,
							uint16 flags, uint16 npcnum, uint16 mapnum,
							uint32 extendedflags, bool objid);

	//! create an actor.
	//! If objid is set, assign actor an actor-objid
	static Actor* createActor(uint32 shape, uint32 frame, uint16 quality,
							  uint16 flags, uint16 npcnum, uint16 mapnum,
							  uint32 extendedflags, bool objid);
};

#endif
