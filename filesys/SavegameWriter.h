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

#ifndef SAVEGAMEWRITER_H
#define SAVEGAMEWRITER_H

class ODataSource;
class OBufferDataSource;

class SavegameWriter
{
public:
	SavegameWriter() : ds(0), writtencount(0), realcount(0) { }
	explicit SavegameWriter(ODataSource* ds);
	virtual ~SavegameWriter();

	//! write the savegame header
	//! \param count the number of files in this savegame
	//!              (can be written later if unknown)
	virtual void start(uint32 count=0);

	//! write the savegame's global version
	void writeVersion(uint16 version);

	//! write a file to the savegame
	//! \param name name of the file
	//! \param data the data
	//! \param size (in bytes) of data
	virtual void writeFile(const char* name, uint8* data, uint32 size);

	//! write a file to the savegame from an OBufferDataSource
	//! \param name name of the file
	//! \param buf the OBufferDataSource to save
	void writeFile(const char* name, OBufferDataSource* buf);

	//! write the written number of files into the savegame header,
	//! if necessary
	virtual void fixupCount();

protected:
	ODataSource* ds;
	uint32 writtencount;
	uint32 realcount;
};

#endif
