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

#include "pent_include.h"

#include "Flex.h"

Flex::Flex(IDataSource* ds_) : objects(0), ds(ds_)
{
	ds->seek(0x54);
	count = ds->read4();
}

Flex::~Flex()
{
	// Delete 'cached' objects
	if (objects) {
		// Want the actual count
		int actual_count = get_count();

		while (actual_count--)
			delete [] objects[actual_count];

		delete [] objects;
	}
}

//static
bool Flex::isFlex(IDataSource* ds)
{
	ds->seek(0);
	for (int i = 0; i < (0x54 / 4); i++)
		if (ds->read4() != 0x1A1A1A1A) return false;

	return true;
}

const uint8* Flex::get_object_nodel(uint32 index)
{
	if (index >= count) return 0;

	if (!objects || !objects[index]) {
		cache(index);
	}

	if (!objects) return 0;

	return objects[index];
}

uint8* Flex::get_object(uint32 index)
{
	if (index >= count) return 0;

	uint8 *object;
	uint32 length;

	// It might be cached
	if (objects && objects[index]) {
		length = get_size(index);
		object = new uint8[length];
		std::memcpy (object, objects[index], length);
		return object;
	}

	ds->seek(0x80 + 8*index);
	uint32 offset = ds->read4();
	length = ds->read4();

	if (length == 0) return 0;

	object = new uint8[length];

	ds->seek(offset);
	ds->read(object, length);

	return object;
}

uint32 Flex::get_size(uint32 index)
{
	if (index >= count) return 0;

	ds->seek(0x84 + 8*index);
	uint32 length = ds->read4();	

	return length;
}

IDataSource* Flex::get_datasource(uint32 index)
{
	if (index >= count) return 0;

	IDataSource* d = new IBufferDataSource(get_object_nodel(index),
										   get_size(index));

	return d;
}

// Loads all data into memory
void Flex::cache()
{
	uint32 actual_count = get_count();

	// Create objects array if required
	if (!objects) {
		objects = new uint8 *[actual_count];
		std::memset (objects, 0, actual_count*sizeof(uint8*));
	}

	while (actual_count--) {
		if (!objects[actual_count]) {
			objects[actual_count] = get_object(actual_count);
		}
	}
}

// Loads all data into memory
void Flex::cache(uint32 index)
{
	uint32 actual_count = get_count();

	// Create objects array if required
	if (!objects) {
		objects = new uint8 *[actual_count];
		std::memset (objects, 0, actual_count*sizeof(uint8*));
	}

	if (index >= actual_count || objects[index]) return;

	objects[index] = get_object(index);
}

// Free all data from memory
void Flex::uncache()
{
	// Delete 'cached' objects
	if (objects) {

		// Want the actual count
		uint32 actual_count = get_count();

		// Delete cached object
		while (actual_count--)
			delete [] objects[actual_count];

		// Delete objects
		delete [] objects;
		objects = 0;
	}
}
