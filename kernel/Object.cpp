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

#include "Object.h"
#include "World.h"

#include "UCProcess.h"
#include "UCMachine.h"


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Object);

Object::~Object()
{
	if (objid != 0xFFFF)
		World::get_instance()->clearObjId(objid);
}

uint16 Object::assignObjId()
{
	if (objid == 0xFFFF)
		objid = World::get_instance()->assignObjId(this);
	return objid;
}

void Object::clearObjId()
{
	if (objid != 0xFFFF)
		World::get_instance()->clearObjId(objid);
	objid = 0xFFFF;
}

uint32 Object::callUsecode(uint16 classid, uint16 offset, Usecode* u)
{
	uint32 objptr = UCMachine::objectToPtr(getObjId());
	UCProcess* p = new UCProcess(u);
	uint16 pid = UCMachine::get_instance()->addProcess(p);

	p->load(classid, offset, objptr, 2);

	return pid;
}
