/*
Copyright (C) 2004 The Pentagram team

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

#ifndef FLEXWRITER_H
#define FLEXWRITER_H

#include <vector>

class IDataSource;
class ODataSource;
class FlexFile;

class FlexWriter {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	explicit FlexWriter(FlexFile * f = 0);
	virtual ~FlexWriter();

	virtual void add_object(const uint8* obj, uint32 size);
	virtual void add_datasource(IDataSource* ds);

	virtual void set_object(uint32 index, const uint8* obj, uint32 size);
	virtual void set_datasource(uint32 index, IDataSource* ds);

	virtual void write(ODataSource* ds);
protected:
	virtual void writeHead(ODataSource* ds);

	struct FlexObject {
		uint8 * obj;
		uint32 size;
	};
	std::vector<FlexObject> objects;
};

#endif
