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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PAPERDOLLGUMP_H
#define PAPERDOLLGUMP_H

#include "ContainerGump.h"

class PaperdollGump : public ContainerGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	PaperdollGump();
	PaperdollGump(Shape* shape, uint32 framenum, uint16 owner,
				  uint32 _Flags = 0, sint32 layer = LAYER_NORMAL);
	virtual ~PaperdollGump(void);

	// Init the gump, call after construction
	virtual void InitGump();

	// Paint this Gump
	virtual void PaintThis(RenderSurface*, sint32 lerp_factor);

	// Trace a click, and return ObjId
	virtual uint16 TraceObjId(int mx, int my);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure.
	virtual bool GetLocationOfItem(uint16 itemid, int &gx, int &gy,
								   sint32 lerp_factor = 256);

	virtual bool StartDraggingItem(Item* item, int mx, int my);
	virtual bool DraggingItem(Item* item, int mx, int my);
#if 0
	virtual void DraggingItemLeftGump(Item* item);
	virtual void StopDraggingItem(Item* item, bool moved);
#endif
	virtual void DropItem(Item* item, int mx, int my);

#if 0
	virtual Gump* OnMouseDown(int button, int mx, int my);
	virtual void OnMouseClick(int button, int mx, int my);
	virtual void OnMouseDouble(int button, int mx, int my);
#endif
	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif
