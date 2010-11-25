/*
Copyright (C) 2003-2004 The Pentagram team

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

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <vector>
#include <map>
#include <set>

class idMan;
class Object;
class Actor;
class IDataSource;
class ODataSource;

typedef Object* (*ObjectLoadFunc)(IDataSource*, uint32);

class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	static ObjectManager* get_instance() { return objectmanager; }

	void reset();

	uint16 assignObjId(Object* obj, ObjId id=0xFFFF);
	uint16 assignActorObjId(Actor* obj, ObjId id=0xFFFF);
	bool reserveObjId(ObjId objid);
	void clearObjId(ObjId objid);
	Object* getObject(ObjId objid) const;

	//! increase the maximum allowed object ID
	//! Note: this shouldn't be used in normal circumstances.
	//!       It exists for dumpMap currently. If that is rewritten not
	//!       to need more than 32768 object IDs, this function should be
	//!       deleted.
	void allow64kObjects();


	void objectStats();
	void objectTypes();

	void save(ODataSource* ods);
	bool load(IDataSource* ids, uint32 version);

	Object* loadObject(IDataSource* ids, uint32 version);
	Object* loadObject(IDataSource* ids, std::string classname,uint32 version);

	//! "ObjectManager::objectTypes" console command
	static void ConCmd_objectTypes(const Console::ArgvType &argv);
	//! "ObjectManager::objectInfo" console command
	static void ConCmd_objectInfo(const Console::ArgvType &argv);

	std::vector<Object*> objects;
	idMan* objIDs;
	idMan* actorIDs;

private:
	void setupLoaders();

	void addObjectLoader(std::string classname, ObjectLoadFunc func)
		{ objectloaders[classname] = func; }
	std::map<std::string, ObjectLoadFunc> objectloaders;

	static ObjectManager* objectmanager;
};

#endif
