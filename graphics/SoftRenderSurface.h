/*
SoftRenderSurface.h : SoftRenderSurface Implementation header

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

#ifndef SOFTRENDERSURFACE_H
#define SOFTRENDERSURFACE_H

#include "RenderSurface.h"
#include "Rect.h"
#include <SDL.h>

#define UNPACK_RGB8(pix,r,g,b) { r = (((pix)&SoftRenderSurface::r_mask)>>SoftRenderSurface::r_shift)<<SoftRenderSurface::r_loss; g = (((pix)&SoftRenderSurface::g_mask)>>SoftRenderSurface::g_shift)<<SoftRenderSurface::g_loss; b = (((pix)&SoftRenderSurface::b_mask)>>SoftRenderSurface::b_shift)<<SoftRenderSurface::b_loss; }
#define PACK_RGB8(r,g,b) (((r)>>SoftRenderSurface::r_loss)<<SoftRenderSurface::r_shift) | (((g)>>SoftRenderSurface::g_loss)<<SoftRenderSurface::g_shift) | (((b)>>SoftRenderSurface::b_loss)<<SoftRenderSurface::b_shift)
#define PACK_RGB16(r,g,b) (((r)>>SoftRenderSurface::r_loss16)<<SoftRenderSurface::r_shift) | (((g)>>SoftRenderSurface::g_loss16)<<SoftRenderSurface::g_shift) | (((b)>>SoftRenderSurface::b_loss16)<<SoftRenderSurface::b_shift)

//
// Class SoftRenderSurface
//
// Desc: The base abstact class for software rendering in Pentagram
//
class SoftRenderSurface : public RenderSurface
{
protected:
	// Frame buffer
	uint8			*pixels;				// Pointer to top Left pixel in clipping window
	uint8			*pixels00;				// Pointer to pixel 0,0

	// Depth Buffer
	uint16			*zbuffer;				// Pointer to top left pixel in clipping window
	uint8			*zbuffer00;				// Pointer to pixel 0,0

	// Colour Palette
	uint32			colour32[256];			// Palette as 16/32 bit colours - TODO: Palette Class

	// Pixel Format (also see 'Colour shifting values' later)
	int				bytes_per_pixel;		// 2 or 4
	int				bits_per_pixel;			// 16 or 32
	int				format_type;			// 16, 555, 565, 32 or 888

	// Dimensions
	uint32			w_real, h_real;			// Actual Surface width and height
	uint32			width, height;			// Effective Width and height (after clipping)
	uint32			pitch;					// Frame buffer pitch (bytes)
	uint32			zpitch;					// Z Buffer pitch (bytes)

	// Clipping Rectangle - Should be public???
	Rect			clip_window;

	// TODO: Guardband (implementing this might be more difficult than previously expected)
	//uint32			gl, gr;					// Guard left and right (left goes negative)
	//uint32			gt, gb;					// Guard top and bottom (up goes negative)

	// Locking count
	uint32			lock_count;				// Number of locks on surface

	// SDL_Surface
	SDL_Surface		*sdl_surf;

	// Create from a SDL_Surface
	SoftRenderSurface(SDL_Surface *);

public:

	// Colour shifting values (should these all be uint32???)
	static uint8	r_loss,   g_loss,   b_loss;
	static uint8	r_loss16, g_loss16, b_loss16;
	static uint8	r_shift,  g_shift,  b_shift;
	static uint32	r_mask,   g_mask,   b_mask;

	// Virtual Destructor
	virtual ~SoftRenderSurface();


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

	// Get Clipping Rectangle
	// TODO: virtual void GetClippingRect(Rect &);

	// Set Clipping Rectangle
	// TODO: virtual void SetClippingRect(Rect &);


	//
	// Surface Palettes
	//
	// TODO: Make a Palette class
	// TODO: Handle Ultima8 and Crusader Xforms
	//

	// Set The Surface Palette
	virtual void SetPalette(uint8 palette[768]);

	// Set The Surface Palette to be the one used by another surface
	// TODO: virtual void SetPalette(RenderSurface &);

	// Get The Surface Palette
	// TODO: virtual void GetPalette(uint8 palette[768]);
};


//
// U8SoftRenderSurface
//
// Desc: The class for software rendering Ultima 8 style in Pentagram
//
// TODO: Get rid of this, move everything to Crusader style
//
template<class uintX> class U8SoftRenderSurface : public SoftRenderSurface
{
public:

	// Create from a SDL_Surface
	U8SoftRenderSurface(SDL_Surface *);

	//
	// Surface Filling
	//

	// Fill buffer (using a palette index)
	virtual void Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h);

	// Fill buffer (using a RGB colour)
	virtual void Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h);


	//
	// The rule for painting methods:
	//
	// First arg are the source object to 'draw' with
	// Next args are any other required data to define the 'source'
	// Next args are the destination position
	//

	//
	// Basic Shape Painting
	//

	// Paint a Shape
	// TODO: virtual void Paint(CachedShape* s, uint32 frame, sint32 x, sint32 y);

	// Paint a Translucent Shape. 
	// TODO: virtual void PaintTranslucent(CachedShape* s, uint32 frame, sint32 x, sint32 y);

	// Paint a Mirrored Shape
	// TODO: virtual void PaintMirrored(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans = false);


	//
	// Basic Line Drawing
	//
	
	// Draw a Line (using a palette index)
	// TODO: virtual void DrawLine8(uint8 index, sint32 sx, sint32 sy, sint32 ex, sint32 ey);

	// Draw a RGB Line
	// TODO: virtual void DrawLine32(uint32 rgb, sint32 sx, sint32 sy, sint32 ex, sint32 ey);


	//
	// Basic Font Drawing
	//
	
	// Draw from a Font
	// TODO: virtual void PrintText(Font *, const char *text, int x, int y);

	// Draw a character from a Font
	// TODO: virtual void PrintChar(Font *, char character, int x, int y);

	// Draw fixed width from a Texture buffer (16x16 characters fixed width and height)
	virtual void PrintTextFixed(Texture *, const char *text, int x, int y);

	// Draw a fixed width character from a Texture buffer
	virtual void PrintCharFixed(Texture *, int character, int x, int y);


	//
	// Basic Texture Blitting
	//

	// Blit a region from a Texture (Alpha == 0 -> skipped)
	virtual void Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy);

	// Blit a region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void AlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy);

	// Blit a stretched region from a Texture (Alpha == 0 -> skipped)
	// TODO: virtual void StretchBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh);

	// Blit a stretched region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void StretchAlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh);


	////////////////////////////////////////
	// TODO: Add in Abstract 3d code Here //
	////////////////////////////////////////
};

#endif
