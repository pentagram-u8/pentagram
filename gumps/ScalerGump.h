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

#ifndef ScalerGump_H
#define ScalerGump_H

#include "DesktopGump.h"
#include <vector>
class RenderSurface;

class ScalerGump : public DesktopGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ScalerGump(sint32 x, sint32 y, sint32 width, sint32 height, sint32 swidth, sint32 sheight, sint32 scaler1, sint32 scaler2 = -1);
	virtual ~ScalerGump(void);

	virtual void Paint(RenderSurface* surf, sint32 lerp_factor);

	virtual void ParentToGump(int &px, int &py);
	virtual void GumpToParent(int &gx, int &gy);
	
	void GetScaledSize(sint32 &sw, sint32 &sh) const { sw = swidth; sh = sheight; }

protected:
	sint32	swidth;
	sint32	sheight;
	sint32	scaler1;
	sint32	scaler2;
	RenderSurface* buffer;
};

#endif
