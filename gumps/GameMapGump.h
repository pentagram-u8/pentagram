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

#ifndef GAMEMAPGUMP_H_INCLUDED
#define GAMEMAPGUMP_H_INCLUDED

#include "Gump.h"
#include <vector>

class ItemSorter;
class CameraProcess;

class GameMapGump : public Gump
{
protected:
	ItemSorter		*display_list;

	std::vector<uint16>			fastAreas[2];
	int							fastArea;	// 0 or 1

public:
	ENABLE_RUNTIME_CLASSTYPE();

	GameMapGump(int x, int y, int w, int h);
	virtual ~GameMapGump();

	virtual bool		Run(const uint32 framenum);

	virtual void		MapChanged();

	virtual void		PaintThis(RenderSurface *surf, sint32 lerp_factor);

	// Trace a click, and return ObjID
	virtual uint16		TraceObjID(int mx, int my);

	// Get the location of an item in the gump (coords reletive to this).
	// Returns false on failure
	virtual bool		GetLocationOfItem(uint16 itemid, int &gx, int &gy,
										  sint32 lerp_factor = 256);

	virtual Gump *		OnMouseDown(int button, int mx, int my);
	virtual void		OnMouseClick(int button, int mx, int my);
	virtual void		OnMouseDouble(int button, int mx, int my);

	void IncSortOrder(int count);
};

#endif //GameMapGUMP_H_INCLUDED

