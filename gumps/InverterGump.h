/*
 *  Copyright (C) 2004-2006  The Pentagram Team
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

#ifndef INVERTERGUMP_H
#define INVERTERGUMP_H

#include "DesktopGump.h"
#include <vector>
class RenderSurface;

class InverterGump : public DesktopGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	InverterGump(sint32 x, sint32 y, sint32 width, sint32 height);
	virtual ~InverterGump(void);

	virtual void Paint(RenderSurface* surf, sint32 lerp_factor, bool scaled);
	virtual void PaintChildren(RenderSurface* surf, sint32 lerp_factor, bool scaled);

	virtual void ParentToGump(int &px, int &py,
							  PointRoundDir r = ROUND_TOPLEFT);
	virtual void GumpToParent(int &gx, int &gy,
							  PointRoundDir r = ROUND_TOPLEFT);

	virtual void RenderSurfaceChanged();

protected:
	RenderSurface* buffer;
};

#endif
