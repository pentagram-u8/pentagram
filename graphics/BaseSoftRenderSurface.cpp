/*
BaseSoftRenderSurface.cpp : Abstract BaseSoftRenderSurface source file

Copyright (C) 2003 The Pentagram Team

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
#include "Palette.h"

///////////////////////////
//                       //
// BaseSoftRenderSurface //
//                       //
///////////////////////////

uint8	BaseSoftRenderSurface::r_loss,   BaseSoftRenderSurface::g_loss,   BaseSoftRenderSurface::b_loss;
uint8	BaseSoftRenderSurface::r_loss16, BaseSoftRenderSurface::g_loss16, BaseSoftRenderSurface::b_loss16;
uint8	BaseSoftRenderSurface::r_shift,  BaseSoftRenderSurface::g_shift,  BaseSoftRenderSurface::b_shift;
uint32	BaseSoftRenderSurface::r_mask,   BaseSoftRenderSurface::g_mask,   BaseSoftRenderSurface::b_mask;


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(SDL_Surface *s)
//
// Desc: Constructor for BaseSoftRenderSurface from a SDL_Surface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(SDL_Surface *s) :
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
// BaseSoftRenderSurface::~BaseSoftRenderSurface()
//
// Desc: Destructor
//
BaseSoftRenderSurface::~BaseSoftRenderSurface()
{
}


//
// BaseSoftRenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::BeginPainting()
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

		pixels = pixels00 = static_cast<uint8*>(sdl_surf->pixels);
		pitch = sdl_surf->pitch;
	}
	lock_count++;
	
	if (pixels == 0) 
	{
		// TODO: SetLastError(GR_SOFT_ERROR_LOCKED_NULL_PIXELS, "Surface Locked with NULL BaseSoftRenderSurface::pixels pointer!");
		perr << "Error: Surface Locked with NULL BaseSoftRenderSurface::pixels pointer!" << std::endl;
		return GR_SOFT_ERROR_LOCKED_NULL_PIXELS;
	}

	// No error
	return P_NO_ERROR;
}


//
// BaseSoftRenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::EndPainting()
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
// void BaseSoftRenderSurface::CreateNativePalette(Palette* palette)
//
// Create a palette of colours native to the surface
//
void BaseSoftRenderSurface::CreateNativePalette(Palette* palette)
{
	for (int i = 0; i < 256; i++)
	{
		palette->native[i] = SDL_MapRGB(sdl_surf->format,
										palette->palette[i*3],
										palette->palette[i*3+1],
										palette->palette[i*3+2]);
	}
}



