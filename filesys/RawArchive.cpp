/*
Copyright (C) 2005 The Pentagram team

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

#include "filesys/RawArchive.h"
#include "filesys/IDataSource.h"
#include "filesys/ArchiveFile.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(RawArchive,Pentagram::Archive);

RawArchive::~RawArchive()
{
	Archive::uncache();
}

void RawArchive::cache(uint32 index)
{
	if (index >= count) return;
	if (objects.empty()) objects.resize(count);

	if (objects[index]) return;

	objects[index] = getRawObject(index);
}

void RawArchive::uncache(uint32 index)
{
	if (index >= count) return;
	if (objects.empty()) return;

	if (objects[index]) {
		delete[] objects[index];
		objects[index] = 0;
	}
}

bool RawArchive::isCached(uint32 index)
{
	if (index >= count) return false;
	if (objects.empty()) return false;

	return (objects[index] != 0);
}

const uint8* RawArchive::get_object_nodel(uint32 index)
{
	if (index >= count) return 0;
	cache(index);
	return objects[index];
}

uint8* RawArchive::get_object(uint32 index)
{
	if (index >= count) return 0;

	if (index < objects.size() && objects[index]) {
		// already cached
		uint32 size = getRawSize(index);
		if (size == 0) return 0;
		uint8* object = new uint8[size];
		std::memcpy(object, objects[index], size);
		return object;
	}

	return getRawObject(index);
}

uint32 RawArchive::get_size(uint32 index)
{
	if (index >= count) return 0;
	return getRawSize(index);
}

IDataSource* RawArchive::get_datasource(uint32 index)
{
	if (index >= count) return 0;
	cache(index);

	if (!objects[index]) return 0;

	return new IBufferDataSource(objects[index], getRawSize(index));
}
