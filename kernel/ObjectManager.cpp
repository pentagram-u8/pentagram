/*
Copyright (C) 2002-2004 The Pentagram team

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

#include <map>
#include "idMan.h"
#include "Object.h"
#include "Item.h"
#include "Actor.h"
#include "Gump.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"

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
#include "PaperdollGump.h"
#include "TextWidget.h"
#include "ButtonWidget.h"
#include "SlidingWidget.h"
#include "MiniStatsGump.h"

ObjectManager* ObjectManager::objectmanager = 0;

ObjectManager::ObjectManager()
{
	assert(objectmanager == 0);
	objectmanager = this;

	setupLoaders();

	objects.resize(65536);

	//!CONSTANTS
	objIDs = new idMan(256,65534,8192);	// Want range of 256 to 65534
	objIDs->reserveID(666);		// 666 is reserved for the Guardian Bark hack
	actorIDs = new idMan(1,255,255);
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
	unsigned int i;

	for (i = 0; i < objects.size(); ++i) {
		if (objects[i] == 0) continue;
#if 0
		Item* item = p_dynamic_cast<Item*>(objects[i]);
		if (item && item->getParent()) continue; // will be deleted by parent
#endif
		Gump* gump = p_dynamic_cast<Gump*>(objects[i]);
		if (gump && gump->GetParent()) continue; // will be deleted by parent
		delete objects[i];
	}

	for (i = 0; i < objects.size(); ++i) {
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
	unsigned int i, npccount = 0, objcount = 0;

	//!constants
	for (i = 1; i < 256; i++) {
		if (objects[i] != 0)
			npccount++;
	}
	for (i = 256; i < objects.size(); i++) {
		if (objects[i] != 0)
			objcount++;
	}

	pout << "Object memory stats:" << std::endl;
	pout << "NPCs       : " << npccount << "/255" << std::endl;
	pout << "Objects    : " << objcount << "/65279" << std::endl;
}

void ObjectManager::objectTypes()
{
	pout << "Current object types:" << std::endl;
	std::map<std::string, unsigned int> objecttypes;
	for (unsigned int i = 1; i < objects.size(); ++i) {
		Object* o = objects[i];
		if (!o) continue;
		objecttypes[o->GetClassType().class_name]++;
	}

	std::map<std::string, unsigned int>::iterator iter;
	for (iter = objecttypes.begin(); iter != objecttypes.end(); ++iter) {
		pout << (*iter).first << ": " << (*iter).second << std::endl;
	}
}

void ObjectManager::ConCmd_objectTypes(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	ObjectManager::get_instance()->objectTypes();
}

uint16 ObjectManager::assignObjId(Object* obj, ObjId new_objid)
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

uint16 ObjectManager::assignActorObjId(Actor* actor, ObjId new_objid)
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

void ObjectManager::clearObjId(ObjId objid)
{
	// need to make this assert check only permanent NPCs
//	assert(objid >= 256); // !constant
	if (objid >= 256) // !constant
		objIDs->clearID(objid);
	else
		actorIDs->clearID(objid);

	objects[objid] = 0;
}

Object* ObjectManager::getObject(ObjId objid) const
{
	return objects[objid];
}

void ObjectManager::save(ODataSource* ods)
{
	ods->write2(1); // objects savegame version 1

	objIDs->save(ods);
	actorIDs->save(ods);

	for (unsigned int i = 0; i < objects.size(); ++i) {
		Object* object = objects[i];
		if (!object) continue;

		// child items/gumps are saved by their parent.
		Item* item = p_dynamic_cast<Item*>(object);
		if (item && item->getParent()) continue;
		Gump* gump = p_dynamic_cast<Gump*>(object);
		if (gump && gump->GetParent()) continue;

		// some gumps shouldn't be saved (MenuGump)
		if (gump && !gump->mustSave()) continue;

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
		perr << "Error loading object of type " << classname << std::endl;
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
	addObjectLoader("PaperdollGump", ObjectLoader<PaperdollGump>::load);
	addObjectLoader("TextWidget", ObjectLoader<TextWidget>::load);
	addObjectLoader("ButtonWidget", ObjectLoader<ButtonWidget>::load);
	addObjectLoader("SlidingWidget", ObjectLoader<SlidingWidget>::load);
	addObjectLoader("MiniStatsGump", ObjectLoader<MiniStatsGump>::load);
}

