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

#ifndef RAWARCHIVE_H
#define RAWARCHIVE_H

#include "filesys/Archive.h"

class ArchiveFile;
class IDataSource;

class RawArchive : public Pentagram::Archive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	RawArchive() : Archive() { }
	explicit RawArchive(ArchiveFile* af) : Archive(af) { }
	explicit RawArchive(IDataSource* ids) : Archive(ids) { }
	explicit RawArchive(const std::string& path) : Archive(path) { }

	virtual ~RawArchive();

	virtual void cache(uint32 index);
	virtual void uncache(uint32 index);
	virtual bool isCached(uint32 index);

	//! return object. DON'T delete or modify!
	virtual const uint8* get_object_nodel(uint32 index);

	//! return object. delete afterwards. This will not cache the object
	virtual uint8* get_object(uint32 index);

	//! get size of object
	virtual uint32 get_size(uint32 index);

	//! return object as IDataSource. Delete the IDataSource afterwards,
	//! but DON'T delete/modify the buffer it points to.
	virtual IDataSource* get_datasource(uint32 index);	

protected:
	std::vector<uint8*> objects;
};

#endif
