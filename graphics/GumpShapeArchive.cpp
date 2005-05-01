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

#include "GumpShapeArchive.h"
#include "Rect.h"
#include "IDataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(GumpShapeArchive,ShapeArchive);

GumpShapeArchive::~GumpShapeArchive()
{
	for (unsigned int i = 0; i < gumpItemArea.size(); ++i)
		delete gumpItemArea[i];
	gumpItemArea.clear();
}

void GumpShapeArchive::loadGumpage(IDataSource* ds)
{
	unsigned int count = ds->getSize() / 8;
	gumpItemArea.resize(count+1);
	for (unsigned int i = 1; i <= count; ++i)
	{
		int x, y, w, h;
		x = static_cast<sint16>(ds->read2());
		y = static_cast<sint16>(ds->read2());
		w = static_cast<sint16>(ds->read2()) - x;
		h = static_cast<sint16>(ds->read2()) - y;
		gumpItemArea[i] = new Pentagram::Rect(x, y, w, h);
	}
}

Pentagram::Rect* GumpShapeArchive::getGumpItemArea(uint32 shapenum)
{
	if (shapenum >= gumpItemArea.size()) return 0;
	return gumpItemArea[shapenum];
}
