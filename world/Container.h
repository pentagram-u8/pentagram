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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "Item.h"
#include <list>

#include "intrinsics.h"

class UCList;

class Container : public Item
{
	friend class ItemFactory;

public:
	Container();
	virtual ~Container();

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(Item);

	bool AddItem(Item* item);
	bool RemoveItem(Item* item);
	void removeContents();
	void destroyContents();

	void containerSearch(UCList* itemlist, const uint8* loopscript,
						 uint32 scriptsize, bool recurse);
	virtual uint32 getTotalWeight();

	virtual uint16 assignObjId(); //get and assign self (and contents) an objID
	virtual void clearObjId(); // clear objID of self (and contents)

	virtual void destroy();

	INTRINSIC(I_removeContents);
	INTRINSIC(I_destroyContents);

protected:
	std::list<Item*> contents;
};

#endif
