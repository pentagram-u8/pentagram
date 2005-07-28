/*
 *  Copyright (C) 2003-2005 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "ShapeArchive.h"
#include "Shape.h"
#include "Palette.h"
#include "ConvertShape.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ShapeArchive,Pentagram::Archive);

ShapeArchive::~ShapeArchive()
{
	Archive::uncache();
}

Shape* ShapeArchive::getShape(uint32 shapenum)
{
	if (shapenum >= count) return 0;
	cache(shapenum);

	return shapes[shapenum];
}

void ShapeArchive::cache(uint32 shapenum)
{
	if (shapenum >= count) return;
	if (shapes.empty()) shapes.resize(count);

	if (shapes[shapenum]) return;

	uint32 shpsize;
	uint8 *data = getRawObject(shapenum, &shpsize);

	if (!data || shpsize == 0) return;

	// Auto detect format
	if (!format) {
		format = Shape::DetectShapeFormat(data,shpsize);
		if (format) pout << "Detected Shape Format: " << format->name << std::endl;
	}
	
	if (!format)
	{
		delete [] data;
		perr << "Error: Unable to detect shape format for flex." << std::endl;
		return;
	}

	Shape* shape = new Shape(data, shpsize, format, id, shapenum);
	if (palette) shape->setPalette(palette);

	shapes[shapenum] = shape;
}

void ShapeArchive::uncache(uint32 shapenum)
{
	if (shapenum >= count) return;
	if (shapes.empty()) return;

	delete shapes[shapenum];
	shapes[shapenum] = 0;
}

bool ShapeArchive::isCached(uint32 shapenum)
{
	if (shapenum >= count) return false;
	if (shapes.empty()) return false;

	return (shapes[shapenum] != 0);
}
