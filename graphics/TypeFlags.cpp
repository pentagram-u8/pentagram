/*
 *  Copyright (C) 2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "TypeFlags.h"
#include "IDataSource.h"

TypeFlags::TypeFlags()
{

}


TypeFlags::~TypeFlags()
{

}

ShapeInfo* TypeFlags::getShapeInfo(uint32 shapenum)
{
	if (shapenum < shapeInfo.size())
		return &(shapeInfo[shapenum]);
	else
		return 0;
}


void TypeFlags::load(IDataSource *ds)
{
	// TODO: detect U8/crusader format somehow?!
	// (Or probably pass it as parameter)
	// The 'parsing' below is only for U8

	uint32 size = ds->getSize();
	uint32 count = size / 8;

	shapeInfo.clear();
	shapeInfo.resize(count);

	for (uint32 i = 0; i < count; ++i)
	{
		uint8 data[8];
		ds->read(data, 8);

		ShapeInfo si;

		si.flags = 0;
		if (data[0] & 0x01) si.flags |= ShapeInfo::SI_FIXED;
		if (data[0] & 0x02) si.flags |= ShapeInfo::SI_SOLID;
		if (data[0] & 0x04) si.flags |= ShapeInfo::SI_SEA;
		if (data[0] & 0x08) si.flags |= ShapeInfo::SI_LAND;
		if (data[0] & 0x10) si.flags |= ShapeInfo::SI_OCCL;
		if (data[0] & 0x20) si.flags |= ShapeInfo::SI_BAG;
		if (data[0] & 0x40) si.flags |= ShapeInfo::SI_DAMAGING;
		if (data[0] & 0x80) si.flags |= ShapeInfo::SI_NOISY;

		if (data[1] & 0x01) si.flags |= ShapeInfo::SI_DRAW;
		if (data[1] & 0x02) si.flags |= ShapeInfo::SI_IGNORE;
		if (data[1] & 0x04) si.flags |= ShapeInfo::SI_ROOF;
		if (data[1] & 0x08) si.flags |= ShapeInfo::SI_TRANSL;
		si.family = data[1] >> 4;

		si.equiptype = data[2] & 0x0F;
		si.x = data[2] >> 4;

		si.y = data[3] & 0x0F;
		si.z = data[3] >> 4;

		si.animtype = data[4] & 0x0F;
		si.animdata = data[4] >> 4;

		if (data[5] & 0x10) si.flags |= ShapeInfo::SI_EDITOR;
		if (data[5] & 0x20) si.flags |= ShapeInfo::SI_EXPLODE;

		si.weight = data[6];

		si.volume = data[7];
		
		shapeInfo[i] = si;
	}
}
