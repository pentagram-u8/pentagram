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

#ifndef IDMAN_H
#define IDMAN_H

//
// idMan. Used to allocate and keep track of unused ids.
// Supposed to be used by Kernel and World for pID and ObjID
//
// The idMan itself uses a nifty linked list that is also an array.
// As such it has the advantages of both. Adds and removals are fast,
// As is checking to see if an ID is used.
//
// idMan works as a LILO (last in last out) for id recycling. So an id that has
// been cleared will not be used until all other currently unused ids have been
// allocated.
//

class idMan
{
	uint16 		begin;
	uint16 		end;

	uint16		*ids;
	uint16		first;
	uint16		last;
public:
	idMan(uint16 begin, uint16 end);
	~idMan();

	void		clearAll();

	uint16		getNewID();
	bool		reserveID(uint16 id); // false if already used
	void		clearID(uint16 id);
	bool		isIDUsed(uint16 id) { return ids[id] == 0 && id != last; }
};

#endif //IDMAN_H
