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

#include "SavegameWriter.h"
#include "ODataSource.h"

static const char* saveid = "PentagramSavegame";

SavegameWriter::SavegameWriter(ODataSource* ds_)
	: ds(ds_), writtencount(0), realcount(0)
{

}

SavegameWriter::~SavegameWriter()
{
	if (ds)
		delete ds;
	ds = 0;
}

void SavegameWriter::start(uint32 count)
{
	ds->write(saveid, strlen(saveid));
	ds->write4(count);
	writtencount = count;
}

void SavegameWriter::writeFile(const char* name, uint8* data, uint32 size)
{
	uint32 namelen = strlen(name);
	ds->write4(namelen);
	ds->write(name, namelen);

	ds->write4(size);
	ds->write(data, size);

	realcount++;
}

void SavegameWriter::writeFile(const char* name, OBufferDataSource* ods)
{
	const std::deque<char>& buf = ods->buf();
	unsigned int size = buf.size();

	uint8* tmp = new uint8[size];
	for (unsigned int i = 0; i < size; ++i) {
		tmp[i] = buf[i];
	}

	writeFile(name, tmp, size);

	delete[] tmp;
}

	//! write the written number of files into the savegame header
void SavegameWriter::fixupCount()
{
	if (realcount != writtencount) {
		ds->seek(strlen(saveid));
		ds->write4(realcount);
	}
}

void SavegameWriter::writeVersion(uint16 version)
{
	uint8 buf[2];
	buf[0] = version & 0xFF;
	buf[1] = (version >> 8) & 0xFF;
	writeFile("VERSION", buf, 2);
}
