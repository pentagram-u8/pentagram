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

#ifndef MAINSHAPEARCHIVE_H
#define MAINSHAPEARCHIVE_H

#include "ShapeArchive.h"

class TypeFlags;
class ShapeInfo;
class AnimDat;
class ActorAnim;
struct AnimAction;

class MainShapeArchive : public ShapeArchive
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	MainShapeArchive(uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(id_, pal_, format_), typeFlags(0), animdat(0) { }
	MainShapeArchive(ArchiveFile* af, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(af, id_, pal_, format_), typeFlags(0), animdat(0) { }
	MainShapeArchive(IDataSource* ds, uint16 id_, Pentagram::Palette* pal_ = 0,
			  const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(ds, id_, pal_, format_), typeFlags(0), animdat(0) { }
	MainShapeArchive(const std::string& path, uint16 id_,
				 Pentagram::Palette* pal_ = 0,
				 const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(path, id_, pal_, format_), typeFlags(0), animdat(0) { }

	virtual ~MainShapeArchive();
	
	void loadTypeFlags(IDataSource *ds);
	ShapeInfo* getShapeInfo(uint32 shapenum);

	void loadAnimDat(IDataSource *ds);
	ActorAnim* getAnim(uint32 shape) const;
	AnimAction* getAnim(uint32 shape, uint32 action) const;
	
protected:
	TypeFlags* typeFlags;
	AnimDat* animdat;
};


#endif
