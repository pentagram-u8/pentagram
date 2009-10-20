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

#ifndef U8SAVEFILE_H
#define U8SAVEFILE_H

#include <vector>

#include "filesys/NamedArchiveFile.h"

class IDataSource;

class U8SaveFile : public NamedArchiveFile {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! create U8SaveFile from datasource; U8SaveFile takes ownership of ds
	//! and deletes it when destructed
	explicit U8SaveFile(IDataSource* ds);
	virtual ~U8SaveFile();

	virtual bool exists(const std::string& name);

	virtual uint8* getObject(const std::string& name, uint32* size=0);

	virtual uint32 getSize(const std::string& name);

	virtual uint32 getCount() { return count; }

	static bool isU8SaveFile(IDataSource* ds);

protected:
	IDataSource* ds;
	uint32 count;

	std::map<std::string, uint32> indices;
	std::vector<uint32> offsets;
	std::vector<uint32> sizes;

private:
	bool readMetadata();
	bool findIndex(const std::string& name, uint32& index);
};


#endif
