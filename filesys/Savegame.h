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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include <string>

class ZipFile;
class IDataSource;

class Savegame
{
public:
	explicit Savegame(IDataSource* ds);
	virtual ~Savegame();

	//! get the savegame's global version
	uint32 getVersion();

	//! get the savegame's description
	std::string getDescription();

	IDataSource* getDataSource(const std::string& name);
protected:
	ZipFile* zipfile;
};

#endif
