/*
Copyright (C) 2003-2004 The Pentagram team

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

#ifndef SHAPEFLEX_H
#define SHAPEFLEX_H

#include <vector>

#include "Flex.h"

class IDataSource;
class Shape;
struct ConvertShapeFormat;
namespace Pentagram { struct Palette; }

class ShapeFlex : protected Flex
{
public:
	ShapeFlex(IDataSource* ds, Pentagram::Palette* pal = 0,
			  const ConvertShapeFormat *format = 0);
	virtual ~ShapeFlex();

	Shape* getShape(uint32 shapenum);

	virtual uint32 get_count() { return Flex::get_count(); }	

	void cache(); // cache all shapes
	void cache(uint32 shapenum); // cache one shape
	inline void uncache() { Flex::uncache(); }; // uncache all shapes
	void uncache(uint32 shapenum); // uncache one shape

protected:
	const ConvertShapeFormat *format;
	Pentagram::Palette* palette;
	std::vector<Shape*> shapes;
};


#endif
