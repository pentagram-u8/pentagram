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

#ifndef SHAPERENDEREDTEXT_H
#define SHAPERENDEREDTEXT_H

#ifdef USE_SDLTTF

#include "RenderedText.h"
#include "Font.h"

class TTFont;
struct Texture;

class TTFRenderedText : public RenderedText
{
public:
	TTFRenderedText(Texture* texture, int width, int height, TTFont* font);
	virtual ~TTFRenderedText();

	virtual void draw(RenderSurface* surface, int x, int y);

	ENABLE_RUNTIME_CLASSTYPE();

protected:
	Texture* texture;
	TTFont* font;
};

#endif

#endif
