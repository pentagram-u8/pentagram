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

#include "pent_include.h"

#include "MapGlob.h"
#include "IDataSource.h"

MapGlob::MapGlob()
{

}


MapGlob::~MapGlob()
{
	contents.clear();
}

void MapGlob::read(IDataSource* ds)
{
	unsigned int itemcount = ds->read2();
	assert(ds->getSize() >= 2+itemcount*6);
	contents.clear();
	contents.resize(itemcount);

	for (unsigned int i = 0; i < itemcount; ++i) {
		GlobItem item;

		item.x = ds->read1();
		item.y = ds->read1();
		item.z = ds->read1();
		item.shape = ds->read2();
		item.frame = ds->read1();

		contents[i] = item;
	}
}
