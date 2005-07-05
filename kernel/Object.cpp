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

#include "pent_include.h"

#include "Object.h"
#include "Kernel.h"
#include "ObjectManager.h"
#include "World.h"
#include "MemoryManager.h"

#include "UCProcess.h"
#include "UCMachine.h"
#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Object);

DEFINE_CUSTOM_MEMORY_ALLOCATION(Object);

Object::~Object()
{
	if (objid != 0xFFFF)
		ObjectManager::get_instance()->clearObjId(objid);
}

ObjId Object::assignObjId()
{
	if (objid == 0xFFFF)
		objid = ObjectManager::get_instance()->assignObjId(this);
	return objid;
}

void Object::clearObjId()
{
	// On clearObjId we kill all processes that belonged to us
	Kernel::get_instance()->killProcesses(objid, 6, true);

	if (objid != 0xFFFF)
		ObjectManager::get_instance()->clearObjId(objid);
	objid = 0xFFFF;
}

void Object::dumpInfo()
{
	pout << "Object " << getObjId() << " (class "
		 << GetClassType().class_name << ")" << std::endl;
}

ProcId Object::callUsecode(uint16 classid, uint16 offset,
						   const uint8* args, int argsize)
{
	uint32 objptr = UCMachine::objectToPtr(getObjId());
	UCProcess* p = new UCProcess(classid, offset, objptr, 2, args, argsize);
	return Kernel::get_instance()->addProcess(p);
}


void Object::save(ODataSource* ods)
{
	writeObjectHeader(ods);
	saveData(ods); // virtual
}

void Object::writeObjectHeader(ODataSource* ods)
{
	const char* cname = GetClassType().class_name; // note: virtual
	uint16 clen = strlen(cname);

	ods->write2(clen);
	ods->write(cname, clen);
}

void Object::saveData(ODataSource* ods)
{
	// note: Object is unversioned. If we ever want to version it,
	// increase the global savegame version

	ods->write2(objid);
}

bool Object::loadData(IDataSource* ids, uint32 version)
{
	objid = ids->read2();

	return true;
}
