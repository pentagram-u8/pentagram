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

#include "Savegame.h"
#include "IDataSource.h"

Savegame::Savegame(IDataSource* ds_)
	: NamedFlex()
{
	ds = ds_;

	if (ds->getSize() < 21) {
		// 17 for "PentagramSavegame", 4 for number of entries
		count = 0;
		return;
	}

	char idbuf[17];
	ds->read(idbuf, 17);
	if (memcmp(idbuf, "PentagramSavegame", 17) != 0) {
		count = 0;
		return;
	}

	count = ds->read4();

	offsets.resize(count);
	sizes.resize(count);
	names.resize(count);

	for (unsigned int i = 0; i < count; ++i)
	{
		uint32 namelen = ds->read4();
		char* buf = new char[namelen+1];
		ds->read(buf, static_cast<sint32>(namelen));
		buf[namelen] = 0;
		names[i] = buf;
		indices[names[i]] = i;
		delete[] buf;
		sizes[i] = ds->read4();
		offsets[i] = ds->getPos();
		ds->skip(sizes[i]); // skip data
	}
}

Savegame::~Savegame()
{

}

uint16 Savegame::getVersion()
{
	IDataSource* ids = get_datasource("VERSION");
	if (!ids || ids->getSize() < 2) return 0;

	uint16 version = ids->read2();
	delete ids;

	return version;
}

uint32 Savegame::get_size(uint32 index)
{
	if (index >= count) return 0;
	return sizes[index];
}

uint32 Savegame::get_offset(uint32 index)
{
	if (index >= count) return 0;
	return offsets[index];
}
