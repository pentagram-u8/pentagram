/*
Copyright (C) 2003-2025  The Pentagram Team

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

//
// Render Surface Shape Display include file
//

//
// Macros to define before including this:
//
// #define NO_CLIPPING to disable shape clipping
//
// #define FLIP_SHAPES to flip rendering
//
// #define FLIP_CONDITIONAL to an argument of the function so FLIPPING can be 
// enabled/disabled with a bool
//
// #define XFORM_SHAPES to enable XFORMing
//
// #define XFORM_CONDITIONAL to an argument of the function so XFORM can be 
// enabled/disabled with a bool
//
// #define BLEND_SHAPES(src,dst) to an a specified blend function.
//
// #define BLEND_CONDITIONAL to an argument of the function so BLEND
// painting can be enabled/disabled with a bool
//

//
// Macros defined by this file:
//
// NOT_CLIPPED_Y - Does Y Clipping check per line
// 
// NOT_CLIPPED_X - Does X Clipping check per Pixel
// 
// LINE_END_ASSIGN - Calcuates the line_end pointer required for X clipping
//
// XNEG - Negates X values if doing shape flipping
// 
// USE_XFORM_FUNC - Checks to see if we want to use XForm Blending for this pixel
// 
// CUSTOM_BLEND - Final Blend for invisiblity
//

//
// XForm = TRUE
//
#ifdef XFORM_SHAPES

#ifdef XFORM_CONDITIONAL
#define USE_XFORM_FUNC ((XFORM_CONDITIONAL) && xform_pal[*linedata])
#else
#define USE_XFORM_FUNC (xform_pal[*linedata])
#endif

//
// XForm = FALSE
//
#else
#define USE_XFORM_FUNC 0
#endif


//
// Flipping = TRUE
//
#ifdef FLIP_SHAPES

#ifdef FLIP_CONDITIONAL
const sint32 neg = (FLIP_CONDITIONAL)?-1:0;
#define XNEG(x) (((x)+neg)^neg)
#else
#define XNEG(x) (-(x))
#endif

// Flipping = FALSE
#else
#define XNEG(x)(+(x))
#endif


//
// No Clipping = TRUE
//	
#ifdef NO_CLIPPING

#define LINE_END_ASSIGN()
#define NOT_CLIPPED_X (1)
#define NOT_CLIPPED_Y (1)
#define OFFSET_PIXELS (pixels)

//
// No Clipping = FALSE
//	
#else

#define LINE_END_ASSIGN() do { line_end = line_start+scrn_width; } while (0)
#define NOT_CLIPPED_Y (line >= 0 && line < scrn_height)
#define NOT_CLIPPED_X (pixptr >= line_start && pixptr < line_end)

int					scrn_width = clip_window.w;
int					scrn_height = clip_window.h;
uintX				*line_end;

#define OFFSET_PIXELS (off_pixels)

uint8				*off_pixels  = static_cast<uint8*>(pixels) + static_cast<sintptr>(clip_window.x)*sizeof(uintX) + static_cast<sintptr>(clip_window.y)*pitch;
x -= clip_window.x;
y -= clip_window.y;

#endif


//
// Invisilibity = TRUE
//
#ifdef BLEND_SHAPES

#ifdef BLEND_CONDITIONAL
#define CUSTOM_BLEND(src) static_cast<uintX>((BLEND_CONDITIONAL)?BLEND_SHAPES(src,*pixptr):src)
#else
#define CUSTOM_BLEND(src) static_cast<uintX>(BLEND_SHAPES(src,*pixptr))
#endif

//
// Invisilibity = FALSE
//
#else

#define CUSTOM_BLEND(src) static_cast<uintX>(src)

#endif

//
// Destination Alpha Masking
//
#ifdef DESTALPHA_MASK

#define NOT_DESTINATION_MASKED	(*pixptr & RenderSurface::format.a_mask)

#else

#define NOT_DESTINATION_MASKED	(1)

#endif

//
// The Function
//

// All the variables we want

const uint8			*linedata;
sint32				xpos;
sintptr				line; // sintptr for pointer arithmetic
sint32				dlen;

uintX				*pixptr;
uintX				*endrun;
uintX				*line_start;
//uint32				pix;

// Sanity check
if (framenum >= s->frameCount()) return;
if (s->getPalette() == 0) return;

// ShapeFrame		*frame			= s->getFrame(framenum); // <-- REMOVE THIS LINE
const uint8		*rle_data		= frame->rle_data; // 'frame' is now from pent_shp.cc
const uint32	*line_offsets	= frame->line_offsets; // 'frame' is now from pent_shp.cc
/*const uint32	*pal			= untformed_pal?
	&(s->getPalette()->native_untransformed[0]):
&(s->getPalette()->native[0]);*/

    
#ifdef XFORM_SHAPES
const uint32	*xform_pal		= untformed_pal?
	&(s->getPalette()->xform_untransformed[0]):
