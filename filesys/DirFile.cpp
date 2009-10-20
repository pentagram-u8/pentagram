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

#include "pent_include.h"

#include "filesys/DirFile.h"
#include "filesys/IDataSource.h"
#include "filesys/FileSystem.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(DirFile,NamedArchiveFile);

DirFile::DirFile(const std::string& path_)
{
	path = path_;
	if (path.size() == 0) {
		valid = false;
		return;
	}

	if (path[path.size()-1] != '/') path += "/";

	valid = readMetadata();
}


DirFile::~DirFile()
{

}

bool DirFile::readMetadata()
{
	FileSystem* filesys = FileSystem::get_instance();
	FileSystem::FileList files;

	/*int ret =*/ filesys->ListFiles(path + "*", files);

	// TODO: check if directory actually exists

	count = files.size();

	FileSystem::FileList::iterator iter;

	for (iter = files.begin(); iter != files.end(); ++iter) {
		std::string name = *iter;
		std::string::size_type pos = name.rfind("/");
		if (pos != std::string::npos) {
			name.erase(0, pos+1);
			pout << "DirFile: " << name << std::endl;
			storeIndexedName(name);
		}
	}

	return true;
}

bool DirFile::exists(const std::string& name)
{
	FileSystem* filesys = FileSystem::get_instance();
	IDataSource* ids = filesys->ReadFile(path + name);
	if (!ids) return false;

	delete ids;
	return true;
}

uint32 DirFile::getSize(const std::string& name)
{
	FileSystem* filesys = FileSystem::get_instance();
	IDataSource* ids = filesys->ReadFile(path + name);
	if (!ids) return 0;

	uint32 size = ids->getSize();
	delete ids;
	return size;
}

uint8* DirFile::getObject(const std::string& name, uint32* sizep)
{
	FileSystem* filesys = FileSystem::get_instance();
	IDataSource* ids = filesys->ReadFile(path + name);
	if (!ids) return 0;

	uint32 size = ids->getSize();
	if (size == 0) return 0;

	uint8* buf = new uint8[size];
	ids->read(buf, size);
	delete ids;

	if (sizep) *sizep = size;

	return buf;
}
