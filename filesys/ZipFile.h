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

#ifndef ZIPFILE_H
#define ZIPFILE_H

#include <map>

#include "filesys/NamedArchiveFile.h"

class IDataSource;

class ZipFile : public NamedArchiveFile {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	//! create ZipFile from datasource; ZipFile takes ownership of ds
	//! and deletes it when destructed
	explicit ZipFile(IDataSource* ds);
	virtual ~ZipFile();

	virtual bool exists(const std::string& name);

	virtual uint8* getObject(const std::string& name, uint32* size=0);

	virtual uint32 getSize(const std::string& name);

	virtual uint32 getCount() { return count; }

	static bool isZipFile(IDataSource* ds);

	std::string getComment() { return globalComment; }

protected:
	bool readMetadata();


	IDataSource* ds;
	uint32 count;
	std::map<std::string, uint32> sizes;

	std::string globalComment;

	// unzip internals
	void* unzipfile;
};


#endif
