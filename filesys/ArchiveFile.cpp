/*
Copyright (C) 2002-2005 The Pentagram team

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

#include "filesys/ArchiveFile.h"
#include "filesys/IDataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(ArchiveFile);

//static
bool ArchiveFile::extractIndexFromName(const std::string& name, uint32& index)
{
	if (name.size() == 0) return false;

	char* endptr;
	long val;

	val = std::strtol(name.c_str(), &endptr, 10);

	// if remainder of name doesn't start with a '.', invalid name
	if (*endptr != '\0' && *endptr != '.') return false;

	if (val < 0) return false;

	index = static_cast<uint32>(val);

	return true;
}

IDataSource* ArchiveFile::getDataSource(uint32 index, bool is_text)
{
	uint32 size;
	uint8* buf = getObject(index, &size);

	if (!buf) return 0;

	return new IBufferDataSource(buf, size, is_text, true);
}

IDataSource* ArchiveFile::getDataSource(const std::string& name, bool is_text)
{
	uint32 size;
	uint8* buf = getObject(name, &size);

	if (!buf) return 0;

	return new IBufferDataSource(buf, size, is_text, true);
}

