/*
 *  Copyright (C) 2005  The Pentagram Team
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

#ifndef MINIMAPGUMP_H_INCLUDED
#define MiniMapGump_H_INCLUDED

#include "Gump.h"
#include "Texture.h"
#include "CurrentMap.h"

#define MINMAPGUMP_SCALE 8

class MiniMapGump : public Gump
{
	Texture				minimap;
	unsigned int		lastMapNum;
	uint32				texbuffer[MAP_NUM_CHUNKS*MINMAPGUMP_SCALE][MAP_NUM_CHUNKS*MINMAPGUMP_SCALE];

	uint32				sampleAtPoint(int x, int y, CurrentMap *map);

public:
	ENABLE_RUNTIME_CLASSTYPE();

	MiniMapGump(void);
	MiniMapGump(int x, int y);
	virtual ~MiniMapGump(void);

	virtual void		PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled);
	virtual uint16		TraceObjId(int mx, int my);

	static void			ConCmd_toggle(const Console::ArgvType &argv);	//!< "MiniMapGump::toggle" console command
	static void			ConCmd_generateWholeMap(const Console::ArgvType &argv);	//!< "MiniMapGump::generateWholeMap" console command

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif // MINIMAPGUMP_H_INCLUDED
