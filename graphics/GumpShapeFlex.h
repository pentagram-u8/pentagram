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

#ifndef GUMPSHAPEFLEX_H
#define GUMPSHAPEFLEX_H

#include "ShapeFlex.h"

struct Rect;

class GumpShapeFlex : public ShapeFlex
{
public:
	GumpShapeFlex(IDataSource* ds, Palette* pal = 0,
				  const ConvertShapeFormat *format = 0);
	virtual ~GumpShapeFlex();
	
	void loadGumpage(IDataSource *ds);
	Rect* getGumpItemArea(uint32 shapenum);
	
protected:
	std::vector<Rect*> gumpItemArea;
};


#endif
