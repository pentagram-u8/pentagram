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

#ifndef SHAPEARCHIVE_H
#define SHAPEARCHIVE_H

#include "filesys/Archive.h"

class Shape;
struct ConvertShapeFormat;
namespace Pentagram { struct Palette; }

class ShapeArchive : public Pentagram::Archive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ShapeArchive(uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: Archive(), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(ArchiveFile* af, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: Archive(af), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(IDataSource* ds, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: Archive(ds), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(const std::string& path, uint16 id_,
				 Pentagram::Palette* pal_ = 0,
				 const ConvertShapeFormat *format_ = 0)
		: Archive(path), id(id_), format(format_), palette(pal_) { }

	virtual ~ShapeArchive();

	Shape* getShape(uint32 shapenum);

	virtual void cache(uint32 shapenum);
	virtual void uncache(uint32 shapenum);
	virtual bool isCached(uint32 shapenum);

protected:
	uint16 id;
	const ConvertShapeFormat *format;
	Pentagram::Palette* palette;
	std::vector<Shape*> shapes;
};


#endif
