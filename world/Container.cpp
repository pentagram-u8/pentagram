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

#include "Container.h"


Container::Container()
{

}


Container::~Container()
{
	// TODO: handle container's contents.
	// Either destroy the contents, or move them up to this container's parent?
}


bool Container::AddItem(Item* item)
{
	if (item == 0) return false;

	// TODO: get rid of dynamic cast maybe? replace by our own?
	Container *c = dynamic_cast<Container*>(item);
	if (c) {
		// To quote Exult: "Watch for snake eating itself."
		Container* p = this;
		do {
			if (p == c)
				return false;
		} while ((p = p->getParent()) != 0);
	}


	// TODO: Check weight, volume, if item is already in the container?

	contents.push_back(item);

	return true;
}


bool Container::RemoveItem(Item* item)
{
	std::list<Item*>::iterator iter;

	// TODO: I'm sure there are things to check for here

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (*iter == item) {
			contents.erase(iter);
			return true;
		}
	}

	return false;
}
