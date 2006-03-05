/*
Copyright (C) 2004-2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef RENDEREDTEXT_H
#define RENDEREDTEXT_H

class RenderSurface;

class RenderedText
{
public:
	RenderedText();
	virtual ~RenderedText();

	//! Draw self to a rendersurface.
	//! \param surface The surface to draw to
	//! \param x X coordinate of target
	//! \param y Y coordinate of target. This will be the top baseline.
	virtual void draw(RenderSurface* surface, int x, int y, bool destmasked = false)=0;

	//! Draw self to a rendersurface blended (0xAABBGGRR, alpha is blend level)
	virtual void drawBlended(RenderSurface* surface, int x, int y, uint32 col, bool destmasked = false)=0;

	//! Get dimensions.
	//! \param x Returns the width
	//! \param y Returns the height
	virtual void getSize(int &x, int &y) { x = width; y = height; }

	//! Get vlead
	virtual int getVlead() { return vlead; }

	ENABLE_RUNTIME_CLASSTYPE();

protected:
	int width, height;
	int vlead;
};

#endif
