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
#include "idMan.h"

#include "IDataSource.h"
#include "ODataSource.h"

idMan::idMan(uint16 Begin, uint16 End) : begin(Begin), end(End)
{
	// 0 is always reserved, as is 65535
	if (begin == 0) begin = 1;
	if (end == 65535) end = 65534;

	ids = new uint16[end+1];
	clearAll();
}

idMan::~idMan()
{
	delete [] ids;
}

void idMan::clearAll()
{
	first = begin;
	last  = end;

	uint16 i;
	for (i = 0; i < first; i++) ids[i] = 0;		// NPCs always used
	for (     ; i < last;  i++) ids[i] = i+1;	// Free IDs
	ids[last] = 0;								// Terminates the list

}

uint16 idMan::getNewID()
{
	// Uh oh, what to do when there is none
	if (!first) 
	{
		perr << "Unable to allocate id" << std::endl;
		return 0;
	}

	// Get the next id
	uint16 id = first;

	// Set the first in the list to next
	first = ids[id];

	// Set us to used
	ids[id] = 0;

	// If there is no first, there is no list, cause there's none left
	// So clear the last pointer
	if (!first) last = 0;

	return id;

}

bool idMan::reserveID(uint16 id)
{
	if (isIDUsed(id))
		return false; // already used

	if (id == first) {
		first = ids[id];
		ids[id] = 0;
		if (!first) last = 0;
		return true;
	}

	uint16 node = ids[first];
	uint16 prev = first;

	while (node != id && node != 0) {
		prev = node;
		node = ids[node];
	}
	assert(node != 0); // list corrupt...

	ids[prev] = ids[node];
	ids[node] = 0;
	if (last == node)
		last = prev;
	return true;
}

void idMan::clearID(uint16 id)
{
	// Only clear IF it is used. We don't want to screw up the linked list
	// if an id gets cleared twice
	if (!ids[id])
	{
		// If there is a last, then set the last's next to us
		// or if there isn't a last, obviously no list exists,
		// so set the first to us
		if (last) ids[last] = id;
		else first = id;

		// Set the end to us
		last = id;

		// Set our next to terminate
		ids[id] = 0;
	}
}

void idMan::save(ODataSource* ods)
{
	ods->write2(1); // version
	ods->write2(begin);
	ods->write2(end);
	uint16 cur = first;
	while (cur) {
		ods->write2(cur);
		cur = ids[cur];
	}
	ods->write2(0); // terminator
}

bool idMan::load(IDataSource* ds)
{
	uint16 version = ds->read2();
	if (version != 1) return false;

	begin = ds->read2();
	end = ds->read2();

	if (ids) delete[] ids;
	ids = new uint16[end+1];

	for (unsigned int i = 0; i <= end; ++i) {
		ids[i] = 0;
	}
	first = last = 0;

	uint16 cur = ds->read2();
	while (cur) {
		clearID(cur);
		cur = ds->read2();
	}

	return true;
}
