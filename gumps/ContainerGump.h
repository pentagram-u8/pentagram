/*
 *  Copyright (C) 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CONTAINERGUMP_H
#define CONTAINERGUMP_H

#include "ItemRelativeGump.h"

class Shape;

class ContainerGump : public ItemRelativeGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ContainerGump(Shape* shape, uint32 framenum, uint16 owner,
				  uint32 _Flags = 0, sint32 layer = LAYER_NORMAL);
	virtual ~ContainerGump(void);

	// Init the gump, call after construction
	virtual void InitGump();

	// Paint the Gump (RenderSurface is relative to parent).
	// Calls PaintThis and PaintChildren
	virtual void Paint(RenderSurface*, sint32 lerp_factor);

	// Trace a click, and return ObjID
	virtual uint16 TraceObjID(int mx, int my);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
//	virtual bool GetLocationOfItem(uint16 itemid, int &gx, int &gy,
//								   sint32 lerp_factor = 256);

};

#endif
