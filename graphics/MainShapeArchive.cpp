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

#include "MainShapeArchive.h"
#include "TypeFlags.h"
#include "ShapeInfo.h"
#include "AnimDat.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MainShapeArchive,ShapeArchive);


MainShapeArchive::~MainShapeArchive()
{
	if (typeFlags) {
		delete typeFlags;
		typeFlags = 0;
	}

	if (animdat) {
		delete animdat;
		animdat = 0;
	}
}
	
void MainShapeArchive::loadTypeFlags(IDataSource *ds)
{
	if (typeFlags) {
		delete typeFlags;
		typeFlags = 0;
	}

	typeFlags = new TypeFlags;
	typeFlags->load(ds);
}

ShapeInfo* MainShapeArchive::getShapeInfo(uint32 shapenum)
{
	assert(typeFlags);

	return typeFlags->getShapeInfo(shapenum);
}

void MainShapeArchive::loadAnimDat(IDataSource *ds)
{
	if (animdat) {
		delete animdat;
		animdat = 0;
	}

	animdat = new AnimDat;
	animdat->load(ds);
}

ActorAnim* MainShapeArchive::getAnim(uint32 shape) const
{
	assert(animdat);

	return animdat->getAnim(shape);
}

AnimAction* MainShapeArchive::getAnim(uint32 shape, uint32 action) const
{
	assert(animdat);

	return animdat->getAnim(shape, action);
}
