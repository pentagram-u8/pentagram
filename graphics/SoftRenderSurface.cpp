/*
SoftRenderSurface.cpp : SoftRenderSurface Implementation source file

Copyright (C) 2002 The Pentagram Team

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
#include "SoftRenderSurface.h"
#include "Texture.h"
#include <SDL.h>


///////////////////////
//                   //
// SoftRenderSurface //
//                   //
///////////////////////

uint8	SoftRenderSurface::r_loss,   SoftRenderSurface::g_loss,   SoftRenderSurface::b_loss;
uint8	SoftRenderSurface::r_loss16, SoftRenderSurface::g_loss16, SoftRenderSurface::b_loss16;
uint8	SoftRenderSurface::r_shift,  SoftRenderSurface::g_shift,  SoftRenderSurface::b_shift;
uint32	SoftRenderSurface::r_mask,   SoftRenderSurface::g_mask,   SoftRenderSurface::b_mask;


//
// SoftRenderSurface::SoftRenderSurface(SDL_Surface *s)
//
// Desc: Constructor for SoftRenderSurface from a SDL_Surface
//
SoftRenderSurface::SoftRenderSurface(SDL_Surface *s) :
	pixels(0), pixels00(0), zbuffer(0), zbuffer00(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0), 
	w_real(0), h_real(0), width(0), height(0), pitch(0), zpitch(0),
	clip_window(0,0,0,0), lock_count(0),
	sdl_surf(s)
{
	clip_window.w = width = sdl_surf->w;
	clip_window.h = height = sdl_surf->h;
	pitch = sdl_surf->pitch;
	bits_per_pixel = sdl_surf->format->BitsPerPixel;
	bytes_per_pixel = sdl_surf->format->BytesPerPixel;

	r_loss = sdl_surf->format->Rloss;
	g_loss = sdl_surf->format->Gloss;
	b_loss = sdl_surf->format->Bloss;
	r_loss16 = r_loss+8;
	g_loss16 = g_loss+8;
	b_loss16 = b_loss+8;
	r_shift = sdl_surf->format->Rshift;
	g_shift = sdl_surf->format->Gshift;
	b_shift = sdl_surf->format->Bshift;
	r_mask = sdl_surf->format->Rmask;
	g_mask = sdl_surf->format->Gmask;
	b_mask = sdl_surf->format->Bmask;

}


//
// SoftRenderSurface::~SoftRenderSurface()
//
// Desc: Destructor
//
SoftRenderSurface::~SoftRenderSurface()
{
}


//
// SoftRenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode SoftRenderSurface::BeginPainting()
{
	if (!lock_count && sdl_surf) {

		// SDL_Surface requires locking
		if (SDL_MUSTLOCK(sdl_surf))
		{
			// Did the lock fail?
			if (SDL_LockSurface(sdl_surf)!=0) {
				pixels = pixels00 = 0;
				// TODO: SetLastError(GR_SOFT_ERROR_SDL_LOCK_FAILED, "SDL Surface Lock Failed!");
				perr << "Error: SDL Surface Lock Failed!" << std::endl;
				return GR_SOFT_ERROR_SDL_LOCK_FAILED;
			}
		}

		pixels = pixels00 = (uint8*)sdl_surf->pixels;
		pitch = sdl_surf->pitch;
	}
	lock_count++;
	
	if (pixels == 0) 
	{
		// TODO: SetLastError(GR_SOFT_ERROR_LOCKED_NULL_PIXELS, "Surface Locked with NULL SoftRenderSurface::pixels pointer!");
		perr << "Error: Surface Locked with NULL SoftRenderSurface::pixels pointer!" << std::endl;
		return GR_SOFT_ERROR_LOCKED_NULL_PIXELS;
	}

	// No error
	return P_NO_ERROR;
}


//
// SoftRenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode SoftRenderSurface::EndPainting()
{
	// Already Unlocked
	if (!lock_count)
	{
		// TODO: SetLastError(GR_SOFT_ERROR_BEGIN_END_MISMATCH, "BeginPainting()/EndPainting() Mismatch!");
		perr << "Error: BeginPainting()/EndPainting() Mismatch!" << std::endl;
		return GR_SOFT_ERROR_BEGIN_END_MISMATCH;
	}

	// Decrement counter
	--lock_count;

	if (!lock_count) if (sdl_surf) {
		// Unlock the SDL_Surface if required
		if (SDL_MUSTLOCK(sdl_surf)) SDL_UnlockSurface(sdl_surf);

		// Clear pointers
		pixels=pixels00=0;

		// Present
		SDL_UpdateRect(sdl_surf, 0,0,0,0);
	}

	// No error
	return P_NO_ERROR;
}


//
// SoftRenderSurface::SetPalette(uint8 palette[768])
//
// Desc: Set The Surface Palette
//
void SoftRenderSurface::SetPalette(uint8 palette[768])
{
}

//
// U8SoftRenderSurface::U8SoftRenderSurface(SDL_Surface *s)
//
// Desc: Create a U8 SoftRenderSurface from a SDL_Surface
//
template<class uintX> U8SoftRenderSurface<uintX>::U8SoftRenderSurface(SDL_Surface *s)
	: SoftRenderSurface(s)
{
}


/////////////////////////
//                     //
// U8SoftRenderSurface //
//                     //
/////////////////////////


//
// U8SoftRenderSurface::Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h)
//
// Desc: Fill buffer (using a palette index)
//
template<class uintX> void U8SoftRenderSurface<uintX>::Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h)
{
}


//
// U8SoftRenderSurface::Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h)
//
// Desc: Fill buffer (using a RGB colour)
//
template<class uintX> void U8SoftRenderSurface<uintX>::Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h)
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
			*((uintX*) pixel) = rgb;
			pixel+=sizeof(uintX);
		}

		line_end += pitch;
		pixel += diff;
	}

}


//
// U8SoftRenderSurface::Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void U8SoftRenderSurface<uintX>::Blit(Texture *tex, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy)
{
	// Clip dx
	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	// Clip w
	if (w > tex->width) w = tex->width;
	if ((dx + w) > width) w = width - dx;
	
	// Clip dy
	if (dy < 0) {
		sy -= dy;
		h += dy;
		dy = 0;
	}

	// Clip h
	if (h > tex->height) w = tex->height;
	if ((dy + h) > height) h = height - dy;
	

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
				*((uintX*) pixel) = PACK_RGB8( TEX32_R(*texel), TEX32_G(*texel), TEX32_B(*texel) );
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
// U8SoftRenderSurface::PrintCharFixed(Texture *, char character, int x, int y)
//
// Desc: Draw a fixed width character from a Texture buffer
//
template<class uintX> void U8SoftRenderSurface<uintX>::PrintCharFixed(Texture *texture, int character, int x, int y)
{
	int char_width = texture->width/16;
	int char_height = texture->height/16;

	if (char_width == 16 && char_height == 16)
	{
		Blit(texture, (character&0x0F) << 4, character&0xF0, 16, 16, x, y);
	}
	else if (char_width == 8 && char_height == 8)
	{
		Blit(texture, (character&0x0F) << 3, (character>>1)&0x78, 8, 8, x, y);
	}
	else
	{
		Blit(texture,  (character&0x0F) * char_width, (character&0xF0>>4) * char_height, char_width, char_height, x, y);
	}
}


//
// U8SoftRenderSurface::PrintTextFixed(Texture *, const char *text, int x, int y)
//
// Desc: Draw fixed width from a Texture buffer (16x16 characters fixed width and height)
//
template<class uintX> void U8SoftRenderSurface<uintX>::PrintTextFixed(Texture *texture, const char *text, int x, int y)
{
	int char_width = texture->width/16;
	int char_height = texture->height/16;

	int character;
	if (char_width == 16 && char_height == 16) while (character = *text)
	{
		Blit(texture, (character&0x0F) << 4, character&0xF0, 16, 16, x, y);
		++text;
		x+=16;
	}
	else if (char_width == 8 && char_height == 8) while (character = *text)
	{
		Blit(texture, (character&0x0F) << 3, (character>>1)&0x78, 8, 8, x, y);
		++text;
		x+=8;
	}
	else while (character = *text)
	{
		Blit(texture,  (character&0x0F) * char_width, (character&0xF0>>4) * char_height, char_width, char_height, x, y);
		++text;
		x+=char_width;
	}
}


//
// Instantiate the U8SoftRenderSurface Class
//
template class U8SoftRenderSurface<uint16>;
template class U8SoftRenderSurface<uint32>;
