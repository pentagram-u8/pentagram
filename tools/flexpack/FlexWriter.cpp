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

#include "pent_include.h"

#include "FlexWriter.h"
#include "FlexFile.h"
#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(FlexWriter);

FlexWriter::FlexWriter(FlexFile * f)
{
	if (f)
	{
		uint32 i;
		uint32 count = f->getIndexCount();
		for (i = 0; i < count; ++i)
		{
			uint32 size;
			uint8* data = f->getObject(i, &size);
			if (!data) size = 0;
			FlexObject o;
			o.size = size;
			o.obj = data;
			objects.push_back(o);
		}
		delete f;
	}
}

FlexWriter::~FlexWriter()
{
	std::vector<FlexObject>::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		delete [] it->obj;
	}
	objects.clear();
}

void FlexWriter::add_object(const uint8* obj, uint32 size)
{
	FlexObject o;
	o.obj = new uint8[size];
	o.size = size;
	std::memcpy (o.obj, obj, size);
	objects.push_back(o);
}
 
void FlexWriter::add_datasource(IDataSource* ds)
{
	FlexObject o;
	o.size = ds->getSize();
	o.obj = new uint8[o.size];
	ds->seek(0);
	ds->read(o.obj, o.size);
	objects.push_back(o);
	delete ds;
}

void FlexWriter::set_object(uint32 index, const uint8* obj, uint32 size)
{
	if (index >= objects.size())
	{ // just add instead
		add_object(obj, size);
	}
	FlexObject * o = &objects.at(index);

	delete o->obj;
	o->obj = new uint8[size];
	o->size = size;
	std::memcpy (o->obj, obj, size);
}

void FlexWriter::set_datasource(uint32 index, IDataSource* ds)
{
	if (index >= objects.size())
	{ // just add instead
		add_datasource(ds);
	}
	FlexObject * o = &objects.at(index);

	delete o->obj;
	o->size = ds->getSize();
	o->obj = new uint8[o->size];
	ds->seek(0);
	ds->read(o->obj, o->size);
	delete ds;
}

void FlexWriter::write(ODataSource* ds)
{
	writeHead(ds);
	std::vector<FlexObject>::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		if (it->obj && it->size != 0)
			ds->write(it->obj, it->size);
	}	
}

void FlexWriter::writeHead(ODataSource* ds)
{
	uint32 i;
	ds->seek(0);
	for (i = 0; i < (0x50 / 4); i++)
	{
		ds->write4(0x1A1A1A1A);
	}
	ds->write4(0x00001A1A);
	ds->write4(objects.size());
	// FIXME! This is what many flexes have next, but not all. Find out why.
	ds->write4(0x00000001);
	// FIXME! Figure out what to write until 0x80.
	for (i = ds->getPos(); i < 0x80; i += 4)
	{
		ds->write4(0);
	}

	ds->seek(0x80);
	i = objects.size() * 8 + 0x80; // begining offset
	std::vector<FlexObject>::iterator it;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
		if (!it->obj || it->size == 0)
		{
			ds->write4(0);
		}
		else
		{
			ds->write4(i);
		}
		ds->write4(it->size);
		i += it->size;
	}
	
	//complete file size
	ds->seek(0x5c);
	ds->write4(i);

	i = objects.size() * 8 + 0x80; // begining offset
	ds->seek(i);
}
