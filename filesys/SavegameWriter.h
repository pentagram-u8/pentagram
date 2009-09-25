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

#ifndef SAVEGAMEWRITER_H
#define SAVEGAMEWRITER_H

#include <string>

class ODataSource;
class OAutoBufferDataSource;

class SavegameWriter
{
public:
	explicit SavegameWriter(ODataSource* ds);
	virtual ~SavegameWriter();

	//! write the savegame's description.
	bool writeDescription(const std::string& desc);

	//! write the savegame's global version
	bool writeVersion(uint32 version);

	//! write a file to the savegame
	//! \param name name of the file
	//! \param data the data
	//! \param size (in bytes) of data
	virtual bool writeFile(const char* name, const uint8* data, uint32 size);

	//! write a file to the savegame from an OAutoBufferDataSource
	//! \param name name of the file
	//! \param buf the OBufferDataSource to save
	bool writeFile(const char* name, OAutoBufferDataSource* buf);

	//! finish savegame
	bool finish();

protected:
	ODataSource* ds;
	std::string comment;
	void* zipfile;
};

#endif
