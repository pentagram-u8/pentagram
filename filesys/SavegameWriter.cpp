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

bool SavegameWriter::start(uint32 count)
{
	ds->write(saveid, strlen(saveid));
	ds->write4(count);
	writtencount = count;

	return true;
}

bool SavegameWriter::writeFile(const char* name,
							   const uint8* data, uint32 size)
{
	perr << name << ": " << size << std::endl;
	uint32 namelen = strlen(name);
	ds->write4(namelen);
	ds->write(name, namelen);

	ds->write4(size);
	ds->write(data, size);

	realcount++;

	return true;
}

bool SavegameWriter::writeFile(const char* name, OAutoBufferDataSource* ods)
{
	return writeFile(name, ods->getBuf(), ods->getSize());
}

bool SavegameWriter::finish()
{
	if (!ds) return false;

	if (realcount != writtencount) {
		ds->seek(strlen(saveid));
		ds->write4(realcount);
	}

	delete ds;
	ds = 0;

	return true;
}

bool SavegameWriter::writeVersion(uint32 version)
{
	uint8 buf[4];
	buf[0] = version & 0xFF;
	buf[1] = (version >> 8) & 0xFF;
	buf[2] = (version >> 16) & 0xFF;
	buf[3] = (version >> 24) & 0xFF;
	return writeFile("VERSION", buf, 4);
}
