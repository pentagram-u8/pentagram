/*
 *  Copyright (C) 2004  The Pentagram Team
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

#ifndef READABLEGUMP_H
#define READABLEGUMP_H

#include "ModalGump.h"
#include "intrinsics.h"
#include "RenderedText.h"

class ReadableGump : public ModalGump
{
	uint16 shapenum;
	int fontnum, tx, ty;
	std::string	text;
	RenderedText* rt;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ReadableGump(ObjId owner, uint16 shape, int font, std::string msg);
	virtual ~ReadableGump();

	// Got to the next page on mouse click
	virtual Gump* OnMouseDown(int button, int mx, int my);

	// Init the gump, call after construction
	virtual void InitGump();

	virtual void PaintThis(RenderSurface*, sint32 lerp_factor);

	INTRINSIC(I_readGrave);
	INTRINSIC(I_readPlaque);

public:
	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif
