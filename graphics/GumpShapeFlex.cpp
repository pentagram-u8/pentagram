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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "GumpShapeFlex.h"
#include "Rect.h"
#include "IDataSource.h"

GumpShapeFlex::GumpShapeFlex(IDataSource* ds, const uint16 id, Pentagram::Palette* pal,
							 const ConvertShapeFormat *format)
	: ShapeFlex(ds, id, pal, format)
{

}


GumpShapeFlex::~GumpShapeFlex()
{
	for (unsigned int i = 0; i < gumpItemArea.size(); ++i)
		delete gumpItemArea[i];
	gumpItemArea.clear();
}

void GumpShapeFlex::loadGumpage(IDataSource* ds)
{
	unsigned int count = ds->getSize() / 8;
	gumpItemArea.resize(count+1);
	for (unsigned int i = 1; i <= count; ++i)
	{
		int x, y, w, h;
		x = ds->read2();
		y = ds->read2();
		w = ds->read2();
		h = ds->read2();
		gumpItemArea[i] = new Pentagram::Rect(x, y, w, h);
	}
}

Pentagram::Rect* GumpShapeFlex::getGumpItemArea(uint32 shapenum)
{
	if (shapenum >= gumpItemArea.size()) return 0;
	return gumpItemArea[shapenum];
}
