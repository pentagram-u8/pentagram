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

#include "U8Save.h"
#include "IDataSource.h"

U8Save::U8Save(IDataSource* ds_)
	: Flex()
{
	ds = ds_;
	ds->seek(0x18);
	count = ds->read2();

	offsets.resize(count);
	sizes.resize(count);
	names.resize(count);

	for (unsigned int i = 0; i < count; ++i)
	{
		uint32 namelen = ds->read4();
		char* buf = new char[namelen];
		ds->read(buf, static_cast<sint32>(namelen));
		names[i] = buf;
		indices[names[i]] = i;
		delete[] buf;
		sizes[i] = ds->read4();
		offsets[i] = ds->getPos();
		ds->skip(sizes[i]); // skip data
	}
}

U8Save::~U8Save()
{
	// Q: Do we need to delete the datasource?
}

uint32 U8Save::get_size(uint32 index)
{
	if (index >= count) return 0;
	return sizes[index];
}

uint32 U8Save::get_offset(uint32 index)
{
	if (index >= count) return 0;
	return offsets[index];
}

IDataSource* U8Save::get_datasource(const char* filename)
{
	std::string fname = filename;
	std::map<std::string,uint32>::iterator iter = indices.find(fname);
	if (iter == indices.end()) return 0;

	return Flex::get_datasource(iter->second);
}
