/*
RenderSurface.h : RenderSurface Interface header

Copyright (C) 2002-2004 The Pentagram Team

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

#ifndef RENDERSURFACE_H
#define RENDERSURFACE_H

#include "GraphicsErrors.h"

struct Texture;
class Shape;
class ShapeFont;
namespace Pentagram
{
	struct Palette;
	struct Rect;
}

//
// RenderSurface
//
// Desc: The base abstact class for rendering in Pentagram
//
class RenderSurface
{
public:

	//
	// TODO: Improve the way SDL Surfaces are created. Should be more versatile.
	//

	// Create a standard RenderSurface
	static RenderSurface *SetVideoMode(uint32 width, uint32 height, uint32 bpp, bool fullscreen, bool use_opengl);

	// Virtual Destructor
	virtual ~RenderSurface();


	//
	// Being/End Painting
	//

	// Begin painting to the buffer. MUST BE CALLED BEFORE DOING ANYTHING TO THE SURFACE!
	// Can be called multiple times
	// Returns Error Code on error. Check return code.....
	virtual ECode BeginPainting() = 0;

	// Finish paining to the buffer. MUST BE CALLED FOR EACH CALL TO BeginPainting()
	// Returns Error Code on error. Check return code.....
	virtual ECode EndPainting() = 0;


	//
	// Surface Properties
	//

	// Set the Origin of the Surface
	virtual void SetOrigin(sint32 x, sint32 y) = 0;

	// Set the Origin of the Surface
	virtual void GetOrigin(sint32 &x, sint32 &y) const = 0;

	// Get the Surface Dimensions
	virtual void GetSurfaceDims(Pentagram::Rect &) const = 0;

	// Get Clipping Rectangle
	virtual void GetClippingRect(Pentagram::Rect &) const = 0;

	// Set Clipping Rectangle
	virtual void SetClippingRect(const Pentagram::Rect &) = 0;

	// Check Clipped. -1 if off screen, 0 if not clipped, 1 if clipped
	virtual sint16 CheckClipped(const Pentagram::Rect &) const = 0;


	//
	// Surface Palettes
	//
	// TODO: Handle Ultima8 and Crusader Xforms
	//

	// Set The Surface Palette
	// virtual void SetPalette(uint8 palette[768]) = 0;

	// Set The Surface Palette to be the one used by another surface
	// TODO: virtual void SetPalette(RenderSurface &) = 0;

	// Get The Surface Palette
	// TODO: virtual void GetPalette(uint8 palette[768]) = 0;

	virtual void CreateNativePalette(Pentagram::Palette* palette) = 0;


	//
	// Surface Filling
	//

	// Fill buffer (using a palette index)
	virtual void Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h) = 0;

	// Fill buffer (using a RGB colour)
	virtual void Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h) = 0;


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
	// TODO: virtual void Paint(CachedShape* s, uint32 frame, sint32 x, sint32 y) = 0;
	virtual void Paint(Shape*s, uint32 frame, sint32 x, sint32 y) = 0;

	// Paint a Shape without clipping
	// TODO: virtual void PaintNoClip(CachedShape*s, uint32 frame, sint32 x, sint32 y) = 0;
	virtual void PaintNoClip(Shape*s, uint32 frame, sint32 x, sint32 y) = 0;

	// Paint a Translucent Shape. 
	// TODO: virtual void PaintTranslucent(CachedShape* s, uint32 frame, sint32 x, sint32 y) = 0;
	virtual void PaintTranslucent(Shape* s, uint32 frame, sint32 x, sint32 y) = 0;

	// Paint a Mirrored Shape
	// TODO: virtual void PaintMirrored(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans = false) = 0;
	virtual void PaintMirrored(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans = false) = 0;

	// Paint an Invisible Shape
	// TODO: virtual void PaintInvisible(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored) = 0;
	virtual void PaintInvisible(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored) = 0;

	// Paint a Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	// TODO: virtual void PaintHighlight(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32);
	virtual void PaintHighlight(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32) = 0;

	//
	// Basic Line Drawing
	//
	
	// Draw a Line (using a palette index)
	// TODO: virtual void DrawLine8(uint8 index, sint32 sx, sint32 sy, sint32 ex, sint32 ey) = 0;

	// Draw a RGB Line
	// TODO: virtual void DrawLine32(uint32 rgb, sint32 sx, sint32 sy, sint32 ex, sint32 ey) = 0;


	//
	// Basic Font Drawing
	//
	// Draw fixed width from a Texture buffer
	// (16x16 characters fixed width and height)
	virtual void PrintTextFixed(Texture *, const char *text, int x, int y) = 0;

	// Draw a fixed width character from a Texture buffer
	virtual void PrintCharFixed(Texture *, int character, int x, int y) = 0;


	//
	// Basic Texture Blitting
	//

	// Blit a region from a Texture (Alpha == 0 -> skipped)
	virtual void Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy) = 0;

	// Blit a region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void AlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy) = 0;

	// Blit a stretched region from a Texture (Alpha == 0 -> skipped)
	// TODO: virtual void StretchBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh) = 0;

	// Blit a stretched region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void StretchAlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh) = 0;


	////////////////////////////////////////
	// TODO: Add in Abstract 3d code Here //
	////////////////////////////////////////
};

#endif
