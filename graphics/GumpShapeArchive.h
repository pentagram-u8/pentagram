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

#ifndef GUMPSHAPEARCHIVE_H
#define GUMPSHAPEARCHIVE_H

#include "ShapeArchive.h"

namespace Pentagram { struct Rect; }

class GumpShapeArchive : public ShapeArchive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	GumpShapeArchive(uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(id_, pal_, format_) { }
	GumpShapeArchive(ArchiveFile* af, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(af, id_, pal_, format_) { }
	GumpShapeArchive(IDataSource* ds, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(ds, id_, pal_, format_) { }
	GumpShapeArchive(const std::string& path, uint16 id_,
				 Pentagram::Palette* pal_ = 0,
				 const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(path, id_, pal_, format_) { }

	virtual ~GumpShapeArchive();
	
	void loadGumpage(IDataSource *ds);
	Pentagram::Rect* getGumpItemArea(uint32 shapenum);
	
protected:
	std::vector<Pentagram::Rect*> gumpItemArea;
};


#endif
