/*
BaseSoftRenderSurface.h : Abstract BaseSoftRenderSurface header

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

#ifndef BASESOFTRENDERSURFACE_H
#define BASESOFTRENDERSURFACE_H

#include "RenderSurface.h"
#include "Rect.h"
#include <SDL.h>

#define UNPACK_RGB8(pix,r,g,b) { r = (((pix)&BaseSoftRenderSurface::r_mask)>>BaseSoftRenderSurface::r_shift)<<BaseSoftRenderSurface::r_loss; g = (((pix)&BaseSoftRenderSurface::g_mask)>>BaseSoftRenderSurface::g_shift)<<BaseSoftRenderSurface::g_loss; b = (((pix)&BaseSoftRenderSurface::b_mask)>>BaseSoftRenderSurface::b_shift)<<BaseSoftRenderSurface::b_loss; }
#define PACK_RGB8(r,g,b) (((r)>>BaseSoftRenderSurface::r_loss)<<BaseSoftRenderSurface::r_shift) | (((g)>>BaseSoftRenderSurface::g_loss)<<BaseSoftRenderSurface::g_shift) | (((b)>>BaseSoftRenderSurface::b_loss)<<BaseSoftRenderSurface::b_shift)
#define PACK_RGB16(r,g,b) (((r)>>BaseSoftRenderSurface::r_loss16)<<BaseSoftRenderSurface::r_shift) | (((g)>>BaseSoftRenderSurface::g_loss16)<<BaseSoftRenderSurface::g_shift) | (((b)>>BaseSoftRenderSurface::b_loss16)<<BaseSoftRenderSurface::b_shift)

//
// Class BaseSoftRenderSurface
//
// Desc: The base abstact class for software rendering in Pentagram
//
class BaseSoftRenderSurface : public RenderSurface
{
protected:
	// Frame buffer
	uint8			*pixels;				// Pointer to logical pixel 0,0
	uint8			*pixels00;				// Pointer to physical pixel 0,0

	// Depth Buffer
	uint16			*zbuffer;				// Pointer to logical pixel 0,0
	uint8			*zbuffer00;				// Pointer to physical pixel 0,0

	// Pixel Format (also see 'Colour shifting values' later)
	int				bytes_per_pixel;		// 2 or 4
	int				bits_per_pixel;			// 16 or 32
	int				format_type;			// 16, 555, 565, 32 or 888

	// Dimensions
	sint32			ox, oy;					// Physical Pixel for Logical Origin
	uint32			width, height;			// Width and height
	uint32			pitch;					// Frame buffer pitch (bytes)
	uint32			zpitch;					// Z Buffer pitch (bytes)

	// Clipping Rectangle
	Pentagram::Rect	clip_window;

	// Locking count
	uint32			lock_count;				// Number of locks on surface

	// SDL_Surface
	SDL_Surface		*sdl_surf;

	// Create from a SDL_Surface
	BaseSoftRenderSurface(SDL_Surface *);

	// Create Generic
	BaseSoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft);
	BaseSoftRenderSurface(int w, int h, uint8 *buf);
	virtual ECode GenericLock()  { return P_NO_ERROR; }
	virtual ECode GenericUnlock()  { return P_NO_ERROR; }

public:

	// Colour shifting values (should these all be uint32???)
	static uint8	r_loss,   g_loss,   b_loss;
	static uint8	r_loss16, g_loss16, b_loss16;
	static uint8	r_shift,  g_shift,  b_shift;
	static uint32	r_mask,   g_mask,   b_mask;
	static uint32	s_bpp;

	// Virtual Destructor
	virtual ~BaseSoftRenderSurface();

	//
	// Being/End Painting
	//

	// Begin painting to the buffer. MUST BE CALLED BEFORE DOING ANYTHING TO THE SURFACE!
	// Can be called multiple times
	// Returns Error Code on error. Check return code.....
	virtual ECode BeginPainting();

	// Finish paining to the buffer. MUST BE CALLED FOR EACH CALL TO BeginPainting()
	// Returns Error Code on error. Check return code.....
	virtual ECode EndPainting();

	
	//
	// Surface Properties
	//

	// Set the Origin of the Surface
	virtual void SetOrigin(sint32 x, sint32 y);

	// Set the Origin of the Surface
	virtual void GetOrigin(sint32 &x, sint32 &y) const;

	// Get the Surface Dimensions
	virtual void GetSurfaceDims(Pentagram::Rect &) const;

	// Get Clipping Rectangle
	virtual void GetClippingRect(Pentagram::Rect &) const;

	// Set Clipping Rectangle
	virtual void SetClippingRect(const Pentagram::Rect &);

	// Check Clipped. -1 if off screen, 0 if not clipped, 1 if clipped
	virtual sint16 CheckClipped(const Pentagram::Rect &) const;

	//
	// Surface Palettes
	//
	// TODO: Make a Palette class
	// TODO: Handle Ultima8 and Crusader Xforms
	//

	// Set The Surface Palette
	// virtual void SetPalette(uint8 palette[768]);

	// Set The Surface Palette to be the one used by another surface
	// TODO: virtual void SetPalette(RenderSurface &);

	// Get The Surface Palette
	// TODO: virtual void GetPalette(uint8 palette[768]);

	virtual void CreateNativePalette(Pentagram::Palette* palette);

};

#endif
