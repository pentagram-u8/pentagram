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

#ifndef OBJECT_H
#define OBJECT_H

class Usecode;

class Object
{
public:
	Object() : objid(0xFFFF) {}
	virtual ~Object();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	uint16 getObjId() const { return objid; }
	virtual uint16 assignObjId(); //get and assign self (and contents) an objID
	virtual void clearObjId(); // clear objID of self (and contents)

	uint32 callUsecode(uint16 classid, uint16 offset, Usecode *u, const uint8* args=0, int argsize=0);

protected:
	uint16 objid;
};

#endif
