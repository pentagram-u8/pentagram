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

#ifndef NAMEDARCHIVEFILE_H
#define NAMEDARCHIVEFILE_H

#include "filesys/ArchiveFile.h"

class NamedArchiveFile : public ArchiveFile {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	NamedArchiveFile() : indexCount(0) { }
	virtual ~NamedArchiveFile() { }

	virtual bool exists(uint32 index) {
		std::string name;
		return (indexToName(index, name));
	}
	virtual bool exists(const std::string& name)=0;

	virtual uint8* getObject(uint32 index, uint32* size=0) {
		std::string name;
		if (!indexToName(index, name)) return 0;
		return getObject(name, size);
	}
	virtual uint8* getObject(const std::string& name, uint32* size=0)=0;

	virtual uint32 getSize(uint32 index) {
		std::string name;
		if (!indexToName(index, name)) return 0;
		return getSize(name);
	}
	virtual uint32 getSize(const std::string& name)=0;

	virtual uint32 getCount()=0;

	virtual uint32 getIndexCount() { return indexCount; }

	virtual bool isIndexed() const { return false; }
	virtual bool isNamed() const { return true; }

protected:
	bool indexToName(uint32 index, std::string& name) {
		std::map<uint32, std::string>::iterator iter;
		iter = indexedNames.find(index);
		if (iter == indexedNames.end()) return false;
		name = iter->second;
		return true;
	}

	void storeIndexedName(const std::string& name) {
		uint32 index;
		bool hasIndex = extractIndexFromName(name, index);
		if (hasIndex) {
			indexedNames[index] = name;
			if (index >= indexCount) indexCount = index+1;
		}
	}

	std::map<uint32, std::string> indexedNames;
	uint32 indexCount;
};


#endif
