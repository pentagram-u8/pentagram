/*
Copyright (C) 2002 The Pentagram team

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

#ifndef FLEX_H
#define FLEX_H

#include "IDataSource.h"

class Flex {
 public:
	Flex(IDataSource* ds);
	virtual ~Flex();

	virtual const uint8* get_object_nodel(uint32 index); // return object. DON'T delete or modify!
	virtual uint8* get_object(uint32 index); //return object. delete afterwards
	virtual uint32 get_size(uint32 index);

	// return object as IDataSource. Delete the IDataSource afterwards,
	// but DON'T delete/modify the buffer it points to.
	virtual IDataSource* get_datasource(uint32 index);
	

	virtual uint32 get_count() const { return count; }

	// Loads all data into memory
	virtual void cache();

	// Loads a single item into memory
	virtual void cache(uint32 index);

	// Free all data from memory
	virtual void uncache();

	static bool isFlex(IDataSource* ds);

protected:
	uint8** objects;
	uint32 count;
	IDataSource* ds;

	Flex() : objects(0), count(0), ds(0) { }

};


#endif
