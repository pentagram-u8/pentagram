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

	int i;
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
	int id = first;

	// Set the first in the list to next
	first = ids[id];

	// Set us to used
	ids[id] = 0;

	// If there is no first, there is no list, cause there's none left
	// So clear the last pointer
	if (!first) last = 0;

	return id;

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

