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

#ifndef U8SAVE_H
#define U8SAVE_H

#include "Flex.h"
#include <string>
#include <vector>
#include <map>

class IDataSource;

class U8Save : public Flex
{
public:
	explicit U8Save(IDataSource* ds);
	virtual ~U8Save();

	virtual uint32 get_size(uint32 index);

	// return object as IDataSource. Delete the IDataSource afterwards,
	// but DON'T delete/modify the buffer it points to.
	virtual IDataSource* get_datasource(const char* filename);
	virtual IDataSource* get_datasource(uint32 index) { return Flex::get_datasource(index); };

protected:
	virtual uint32 get_offset(uint32 index);

	std::vector<uint32> offsets;
	std::vector<uint32> sizes;
	std::vector<std::string> names;
	std::map<std::string, uint32> indices;
};

#endif