&(s->getPalette()->xform[0]);
#endif

sint32 width = frame->width; // 'frame' is now from pent_shp.cc
sint32 height = frame->height; // 'frame' is now from pent_shp.cc
x -= XNEG(frame->xoff); // 'frame' is now from pent_shp.cc
y -= frame->yoff; // 'frame' is now from pent_shp.cc

// Do it this way if compressed
if (frame->compressed) for (int i=0; i<height; i++) // <-- CORRECTED: Added 'int'
{
	xpos = 0;
	line = y+i;

	if (NOT_CLIPPED_Y)
	{

		linedata = rle_data + line_offsets[i];
		line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(OFFSET_PIXELS) + pitch*line);

		LINE_END_ASSIGN();

		do 
		{
			xpos += *linedata++;
			  
			if (xpos == width) break;

			dlen = *linedata++;
			int type = dlen & 1;
			dlen >>= 1;

			pixptr= line_start+x+XNEG(xpos);
			endrun = pixptr + XNEG(dlen);
				
			if (!type) 
			{
				while (pixptr != endrun) 
				{
					if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) 
					{
#ifndef PENT_SHP_OUTPUT_8BIT_INDICES // Original Logic
#ifdef XFORM_SHAPES
						if (USE_XFORM_FUNC) 
						{
							*pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
						}
						else 
#endif
						{
							*pixptr = CUSTOM_BLEND(pal[*linedata]);
						}
#else // PENT_SHP_OUTPUT_8BIT_INDICES Logic
						*pixptr = CUSTOM_BLEND(*linedata); // Write the index directly
#endif
					}
					pixptr += XNEG(1);
					linedata++;
				}
			} 
			else // This is the solid run part
			{
#ifndef PENT_SHP_OUTPUT_8BIT_INDICES // Original Logic
#ifdef XFORM_SHAPES
				pix = xform_pal[*linedata]; // pix is uint32
				if (USE_XFORM_FUNC) 
				{
					while (pixptr != endrun) 
					{
						if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) *pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
						pixptr += XNEG(1);
					}
				} 
				else 
#endif
				{
					pix = pal[*linedata]; // pix is uint32
					while (pixptr != endrun) 
					{
						if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) 
						{
							*pixptr = CUSTOM_BLEND(pix);
						}
						pixptr += XNEG(1);
					}
				}	
#else // PENT_SHP_OUTPUT_8BIT_INDICES Logic for solid run
				uint8_t solid_run_idx = *linedata; // Get the index for the solid run
				while (pixptr != endrun)
				{
					if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED)
					{
						*pixptr = CUSTOM_BLEND(solid_run_idx); // Write the same index
					}
					pixptr += XNEG(1);
				}
#endif
				linedata++; // Consume the single color index for the solid run
			}
			xpos += dlen; // This should be present, advances xpos for the segment

		} while (xpos < width); // This closes the do-while loop
	} // This closes if(NOT_CLIPPED_Y)
} // This closes the for (int i=0; i<height; i++) for compressed frames

// Uncompressed
else for (int i=0; i<height; i++) 
{
	linedata = rle_data + line_offsets[i];
	xpos = 0;
	line = y+i;

	if (NOT_CLIPPED_Y)
	{
		line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(OFFSET_PIXELS) + pitch*line);
		LINE_END_ASSIGN();

		do 
		{
			xpos += *linedata++;
              
			if (xpos == width) break;

			dlen = *linedata++;

			pixptr= line_start+x+XNEG(xpos);
			endrun = pixptr + XNEG(dlen);

			while (pixptr != endrun) 
			{
				if (NOT_CLIPPED_X && NOT_DESTINATION_MASKED) 
				{
#ifndef PENT_SHP_OUTPUT_8BIT_INDICES // Original Logic
#ifdef XFORM_SHAPES
					if (USE_XFORM_FUNC) 
					{
						*pixptr = CUSTOM_BLEND(BlendPreModulated(xform_pal[*linedata],*pixptr));
					}
					else 
#endif
					{
						*pixptr = CUSTOM_BLEND(pal[*linedata]);
					}
#else // PENT_SHP_OUTPUT_8BIT_INDICES Logic
					*pixptr = CUSTOM_BLEND(*linedata); // Write the index directly
#endif
				}
				pixptr += XNEG(1);
				linedata++;
			}

			xpos += dlen;

		} while (xpos < width);
	}
}

#undef NOT_DESTINATION_MASKED
#undef OFFSET_PIXELS
#undef CUSTOM_BLEND
#undef LINE_END_ASSIGN
#undef NOT_CLIPPED_X
#undef NOT_CLIPPED_Y
#undef XNEG
#undef USE_XFORM_FUNC
