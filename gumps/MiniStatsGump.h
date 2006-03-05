/*
 *  Copyright (C) 2004-2005  The Pentagram Team
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

#ifndef MINISTATSGUMP_H
#define MINISTATSGUMP_H

#include "Gump.h"

class MiniStatsGump : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	MiniStatsGump();
	MiniStatsGump(int x, int y, uint32 _Flags = FLAG_DRAGGABLE,
				  sint32 layer = LAYER_NORMAL);
	virtual ~MiniStatsGump(void);

	// Init the gump, call after construction
	virtual void InitGump(Gump* newparent, bool take_focus=true);

	// Paint this Gump
	virtual void PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	// Trace a click, and return ObjId
	virtual uint16 TraceObjId(int mx, int my);

	virtual Gump* OnMouseDown(int button, int mx, int my);
	virtual void OnMouseDouble(int button, int mx, int my);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif
