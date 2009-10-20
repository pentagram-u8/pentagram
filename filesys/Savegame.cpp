/*
Copyright (C) 2003-2005 The Pentagram team

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

#include "filesys/Savegame.h"
#include "filesys/IDataSource.h"
#include "filesys/ZipFile.h"

Savegame::Savegame(IDataSource* ds)
{
	zipfile = new ZipFile(ds);
}

Savegame::~Savegame()
{
	delete zipfile;
}

uint32 Savegame::getVersion()
{
	IDataSource* ids = getDataSource("VERSION");
	if (!ids || ids->getSize() != 4) return 0;

	uint32 version = ids->read4();
	delete ids;

	return version;
}

std::string Savegame::getDescription()
{
	return zipfile->getComment();
}

IDataSource* Savegame::getDataSource(const std::string& name)
{
	uint32 size;
	uint8* data = zipfile->getObject(name, &size);
	return new IBufferDataSource(data, size, false, true);
}
