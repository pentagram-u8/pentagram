/*
Copyright (C) 2004 The Pentagram team

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

	virtual void draw(RenderSurface* surface, int x, int y)=0;
	virtual void getSize(int &x, int &y) { x = width; y = height; }

	ENABLE_RUNTIME_CLASSTYPE();

protected:
	int width, height;
};

#endif
