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

#include "ObjectManager.h"
#include "UCMachine.h"
#include "UCList.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Container,Item);

Container::Container()
{

}


Container::~Container()
{
	// TODO: handle container's contents.
	// Either destroy the contents, or move them up to this container's parent?



	// if we don't have an objid, we _must_ delete children
	if (objid == 0xFFFF) {
		std::list<Item*>::iterator iter;
		for (iter = contents.begin(); iter != contents.end(); ++iter) {
			delete (*iter);
		}
	}
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


bool Container::CanAddItem(Item* item, bool checkwghtvol)
{
	if (!item) return false;
	if (item->getParent() == this) return true; // already in here

	Container *c = p_dynamic_cast<Container*>(item);
	if (c) {
		// To quote Exult: "Watch for snake eating itself."
		Container* p = this;
		do {
			if (p == c)
				return false;
		} while ((p = p->getParent()) != 0);
	}

	if (checkwghtvol) {
		//TODO: check weight and volume

	}

	return true;
}

bool Container::addItem(Item* item, bool checkwghtvol)
{
	if (!CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == this) return true; // already in here

	contents.push_back(item);
	return true;
}


bool Container::removeItem(Item* item)
{
	std::list<Item*>::iterator iter;

	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		if (*iter == item) {
			contents.erase(iter);
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
		Container *cont = p_dynamic_cast<Container*>(item);
		if (cont) cont->destroyContents();
		item->destroy(); 
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

void Container::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Item::saveData(ods);
	ods->write4(contents.size());
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->save(ods);
	}
}

bool Container::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Item::loadData(ids)) return false;

	uint32 contentcount = ids->read4();

	// read contents
	for (unsigned int i = 0; i < contentcount; ++i)
	{
		Object* obj = ObjectManager::get_instance()->loadObject(ids);
		Item* item = p_dynamic_cast<Item*>(obj);
		if (!item) return false;

		addItem(item);
		item->setParent(this);
	}

	return true;
}


uint32 Container::I_removeContents(const uint8* args, unsigned int /*argsize*/)
{
	ARG_CONTAINER_FROM_PTR(container);
	if (!container) return 0;

	container->removeContents();
	return 0;
}

uint32 Container::I_destroyContents(const uint8* args,unsigned int /*argsize*/)
{
	ARG_CONTAINER_FROM_PTR(container);
	if (!container) return 0;

	container->destroyContents();
	return 0;
}
