/*
 *  Copyright (C) 2003-2006  The Pentagram Team
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

#ifndef ITEMRELATIVEGUMP_H_INCLUDED
#define ITEMRELATIVEGUMP_H_INCLUDED

#include "Gump.h"

class ItemRelativeGump : public Gump
{
protected:
	sint32 ix, iy;

public:
	ENABLE_RUNTIME_CLASSTYPE();

	ItemRelativeGump();
	ItemRelativeGump(sint32 x, sint32 y, sint32 width, sint32 height, uint16 owner, uint32 _Flags = 0, sint32 layer = LAYER_NORMAL);
	virtual ~ItemRelativeGump(void);

	virtual void		InitGump(Gump* newparent, bool take_focus=true);

	// Paint the Gump (RenderSurface is relative to parent).
	// Calls PaintThis and PaintChildren.
	virtual void		Paint(RenderSurface*, sint32 lerp_factor, bool scaled);

	virtual void		ParentToGump(int &px, int &py,
									 PointRoundDir r = ROUND_TOPLEFT);
	virtual void		GumpToParent(int &gx, int &gy,
									 PointRoundDir r = ROUND_TOPLEFT);

	virtual void		Move(int x, int y);

	bool				loadData(IDataSource* ids, uint32 version);
protected:
	virtual void 		saveData(ODataSource* ods);

	virtual void		GetItemLocation(sint32 lerp_factor);

	//! Move Gump so that it totally overlaps parent.
	void				MoveOnScreen();
};

#endif
