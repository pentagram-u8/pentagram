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

#include "pent_include.h"
#include "ScalerGump.h"

#include "RenderSurface.h"
#include "Texture.h"
#include "GUIApp.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ScalerGump,DesktopGump);

ScalerGump::ScalerGump(sint32 _x, sint32 _y, sint32 _width, sint32 _height, sint32 _swidth, sint32 _sheight, sint32 _scaler1, sint32 _scaler2)
	: DesktopGump(_x, _y, _width, _height), swidth(_swidth), sheight(_sheight), scaler1(_scaler1), scaler2(_scaler2)
{
	buffer = 0;
}

ScalerGump::~ScalerGump()
{
	delete buffer;
}

static inline int getLine(int index, int n)
{
        index = index % (2*n);

        if (index >= n)
                return 2*n - 1 - 2*(index - n);
        else
                return 2*index;
}

static inline int getIndex(int line, int n)
{
        if (line % 2 == 0)
                return line / 2;
        else
                return 2*n - 1 - (line/2);
}

void ScalerGump::Paint(RenderSurface* surf, sint32 lerp_factor)
{
	// Skip the clipping rect/origin setting, since they will already be set
	// correctly by our parent.
	// (Or maybe I'm just to lazy to figure out the correct coordinates
	//  to use to compensate for the flipping... -wjp :-) )

	// Don't paint if hidden
	if (IsHidden()) return;

	int width = dims.w, height = dims.h;


	if (swidth == width && sheight == height)
	{
		PaintChildren(surf, lerp_factor);
		return;
	}

	// need a backbuffer
	if (!buffer) {
		buffer = RenderSurface::CreateSecondaryRenderSurface(swidth, sheight);
	}

	// Paint children
	PaintChildren(buffer, lerp_factor);

	Texture* tex = buffer->GetSurfaceAsTexture();
	surf->StretchBlit(tex, 0, 0, swidth, sheight, 0, 0, width, height);
}

// Convert a parent relative point to a gump point
void ScalerGump::ParentToGump(int &px, int &py)
{
	px -= x;
	px *= swidth;
	px /= dims.w;

	py -= y;
	py *= sheight;
	py /= dims.h;
}

// Convert a gump point to parent relative point
void ScalerGump::GumpToParent(int &gx, int &gy)
{
	gx *= dims.w;
	gx /= swidth;
	gx += x;

	gy *= dims.h;
	gy /= sheight;
	gy += y;
}
