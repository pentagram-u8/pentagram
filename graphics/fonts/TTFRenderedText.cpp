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

#include "pent_include.h"

#ifdef USE_SDLTTF

#include "TTFRenderedText.h"
#include "TTFont.h"
#include "RenderSurface.h"
#include "Texture.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFRenderedText,RenderedText);


TTFRenderedText::TTFRenderedText(Texture* texture_,
								 int width_, int height_, TTFont* font_)
	: texture(texture_), font(font_)
{
	width = width_;
	height = height_;
}

TTFRenderedText::~TTFRenderedText()
{
	delete texture;
}

void TTFRenderedText::draw(RenderSurface* surface, int x, int y)
{
	surface->Blit(texture, 0, 0, width, height, x, y-font->getBaseline());
//	surface->FadedBlit(texture, 0, 0, width, height, x, y-font->getBaseline(), 0x800000F0);
}

#endif
