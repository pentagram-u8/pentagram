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

#include "World.h"
#include "UCMachine.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Container,Item);

Container::Container()
{

}


Container::~Container()
{
	// TODO: handle container's contents.
	// Either destroy the contents, or move them up to this container's parent?
}


uint16 Container::assignObjId()
{
	uint16 id = Item::assignObjId();

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->assignObjId();
	}

	return id;
}

void Container::clearObjId()
{
	Item::clearObjId();

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->clearObjId();
	}
}


bool Container::AddItem(Item* item)
{
	if (item == 0) return false;

	Container *c = p_dynamic_cast<Container*>(item);
	if (c) {
		// To quote Exult: "Watch for snake eating itself."
		Container* p = this;
		do {
			if (p == c)
				return false;
		} while ((p = p->getParent()) != 0);
	}


	// TODO: Check weight, volume, if item is already in the container?
	// (These checks should be skippable)

	contents.push_back(item);

	item->setFlag(Item::FLG_CONTAINED);

	return true;
}


bool Container::RemoveItem(Item* item)
{
	std::list<Item*>::iterator iter;

	// TODO: I'm sure there are things to check for here

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (*iter == item) {
			contents.erase(iter);
			item->clearFlag(Item::FLG_CONTAINED);
			return true;
		}
	}

	return false;
}

void Container::removeContents()
{
	//!! todo
}


void Container::destroyContents()
{
	while (contents.begin() != contents.end()) {
		Item *item = *(contents.begin());
		item->destroy(); // this (should) remove(s) item from contents
	}
}

void Container::destroy()
{
	//! What do we do with our contents?
	//! (in Exult we remove the contents)

	removeContents();

	Item::destroy();
}

uint32 Container::getTotalWeight()
{
	uint32 weight = Item::getTotalWeight();

	std::list<Item*>::iterator iter;
	
	for (iter = contents.begin(); iter != contents.end(); ++iter) {	
		weight += (*iter)->getTotalWeight();
	}

	return weight;
}

void Container::containerSearch(UCList* itemlist, const uint8* loopscript,
								uint32 scriptsize, bool recurse)
{
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (recurse) {
			// recurse into child-containers
			Container *container = p_dynamic_cast<Container*>(*iter);
			if (container)
				container->containerSearch(itemlist, loopscript,
										   scriptsize, recurse);
		}

		// check item against loopscript
		if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
			uint16 objid = (*iter)->getObjId();
			uint8 buf[2];
			buf[0] = static_cast<uint8>(objid);
			buf[1] = static_cast<uint8>(objid >> 8);
			itemlist->append(buf);
		}
	}	
}


uint32 Container::I_removeContents(const uint8* args, unsigned int /*argsize*/)
{
	ARG_CONTAINER(container);
	if (!container) return 0;

	container->removeContents();
	return 0;
}

uint32 Container::I_destroyContents(const uint8* args,unsigned int /*argsize*/)
{
	ARG_CONTAINER(container);
	if (!container) return 0;

	container->destroyContents();
	return 0;
}
