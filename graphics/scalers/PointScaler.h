/*
Copyright (C) 2005 The Pentagram Team

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

//#include "Scaler.h"
#include "Texture.h"

// Very very simple pointer scaler
template<class uintX, class Manip, class uintS=uintX> class PointScaler
{
public:
	static bool Scale( Texture *tex	, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch)
	{
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS*>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
		uintS *tline_end = texel + sw;
		uintS *tex_end = texel + sh*tex->width;
		int tex_diff = tex->width - sw;


		// First detect integer up scalings, since they are 'easy'
		bool x_intscale = ((dw / sw) * sw) == dw;
		bool y_intscale = ((dh / sh) * sh) == dh;

		// Integer scaling, x and y
		if (x_intscale && y_intscale)
		{
			int xf = dw/sw;
			int yf = dh/sh;

			uint8 *px_end = pixel + xf*sizeof(uintX);
			uint8 *py_end = pixel + yf*pitch;

			int block_w = xf*sizeof(uintX);
			int block_h = pitch*yf;
			int block_xdiff = pitch - block_w;
			int p_diff = block_h - dw*sizeof(uintX);

			// Src Loop Y
			while (texel != tex_end)
			{
				// Src Loop X
				while (texel != tline_end)
				{
					uintX p = Manip::copy(*texel);
					texel++;

					//
					// Inner loops
					//

					// Dest Loop Y
					while (pixel != py_end)
					{
						// Dest Loop X
						while (pixel != px_end)
						{
							*(reinterpret_cast<uintX*>(pixel)) = p;
							pixel+=sizeof(uintX);
						}
						pixel+=block_xdiff;
						px_end+=pitch;
					}

					pixel  += block_w-block_h;
					px_end += block_w-block_h;
					py_end += block_w;
				}

				pixel += p_diff;
				py_end += p_diff;
				px_end += p_diff;

				texel += tex_diff;
				tline_end += tpitch;
			}

		}

		return TRUE;
	}

};