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

class idMan;
class Object;
class Actor;
class IDataSource;
class ODataSource;

typedef Object* (*ObjectLoadFunc)(IDataSource*);

class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();

	static ObjectManager* get_instance() { return objectmanager; }

	void reset();

	uint16 assignObjId(Object* obj, ObjId id=0xFFFF);
	uint16 assignActorObjId(Actor* obj, ObjId id=0xFFFF);
	void clearObjId(ObjId objid);
	Object* getObject(ObjId objid) const;

	void objectStats();
	void objectTypes();

	void save(ODataSource* ods);
	bool load(IDataSource* ids);

	Object* loadObject(IDataSource* ids);

	//! "ObjectManager::objectTypes" console command
	static void ConCmd_objectTypes(const Pentagram::istring &args);

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

// a bit of a hack to prevent having to write a load function for
// every object
template<class T>
struct ObjectLoader {
	static Object* load(IDataSource* ids) {
		T* p = new T();
		bool ok = p->loadData(ids);
		if (!ok) {
			delete p;
			p = 0;
		}
		return p;
	}
};


#endif
