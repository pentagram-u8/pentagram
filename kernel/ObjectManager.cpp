/*
Copyright (C) 2002,2003 The Pentagram team

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

#include "ObjectManager.h"

#include "idMan.h"
#include "Object.h"
#include "Item.h"
#include "Actor.h"
#include "Gump.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"

// temporary...
#include "MainActor.h"
#include "Egg.h"
#include "MonsterEgg.h"
#include "TeleportEgg.h"
#include "GlobEgg.h"
#include "GameMapGump.h"
#include "DesktopGump.h"
#include "ConsoleGump.h"
#include "AskGump.h"
#include "BarkGump.h"
#include "ContainerGump.h"
#include "SimpleTextWidget.h"
#include "ButtonWidget.h"

//#define DUMP_OBJECTTYPES

#ifdef DUMP_OBJECTTYPES
#include <map>
#endif

ObjectManager* ObjectManager::objectmanager = 0;

ObjectManager::ObjectManager()
{
	assert(objectmanager == 0);
	objectmanager = this;

	setupLoaders();

	objects.resize(65536);

	//!CONSTANTS
	objIDs = new idMan(256,65534);	// Want range of 256 to 65534
	objIDs->reserveID(666);		// 666 is reserved for the Guardian Bark hack
	actorIDs = new idMan(1,255);
}

ObjectManager::~ObjectManager()
{
	reset();

	objectmanager = 0;

	delete objIDs;
	delete actorIDs;
}

void ObjectManager::reset()
{
	for (unsigned int i = 0; i < objects.size(); ++i) {
		if (objects[i] == 0) continue;
#if 0
		Item* item = p_dynamic_cast<Item*>(objects[i]);
		if (item && item->getParent()) continue; // will be deleted by parent
#endif
		Gump* gump = p_dynamic_cast<Gump*>(objects[i]);
		if (gump && gump->GetParent()) continue; // will be deleted by parent
		delete objects[i];
	}

	for (unsigned int i = 0; i < objects.size(); ++i) {
#if 0
		if (objects[i]) {
			perr << i << ": " << objects[i]->GetClassType().class_name << std::endl;
			delete objects[i];

		}
#endif
		assert(objects[i] == 0);
	}


	objects.clear();
	objects.resize(65536);
	objIDs->clearAll();
	objIDs->reserveID(666);		// 666 is reserved for the Guardian Bark hack
	actorIDs->clearAll();
}

void ObjectManager::objectStats()
{
	unsigned int npccount = 0, objcount = 0;

	//!constants
	for (unsigned int i = 1; i < 256; i++) {
		if (objects[i] != 0)
			npccount++;
	}
	for (unsigned int i = 256; i < objects.size(); i++) {
		if (objects[i] != 0)
			objcount++;
	}

#ifdef DUMP_OBJECTTYPES
	std::map<std::string, unsigned int> objecttypes;
	for (unsigned int i = 1; i < objects.size(); ++i) {
		Object* o = objects[i];
		if (!o) continue;
		objecttypes[o->GetClassType().class_name]++;
	}
#endif

	pout << "Object memory stats:" << std::endl;
	pout << "NPCs       : " << npccount << "/255" << std::endl;
	pout << "Objects    : " << objcount << "/65279" << std::endl;

#ifdef DUMP_OBJECTTYPES
	std::map<std::string, unsigned int>::iterator iter;
	for (iter = objecttypes.begin(); iter != objecttypes.end(); ++iter) {
		pout << (*iter).first << ": " << (*iter).second << std::endl;
	}
#endif
}

uint16 ObjectManager::assignObjId(Object* obj, uint16 new_objid)
{
	if (new_objid == 0xFFFF)
		new_objid = objIDs->getNewID();
	else
		objIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = obj;
	}
	return new_objid;
}

uint16 ObjectManager::assignActorObjId(Actor* actor, uint16 new_objid)
{
	if (new_objid == 0xFFFF)
		new_objid = actorIDs->getNewID();
	else
		actorIDs->reserveID(new_objid);

	// failure???
	if (new_objid != 0) {
		assert(objects[new_objid] == 0);
		objects[new_objid] = actor;
	}
	return new_objid;
}

void ObjectManager::clearObjId(uint16 objid)
{
	// need to make this assert check only permanent NPCs
//	assert(objid >= 256); // !constant
	if (objid >= 256) // !constant
		objIDs->clearID(objid);
	else
		actorIDs->clearID(objid);

	objects[objid] = 0;
}

Object* ObjectManager::getObject(uint16 objid) const
{
	return objects[objid];
}

void ObjectManager::save(ODataSource* ods)
{
	ods->write2(1); // objects savegame version 1

	objIDs->save(ods);
	actorIDs->save(ods);

	// first, have all GlobEggs check if their contents changed:
	for (unsigned int i = 0; i < objects.size(); ++i) {
		GlobEgg* egg = p_dynamic_cast<GlobEgg*>(objects[i]);
		if (!egg) continue;
		egg->checkContents();
	}


	for (unsigned int i = 0; i < objects.size(); ++i) {
		Object* object = objects[i];
		if (!object) continue;

		// we need to ensure container contents are stored directly after
		// the container, and gump children directly after their parent
		// hackish...
		Item* item = p_dynamic_cast<Item*>(object);
		if (item && item->getParent()) continue;
		Gump* gump = p_dynamic_cast<Gump*>(object);
		if (gump && gump->GetParent()) continue;

		object->save(ods);
	}

	ods->write2(0);
}

bool ObjectManager::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	if (!objIDs->load(ids)) return false;
	if (!actorIDs->load(ids)) return false;

	do {
		// peek ahead for terminator
		uint16 classlen = ids->read2();
		if (classlen == 0) break;
		ids->skip(-2);

		Object* obj = loadObject(ids);
		if (!obj) return false;
	} while(true);

	// finally, have all GlobEggs restore any unsaved contents
	for (unsigned int i = 0; i < objects.size(); ++i) {
		GlobEgg* egg = p_dynamic_cast<GlobEgg*>(objects[i]);
		if (!egg) continue;
		egg->restoreContents();
	}


	return true;
}


Object* ObjectManager::loadObject(IDataSource* ids)
{

	uint16 classlen = ids->read2();
	char* buf = new char[classlen+1];
	ids->read(buf, classlen);
	buf[classlen] = 0;

	std::string classname = buf;
	delete[] buf;

	std::map<std::string, ObjectLoadFunc>::iterator iter;
	iter = objectloaders.find(classname);

	if (iter == objectloaders.end()) {
		perr << "Unknown Object class: " << classname << std::endl;
		return 0;
	}

	Object* obj = (*(iter->second))(ids);

	if (!obj) {
		return 0;
	}
	uint16 objid = obj->getObjId();

	if (objid != 0xFFFF)
		objects[objid] = obj;

	return obj;
}

void ObjectManager::setupLoaders()
{
	addObjectLoader("Item", ObjectLoader<Item>::load);
	addObjectLoader("Container", ObjectLoader<Container>::load);
	addObjectLoader("Actor", ObjectLoader<Actor>::load);
	addObjectLoader("MainActor", ObjectLoader<MainActor>::load);
	addObjectLoader("Egg", ObjectLoader<Egg>::load);
	addObjectLoader("MonsterEgg", ObjectLoader<MonsterEgg>::load);
	addObjectLoader("TeleportEgg", ObjectLoader<TeleportEgg>::load);
	addObjectLoader("GlobEgg", ObjectLoader<GlobEgg>::load);
	addObjectLoader("Gump", ObjectLoader<Gump>::load);
	addObjectLoader("DesktopGump", ObjectLoader<DesktopGump>::load);
	addObjectLoader("GameMapGump", ObjectLoader<GameMapGump>::load);
	addObjectLoader("ConsoleGump", ObjectLoader<ConsoleGump>::load);
	addObjectLoader("ItemRelativeGump", ObjectLoader<ItemRelativeGump>::load);
	addObjectLoader("AskGump", ObjectLoader<AskGump>::load);
	addObjectLoader("BarkGump", ObjectLoader<BarkGump>::load);
	addObjectLoader("ContainerGump", ObjectLoader<ContainerGump>::load);
	addObjectLoader("SimpleTextWidget", ObjectLoader<SimpleTextWidget>::load);
	addObjectLoader("ButtonWidget", ObjectLoader<ButtonWidget>::load);
}	
