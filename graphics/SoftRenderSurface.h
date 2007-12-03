/*
SoftRenderSurface.h : SoftRenderSurface Implementation header

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

#ifndef SOFTRENDERSURFACE_H
#define SOFTRENDERSURFACE_H

#include "BaseSoftRenderSurface.h"

//
// SoftRenderSurface
//
// Desc: The class for software rendering in Pentagram
//
template<class uintX> class SoftRenderSurface : public BaseSoftRenderSurface
{
protected:
	// Create Generic surface
	SoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft, int asft);

public:

	// Create from a SDL_Surface
	SoftRenderSurface(SDL_Surface *);

	// Create a Generic surface that matches current screen parameters
	SoftRenderSurface(int w, int h, uint8 *buf);

	// Create a Render to texture surface
	SoftRenderSurface(int w, int h);

	//
	// Surface Filling
	//

	// Fill buffer (using a palette index)
	virtual void Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h);

	// Fill buffer (using a RGB colour)
	virtual void Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h);

	//! Fill alpha channel
	virtual void FillAlpha(uint8 alpha, sint32 sx, sint32 sy, sint32 w, sint32 h);

	// Fill the region doing alpha blending 
	virtual void FillBlended(uint32 rgba, sint32 sx, sint32 sy, sint32 w, sint32 h);

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
	virtual void Paint(Shape*s, uint32 frame, sint32 x, sint32 y, bool untformed_pal = false);

	// Paint an Shape without clipping
	// TODO: virtual void PaintNoClip(CachedShape*s, uint32 frame, sint32 x, sint32 y);
	virtual void PaintNoClip(Shape*s, uint32 frame, sint32 x, sint32 y, bool untformed_pal = false);

	// Paint a Translucent Shape. 
	// TODO: virtual void PaintTranslucent(CachedShape* s, uint32 frame, sint32 x, sint32 y);
	virtual void PaintTranslucent(Shape* s, uint32 frame, sint32 x, sint32 y, bool untformed_pal = false);

	// Paint a Mirrored Shape
	// TODO: virtual void PaintMirrored(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans = false);
	virtual void PaintMirrored(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans = false, bool untformed_pal = false);

	// Paint a Invisible Shape
	// TODO: virtual void PaintInvisible(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored);
	virtual void PaintInvisible(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, bool untformed_pal = false);

	// Paint a Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	// TODO: virtual void PaintHighlight(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32);
	virtual void PaintHighlight(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false);

	// Paint a Invisible Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	// TODO: virtual void PaintHighlightInvis(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32);
	virtual void PaintHighlightInvis(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false);

	// Paint a shape masked against destination alpha
	virtual void PaintMasked(Shape* s, uint32 framenum, sint32 x, sint32 y, bool trans, bool mirrored, uint32 col32 = 0, bool untformed_pal = false);


	//
	// Basic Line Drawing
	//
	
	// Draw a Line (using a palette index)
	// TODO: virtual void DrawLine8(uint8 index, sint32 sx, sint32 sy, sint32 ex, sint32 ey);

	// Draw a RGB Line
	virtual void DrawLine32(uint32 rgb, sint32 sx, sint32 sy, sint32 ex, sint32 ey);


	//
	// Basic Font Drawing
	//
	// Draw FixedWidthFont
	virtual void PrintTextFixed(FixedWidthFont *, const char *text, int x, int y);

	// Draw a fixed width character from a FixedWidthFont
	virtual void PrintCharFixed(FixedWidthFont *, int character, int x, int y);


	//
	// Basic Texture Blitting
	//

	// Blit a region from a Texture (Alpha == 0 -> skipped)
	virtual void Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy, bool alpha_blend=false);

	// Blit a region from a Texture with a Colour blend (AlphaTex == 0 -> skipped. AlphaCol32 -> Blend Factors)
	virtual void FadedBlit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy, uint32 col32, bool alpha_blend=false);

	// Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
	virtual void MaskedBlit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy, uint32 col32, bool alpha_blend=false);

	// Blit a stretched region from a Texture (Alpha == 0 -> skipped)
	virtual void StretchBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh, bool bilinear = false, bool clampedges = false);

	// Blit a region from a Texture using a scaler
	virtual bool ScalerBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh, const Pentagram::Scaler *, bool clampedges = false);

	////////////////////////////////////////
	// TODO: Add in Abstract 3d code Here //
	////////////////////////////////////////
};

#endif
