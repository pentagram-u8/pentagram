/*
SoftRenderSurface.cpp : SoftRenderSurface Implementation source file

Copyright (C) 2002, 2003 The Pentagram Team

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
#include <SDL.h>

#include "SoftRenderSurface.h"
#include "Texture.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "Palette.h"


///////////////////////
//                   //
// SoftRenderSurface //
//                   //
///////////////////////


//
// SoftRenderSurface::SoftRenderSurface(SDL_Surface *s)
//
// Desc: Create a SoftRenderSurface from a SDL_Surface
//
template<class uintX> SoftRenderSurface<uintX>::SoftRenderSurface(SDL_Surface *s)
	: BaseSoftRenderSurface(s)
{
}


//
// SoftRenderSurface::Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h)
//
// Desc: Fill buffer (using a palette index) - Remove????
//
template<class uintX> void SoftRenderSurface<uintX>::Fill8(uint8 /*index*/, sint32 /*sx*/, sint32 /*sy*/, sint32 /*w*/, sint32 /*h*/)
{
}


//
// SoftRenderSurface::Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h)
//
// Desc: Fill buffer (using a RGB colour)
//
template<class uintX> void SoftRenderSurface<uintX>::Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h)
{
	uint8 *pixel = pixels + sy * pitch + sx * sizeof(uintX);
	uint8 *line_end = pixel + w*sizeof(uintX);
	uint8 *end = pixel + h * pitch;
	int diff = pitch - w*sizeof(uintX);

	rgb = PACK_RGB8( (rgb>>16)&0xFF , (rgb>>8)&0xFF , rgb&0xFF );

	while (pixel != end)
	{
		while (pixel != line_end)
		{
			*(reinterpret_cast<uintX*>(pixel)) = rgb;
			pixel+=sizeof(uintX);
		}

		line_end += pitch;
		pixel += diff;
	}
}


//
// SoftRenderSurface::Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::Blit(Texture *tex, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy)
{
	// Clip dx
	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	// Clip w
	if (w > static_cast<sint32>(tex->width)) w = tex->width;
	if ((dx + w) > static_cast<sint32>(width)) w = width - dx;
	
	// Clip dy
	if (dy < 0) {
		sy -= dy;
		h += dy;
		dy = 0;
	}

	// Clip h
	if (h > static_cast<sint32>(tex->height)) w = tex->height;
	if ((dy + h) > static_cast<sint32>(height)) h = height - dy;
	

	uint8 *pixel = pixels + dy * pitch + dx * sizeof(uintX);
	uint8 *line_end = pixel + w*sizeof(uintX);
	uint8 *end = pixel + h * pitch;
	int diff = pitch - w*sizeof(uintX);

	uint32 *texel = tex->buffer + (sy * tex->width + sx);
	int tex_diff = tex->width - w;

	//b = PACK_RGB8( (rgb>>16)&0xFF , (rgb>>8)&0xFF , rgb&0xFF );

	while (pixel != end)
	{
		while (pixel != line_end)
		{
			if (*texel & TEX32_A_MASK)
			{
				*(reinterpret_cast<uintX*>(pixel)) = PACK_RGB8( TEX32_R(*texel), TEX32_G(*texel), TEX32_B(*texel) );
			}
			pixel+=sizeof(uintX);
			texel++;
		}

		line_end += pitch;
		pixel += diff;
		texel+= tex_diff;
	}


}


//
// SoftRenderSurface::PrintCharFixed(Texture *, char character, int x, int y)
//
// Desc: Draw a fixed width character from a Texture buffer
//
template<class uintX> void SoftRenderSurface<uintX>::PrintCharFixed(Texture *texture, int character, int x, int y)
{
	int char_width = texture->width/16;
	int char_height = texture->height/16;

	if (char_width == 16 && char_height == 16)
	{
		SoftRenderSurface::Blit(texture, (character&0x0F) << 4, character&0xF0, 16, 16, x, y);
	}
	else if (char_width == 8 && char_height == 8)
	{
		SoftRenderSurface::Blit(texture, (character&0x0F) << 3, (character>>1)&0x78, 8, 8, x, y);
	}
	else
	{
		SoftRenderSurface::Blit(texture,  (character&0x0F) * char_width, (character&0xF0>>4) * char_height, char_width, char_height, x, y);
	}
}


//
// SoftRenderSurface::PrintTextFixed(Texture *, const char *text, int x, int y)
//
// Desc: Draw fixed width from a Texture buffer (16x16 characters fixed width and height)
//
template<class uintX> void SoftRenderSurface<uintX>::PrintTextFixed(Texture *texture, const char *text, int x, int y)
{
	int char_width = texture->width/16;
	int char_height = texture->height/16;

	int character;
	if (char_width == 16 && char_height == 16) while (character = *text)
	{
		SoftRenderSurface::Blit(texture, (character&0x0F) << 4, character&0xF0, 16, 16, x, y);
		++text;
		x+=16;
	}
	else if (char_width == 8 && char_height == 8) while (character = *text)
	{
		SoftRenderSurface::Blit(texture, (character&0x0F) << 3, (character>>1)&0x78, 8, 8, x, y);
		++text;
		x+=8;
	}
	else while (character = *text)
	{
		SoftRenderSurface::Blit(texture,  (character&0x0F) * char_width, (character&0xF0>>4) * char_height, char_width, char_height, x, y);
		++text;
		x+=char_width;
	}
}


//
// void SoftRenderSurface::Paint(Shape*s, uint32 framenum, sint32 x, sint32 y)
//
// Desc: Standard shape drawing functions. Clips but doesn't do anything else
//
template<class uintX> void SoftRenderSurface<uintX>::Paint(Shape*s, uint32 framenum, sint32 x, sint32 y)
{
	#include "SoftRenderSurface.inl"
}


//
// void SoftRenderSurface::PaintNoClip(Shape*s, uint32 framenum, sint32 x, sint32 y)
//
// Desc: Standard shape drawing functions. Doesn't clips
//
template<class uintX> void SoftRenderSurface<uintX>::PaintNoClip(Shape*s, uint32 framenum, sint32 x, sint32 y)
{
#define NO_CLIPPING
	#include "SoftRenderSurface.inl"
#undef NO_CLIPPING
}


//
// void SoftRenderSurface::PaintTranslucent(Shape*s, uint32 framenum, sint32 x, sint32 y)
//
// Desc: Standard shape drawing functions. Clips and XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintTranslucent(Shape* s, uint32 framenum, sint32 x, sint32 y)
{
#define XFORM_SHAPES
	#include "SoftRenderSurface.inl"
#undef XFORM_SHAPES
}


//
// void SoftRenderSurface::PaintMirrored(Shape*s, uint32 framenum, sint32 x, sint32 y, bool trans)
//
// Desc: Standard shape drawing functions. Clips, Flips and conditionally XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintMirrored(Shape* s, uint32 framenum, sint32 x, sint32 y, bool trans)
{
#define FLIP_SHAPES
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans

	#include "SoftRenderSurface.inl"

#undef FLIP_SHAPES
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
}


//
// Instantiate the SoftRenderSurface Class
//
template class SoftRenderSurface<uint16>;
template class SoftRenderSurface<uint32>;
