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

#ifndef FLEXFILE_H
#define FLEXFILE_H

#include "filesys/ArchiveFile.h"

class IDataSource;

class FlexFile : public ArchiveFile {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! create FlexFile from datasource; FlexFile takes ownership of ds
	//! and deletes it when destructed
	explicit FlexFile(IDataSource* ds);
	virtual ~FlexFile();

	virtual bool exists(uint32 index) { return getSize(index) > 0; }
	virtual bool exists(const std::string& name) {
		uint32 index;
		if (nameToIndex(name, index))
			return exists(index);
		else
			return false;
	}

	virtual uint8* getObject(uint32 index, uint32* size=0);
	virtual uint8* getObject(const std::string& name, uint32* size=0) {
		uint32 index;
		if (nameToIndex(name, index))
			return getObject(index, size);
		else
			return 0;
	}
	

	virtual uint32 getSize(uint32 index);
	virtual uint32 getSize(const std::string& name) {
		uint32 index;
		if (nameToIndex(name, index))
			return getSize(index);
		else
			return 0;
	}

	virtual uint32 getCount() { return count; }

	virtual uint32 getIndexCount() { return count; }

	virtual bool isIndexed() const { return true; }
	virtual bool isNamed() const { return false; }

	static bool isFlexFile(IDataSource* ds);

protected:
	bool nameToIndex(const std::string& name, uint32& index);

	IDataSource* ds;
	uint32 count;

private:
	uint32 getOffset(uint32 index);
};


#endif
