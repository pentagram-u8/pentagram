/*
Copyright (C) 2003  The Pentagram Team

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
// #define FLIP_SHAPES to flip rendering
//
// #define NO_CLIPPING to disable shape clipping
//
// #define XFORM_SHAPES to enable XFORMing
//
// #define XFORM_CONDITIONAL to an argument of the function so XFORM can be 
// enabled/disabled with a bool
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

//
// XForm = TRUE
//
#ifdef XFORM_SHAPES

#ifdef XFORM_CONDITIONAL
#define USE_XFORM_FUNC ((XFORM_CONDITIONAL) && xf_func != 0)
#else
#define USE_XFORM_FUNC (xf_func != 0)
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
#define XNEG(x)(-(x))

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

//
// No Clipping = FALSE
//	
#else

#define LINE_END_ASSIGN() line_end = line_start+scrn_width;
#define NOT_CLIPPED_Y (line >= 0 && line < scrn_height)
#define NOT_CLIPPED_X (pixptr >= line_start && pixptr < line_end)

	int					scrn_width = this->width;
	int					scrn_height = this->height;
	uintX				*line_end;

#endif


//
// The Function
//

// All the variables we want

	const uint8			*linedata;
	int					xpos;
	int					line;
	int					dlen;

	uintX				*pixptr;
	uintX				*endrun;
	uintX				*line_start;
	uintX				pix;

	// Sanity check
	if (framenum >= s->frameCount()) return;
	if (s->getPalette() == 0) return;

	ShapeFrame		*frame			= s->getFrame(framenum);
	const uint8		*rle_data		= frame->rle_data;
	const uint32	*line_offsets	= frame->line_offsets;
	const uint32	*pal			= &(s->getPalette()->native[0]);

#ifdef XFORM_SHAPES
	const xformBlendFuncType	*xform_funcs = s->getPalette()->xform_funcs;
	xformBlendFuncType			xf_func;
#endif


	sint32 width = frame->width;
	sint32 height = frame->height;
	x -= XNEG(frame->xoff);
	y -= frame->yoff;

	// Do it this way if compressed
	if (frame->compressed) for (int i=0; i<height; i++) 
	{

		linedata = rle_data + line_offsets[i];
		xpos = 0;
		line = y+i;

		if (NOT_CLIPPED_Y)
		{

			line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(pixels) + pitch*line);
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
						if (NOT_CLIPPED_X) 
						{
							#ifdef XFORM_SHAPES
							xf_func = xform_funcs[*linedata];
							if (USE_XFORM_FUNC) 
							{
								*pixptr = xf_func(*pixptr);
							}
							else 
							#endif
							{
								*pixptr = pal[*linedata];
							}
						}
						pixptr += XNEG(1);
						linedata++;
					}
				} 
				else 
				{
					#ifdef XFORM_SHAPES
					xf_func = xform_funcs[*linedata];
					if (USE_XFORM_FUNC) 
					{
						while (pixptr != endrun) 
						{
							if (NOT_CLIPPED_X) *pixptr = xf_func(*pixptr);
							pixptr += XNEG(1);
						}
					} 
					else 
					#endif
					{
						pix = pal[*linedata];
						while (pixptr != endrun) 
						{
							if (NOT_CLIPPED_X) 
							{
								*pixptr = pix;
							}
							pixptr += XNEG(1);
						}
					}	
					linedata++;
				}

				xpos += dlen;

			} while (xpos < width);
		}
	}
	// Uncompressed
	else for (int i=0; i<height; i++) 
	{
		linedata = rle_data + line_offsets[i];
		xpos = 0;
		line = y+i;

		if (NOT_CLIPPED_Y)
		{
			line_start = reinterpret_cast<uintX *>(static_cast<uint8*>(pixels) + pitch*line);
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
					if (NOT_CLIPPED_X) 
					{
						#ifdef XFORM_SHAPES
						xf_func = xform_funcs[*linedata];
						if (USE_XFORM_FUNC) 
						{
							*pixptr = xf_func(*pixptr);
						}
						else 
						#endif
						{
							*pixptr = pal[*linedata];
						}
					}
					pixptr += XNEG(1);
					linedata++;
				}

				xpos += dlen;

			} while (xpos < width);
		}
	}

#undef LINE_END_ASSIGN
#undef NOT_CLIPPED_X
#undef NOT_CLIPPED_Y
#undef XNEG
#undef USE_XFORM_FUNC
