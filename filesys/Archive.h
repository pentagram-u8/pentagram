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

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <vector>

class ArchiveFile;
class IDataSource;

namespace Pentagram {

class Archive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! create Archive without any input sources
	Archive();

	//! create Archive with a single input source
	explicit Archive(ArchiveFile* af);

	//! create Archive with a single input source, autodetecting the type
	//! Will create FlexFile, U8SaveFile or ZipFile; ids will be deleted.
	explicit Archive(IDataSource* ids);

	//! create Archive with a single directory input source
	//! Will create DirFile.
	explicit Archive(const std::string& path);

	virtual ~Archive();

	//! add input source.
	//! ArchiveFile will be deleted on destruction
	//! Input sources are used in the reversed order they are added.
	//! Effect of adding sources after having accessed objects is undef.
	bool addSource(ArchiveFile* af);

	//! add input source, autodetecting the type (as the constructor)
	bool addSource(IDataSource* ids);

	//! add directory input source (as the constructor)
	bool addSource(const std::string& path);


	//! Cache all objects
	void cache();

	//! Cache a single object
	virtual void cache(uint32 index)=0;

	//! Uncache all objects
	//! Potentially dangerous: all stored objects will be deleted; make sure
	//!  they are no longer in use.
	void uncache();

	//! Uncache a single object
	//! Potentially dangerous. See uncache()
	virtual void uncache(uint32 index)=0;

	//! Check if an object is cached
	virtual bool isCached(uint32 index)=0;

	uint32 getCount() const { return count; }

protected:
	uint32 count;

	uint8* getRawObject(uint32 index, uint32* sizep=0);
	uint32 getRawSize(uint32 index);

private:
	std::vector<ArchiveFile*> sources;

	ArchiveFile* findArchiveFile(uint32 index);
};

}

#endif
