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

#include "pent_include.h"
#include "BilinearScaler.h"
#include "Manips.h"

#define SimpleLerp(a,b,fac) ((b<<8)+((a)-(b))*(fac))
#define SimpleLerp2(a,b,fac) ((b<<16)+((a)-(b))*(fac))

#define CopyLerp(d,a,b,f) { \
			(d)[0] = SimpleLerp2(b[0],a[0],f)>>16;\
			(d)[1] = SimpleLerp2(b[1],a[1],f)>>16;\
			(d)[2] = SimpleLerp2(b[2],a[2],f)>>16;\
			(d)[3] = SimpleLerp2(b[3],a[3],f)>>16;}

#define FilterPixel(a,b,f,g,fx,fy) { \
		*reinterpret_cast<uintX*>(pixel) = Manip::merge( \
			SimpleLerp(SimpleLerp(a[0],f[0],fx),SimpleLerp(b[0],g[0],fx),fy)>>16,\
			SimpleLerp(SimpleLerp(a[1],f[1],fx),SimpleLerp(b[1],g[1],fx),fy)>>16,\
			SimpleLerp(SimpleLerp(a[2],f[2],fx),SimpleLerp(b[2],g[2],fx),fy)>>16,\
			SimpleLerp(SimpleLerp(a[3],f[3],fx),SimpleLerp(b[3],g[3],fx),fy)>>16);}

#define ScalePixel2x(a,b,f,g) { \
		*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(a[0], a[1], a[2], a[3]); \
		*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge((a[0]+f[0])>>1, (a[1]+f[1])>>1, (a[2]+f[2])>>1, (a[3]+f[3])>>1); \
		pixel+=pitch; \
		*(reinterpret_cast<uintX*>(pixel)) = Manip::merge((a[0]+b[0])>>1, (a[1]+b[1])>>1, (a[2]+b[2])>>1, (a[3]+b[3])>>1);\
		*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge((a[0]+b[0]+f[0]+g[0])>>2, (a[1]+b[1]+f[1]+g[1])>>2, (a[2]+b[2]+f[2]+g[2])>>2, (a[3]+b[3]+f[3]+g[3])>>2);\
		pixel+=pitch; } \

#define X2Xy24xLerps(c0,c1,y)	\
			*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(						\
				cols[c0][y][0], cols[c0][y][1], cols[c0][y][2], cols[c0][y][3]);	\
			*(reinterpret_cast<uintX*>(pixel+sizeof(uintX))) = Manip::merge(		\
				(cols[c0][y][0]+cols[c1][y][0])>>1, 								\
				(cols[c0][y][1]+cols[c1][y][1])>>1, 								\
				(cols[c0][y][2]+cols[c1][y][2])>>1, 								\
				(cols[c0][y][3]+cols[c1][y][3])>>1);

#define X2xY24xInnerLoop(c0,c1) {			\
	X2Xy24xLerps(c0,c1,0); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,1); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,2); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,3); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,4); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,5); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,6); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,7); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,8); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,9); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,10); pixel+=pitch;	\
	X2Xy24xLerps(c0,c1,11); pixel+=pitch;	}

#define X2xY24xDoColsA() {	\
	CopyLerp(cols[0][0],a,b, 0x0000);	\
	CopyLerp(cols[0][1],a,b, 0x6AAA);	\
	CopyLerp(cols[0][2],a,b, 0xD554);	\
	CopyLerp(cols[0][3],b,c, 0x3FFE);	\
	CopyLerp(cols[0][4],b,c, 0xAAA8);	\
	CopyLerp(cols[0][5],c,d, 0x1552);	\
	CopyLerp(cols[0][6],c,d, 0x7FFC);	\
	CopyLerp(cols[0][7],c,d, 0xEAA6);	\
	CopyLerp(cols[0][8],d,e, 0x5550);	\
	CopyLerp(cols[0][9],d,e, 0xBFFA);	\
	CopyLerp(cols[0][10],e,l, 0x2AA4);	\
	CopyLerp(cols[0][11],e,l, 0x954E);	}

#define X2xY24xDoColsB() {	\
	CopyLerp(cols[1][0],f,g, 0x0000);	\
	CopyLerp(cols[1][1],f,g, 0x6AAA);	\
	CopyLerp(cols[1][2],f,g, 0xD554);	\
	CopyLerp(cols[1][3],g,h, 0x3FFE);	\
	CopyLerp(cols[1][4],g,h, 0xAAA8);	\
	CopyLerp(cols[1][5],h,i, 0x1552);	\
	CopyLerp(cols[1][6],h,i, 0x7FFC);	\
	CopyLerp(cols[1][7],h,i, 0xEAA6);	\
	CopyLerp(cols[1][8],i,j, 0x5550);	\
	CopyLerp(cols[1][9],i,j, 0xBFFA);	\
	CopyLerp(cols[1][10],j,k, 0x2AA4);	\
	CopyLerp(cols[1][11],j,k, 0x954E); }

#define X1xY12xCopy(y)	\
			*(reinterpret_cast<uintX*>(pixel)) = Manip::merge(						\
				cols[y][0], cols[y][1], cols[y][2], cols[y][3]);

#define X1xY12xInnerLoop() {		\
	X1xY12xCopy(0); pixel+=pitch;	\
	X1xY12xCopy(1); pixel+=pitch;	\
	X1xY12xCopy(2); pixel+=pitch;	\
	X1xY12xCopy(3); pixel+=pitch;	\
	X1xY12xCopy(4); pixel+=pitch;	\
	X1xY12xCopy(5); pixel+=pitch;	}

#define X1xY12xDoCols() {	\
	CopyLerp(cols[0],a,b, 0x0000);	\
	CopyLerp(cols[1],a,b, 0xD554);	\
	CopyLerp(cols[2],b,c, 0xAAA8);	\
	CopyLerp(cols[3],c,d, 0x7FFC);	\
	CopyLerp(cols[4],d,e, 0x5550);	\
	CopyLerp(cols[5],e,l, 0x2AA4);	}

#define ArbInnerLoop(a,b,f,g) {		\
	while (pos_y < end_y) {			\
		pos_x = dst_x;				\
		pixel = blockline_start;	\
		/* Dest Loop X */			\
		while (pos_x < end_x) {		\
			FilterPixel(a,b,f,g,(end_x-pos_x)>>8,(end_y-pos_y)>>8);\
			pixel+=sizeof(uintX);	\
			pos_x += add_x;			\
		}							\
		if (!next_block) next_block = pixel;	\
		blockline_start += pitch;	\
		pos_y += add_y;				\
	} end_y += 1 << 16; }


#define Read5(a,b,c,d,e) {	\
	Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
	Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
	Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
	Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
	Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]); }

#define Read5_Clipped(a,b,c,d,e) {	\
	Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
	Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
	Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
	Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
	e[0]=d[0]; e[1]=d[1], e[2]=d[2], e[3]=d[3]; }

#define Read6(a,b,c,d,e,l) {	\
	Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
	Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
	Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
	Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
	Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]);\
	Manip::split(*(texel+tpitch*5), l[0], l[1], l[2], l[3]); }

#define Read6_Clipped(a,b,c,d,e,l) {	\
	Manip::split(*(texel+tpitch*0), a[0], a[1], a[2], a[3]);\
	Manip::split(*(texel+tpitch*1), b[0], b[1], b[2], b[3]);\
	Manip::split(*(texel+tpitch*2), c[0], c[1], c[2], c[3]);\
	Manip::split(*(texel+tpitch*3), d[0], d[1], d[2], d[3]);\
	Manip::split(*(texel+tpitch*4), e[0], e[1], e[2], e[3]);\
	l[0]=e[0]; l[1]=e[1]; l[2]=e[2]; l[3]=e[3]; }


// Very very simple point scaler
template<class uintX, class Manip, class uintS=uintX> class BilinearScalerInternal
{
	
public:
	static bool Scale( Texture *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src)
	{
		// Must be multiples of 4!!!
		if ((sh&3) || (sw&3)) return false;

		//
		// We work like this:
		//
		// Read texels
		//
		// A F
		// B G
		// C H
		// D I
		// E J
		//
		// Interpolate for all dest pixels
		//
		// The read 5 texels for A B C D E
		// * F A 
		// * G B
		// * H C
		// * I D
		// * J E
		//
		// Interpolate for all dest pixels
		//
		// And so on
		//


		// 2x Scaling
		if ((sw*2 == dw) && (sh*2 == dh))
			return Scale2x(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
		// 2 X 2.4 Y
		else if ((sw*2 == dw) && (dh*5 == sh*12))
			return ScaleX2Y24(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
		// 1 X 1.2 Y 
		else if ((sw == dw) && (dh*5 == sh*6))
			return ScaleX1Y12(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
		// Arbitrary 
		else 
			return ScaleArb(tex,sx,sy,sw,sh,pixel,dw,dh,pitch,clamp_src);
	}

private:

	////////////////
	//            //
	// 2x Scaling //
	//            //
	////////////////
	static bool Scale2x(Texture *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src)
	{
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS*>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
		uintS *tline_end = texel + (sw-1);
		uintS *tex_end = texel + (sh-4)*tex->width;
		int tex_diff = (tex->width*4) - sw;

		uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4];
		int p_diff    = (pitch*8)-(dw*sizeof(uintX));

		bool clip_x = true;
		if (sw+sx < tex->width && clamp_src == false)
		{
			clip_x = false;
			tline_end = texel + (sw+1);
			tex_diff--;
		}

		bool clip_y = true;
		if (sh+sy < tex->height && clamp_src == false)
		{
			clip_y = false;
			tex_end = texel + (sh)*tex->width;
		}

		// Src Loop Y
		do {
			Read5(a,b,c,d,e);
			texel++;

			// Src Loop X
			do {
				Read5(f,g,h,i,j);
				texel++;

				ScalePixel2x(a, b, f, g);
				ScalePixel2x(b, c, g, h);
				ScalePixel2x(c, d, h, i);
				ScalePixel2x(d, e, i, j);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;

				Read5(a,b,c,d,e);
				texel++;

				ScalePixel2x(f, g, a, b);
				ScalePixel2x(g, h, b, c);
				ScalePixel2x(h, i, c, d);
				ScalePixel2x(i, j, d, e);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;

			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				Read5(f,g,h,i,j);
				texel++;

				ScalePixel2x(a, b, f, g);
				ScalePixel2x(b, c, g, h);
				ScalePixel2x(c, d, h, i);
				ScalePixel2x(d, e, i, j);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;

				ScalePixel2x(f, g, f, g);
				ScalePixel2x(g, h, g, h);
				ScalePixel2x(h, i, h, i);
				ScalePixel2x(i, j, i, j);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;
			};

			pixel  += p_diff;

			texel += tex_diff;
			tline_end += tpitch*4;
		} while (texel != tex_end);

		//
		// Final Rows - Clipping
		//

		// Src Loop Y
		if (clip_y) {
			Read5_Clipped(a,b,c,d,e);
			texel++;

			// Src Loop X
			do {
				Read5_Clipped(f,g,h,i,j);
				texel++;
				ScalePixel2x(a, b, f, g);
				ScalePixel2x(b, c, g, h);
				ScalePixel2x(c, d, h, i);
				ScalePixel2x(d, e, i, j);
				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;

				Read5_Clipped(a,b,c,d,e);
				texel++;
				ScalePixel2x(f, g, a, b);
				ScalePixel2x(g, h, b, c);
				ScalePixel2x(h, i, c, d);
				ScalePixel2x(i, j, d, e);
				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;
			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				Read5_Clipped(f,g,h,i,j);
				texel++;

				ScalePixel2x(a, b, f, g);
				ScalePixel2x(b, c, g, h);
				ScalePixel2x(c, d, h, i);
				ScalePixel2x(d, e, i, j);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;

				ScalePixel2x(f, g, f, g);
				ScalePixel2x(g, h, g, h);
				ScalePixel2x(h, i, h, i);
				ScalePixel2x(i, j, i, j);

				pixel-=pitch*8;
				pixel+=sizeof(uintX)*2;
			};

			pixel  += p_diff;

			texel += tex_diff;
			tline_end += tpitch*4;
		}

		return true;
	}

	////////////////
	//            //
	// 2 X 2.4 Y  //
	//            //
	////////////////
	static bool ScaleX2Y24(Texture *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src)
	{
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS*>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
		uintS *tline_end = texel + (sw-1);
		uintS *tex_end = texel + (sh-5)*tex->width;
		int tex_diff = (tex->width*5) - sw;

		uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4], k[4], l[4];
		uint8 cols[2][12][4];

		bool clip_x = true;
		if (sw+sx < tex->width && clamp_src == false)
		{
			clip_x = false;
			tline_end = texel + (sw+1);
			tex_diff--;
		}

		bool clip_y = true;
		if (sh+sy < tex->height && clamp_src == false)
		{
			clip_y = false;
			tex_end = texel + (sh)*tex->width;
		}


		// Src Loop Y
		do {
			Read6(a,b,c,d,e,l);
			texel++;

			X2xY24xDoColsA();

			// Src Loop X
			do {
				Read6(f,g,h,i,j,k);
				texel++;

				X2xY24xDoColsB();
				X2xY24xInnerLoop(0,1);
				pixel -= pitch*12-sizeof(uintX)*2;

				Read6(a,b,c,d,e,l);
				texel++;

				X2xY24xDoColsA();
				X2xY24xInnerLoop(1,0);
				pixel -= pitch*12-sizeof(uintX)*2;
			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				Read6(f,g,h,i,j,k);
				texel++;

				X2xY24xDoColsB();
				X2xY24xInnerLoop(0,1);
				pixel -= pitch*12-sizeof(uintX)*2;

				X2xY24xInnerLoop(1,1);
				pixel -= pitch*12-sizeof(uintX)*2;
			}

			pixel += pitch*12-sizeof(uintX)*(dw);
			texel += tex_diff;
			tline_end += tpitch*5;
		} while (texel != tex_end);


		//
		// Final Rows - Clipping
		//

		// Src Loop Y
		if (clip_y) {
			Read6_Clipped(a,b,c,d,e,l);
			texel++;

			X2xY24xDoColsA();

			// Src Loop X
			do {
				Read6_Clipped(f,g,h,i,j,k);
				texel++;

				X2xY24xDoColsB();
				X2xY24xInnerLoop(0,1);
				pixel -= pitch*12-sizeof(uintX)*2;

				Read6_Clipped(a,b,c,d,e,l);
				texel++;

				X2xY24xDoColsA();
				X2xY24xInnerLoop(1,0);
				pixel -= pitch*12-sizeof(uintX)*2;
			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				Read6_Clipped(f,g,h,i,j,k);
				texel++;

				X2xY24xDoColsB();

				X2xY24xInnerLoop(0,1);
				pixel -= pitch*12-sizeof(uintX)*2;

				X2xY24xInnerLoop(1,1);
				pixel -= pitch*12-sizeof(uintX)*2;
			};
		}

		return true;
	}


	////////////////
	//            //
	// 1 X 1.2 Y  //
	//            //
	////////////////
	static bool ScaleX1Y12(Texture *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src)
	{
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS*>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
		uintS *tline_end = texel + (sw);
		uintS *tex_end = texel + (sh-5)*tex->width;
		int tex_diff = (tex->width*5) - sw;

		uint8 a[4], b[4], c[4], d[4], e[4], l[4];
		uint8 cols[6][4];

		bool clip_y = true;
		if (sh+sy < tex->height && clamp_src == false)
		{
			clip_y = false;
			tex_end = texel + (sh)*tex->width;
		}

		// Src Loop Y
		do {
			// Src Loop X
			do {
				Read6(a,b,c,d,e,l);
				texel++;

				X1xY12xDoCols();
				X1xY12xInnerLoop();
				pixel -= pitch*6-sizeof(uintX);

			} while (texel != tline_end);

			pixel += pitch*6-sizeof(uintX)*(dw);
			texel += tex_diff;
			tline_end += tpitch*5;

		} while (texel != tex_end);


		//
		// Final Rows - Clipping
		//

		// Src Loop Y
		if (clip_y) {
			// Src Loop X
			do {
				Read6_Clipped(a,b,c,d,e,l);
				texel++;

				X1xY12xDoCols();
				X1xY12xInnerLoop();
				pixel -= pitch*6-sizeof(uintX);

			} while (texel != tline_end);
		}

		return true;
	}


	///////////////////////
	//                   //
	// Arbitrary Scaling //
	//                   //
	///////////////////////
	static bool ScaleArb(Texture *tex, sint32 sx, sint32 sy, sint32 sw, sint32 sh, 
						uint8* pixel, sint32 dw, sint32 dh, sint32 pitch, bool clamp_src)
	{
		// Source buffer pointers
		uintS *texel = reinterpret_cast<uintS*>(tex->buffer) + (sy * tex->width + sx);
		int tpitch = tex->width;
		uintS *tline_end = texel + (sw-1);
		uintS *tex_end = texel + (sh-4)*tex->width;
		int tex_diff = (tex->width*4) - sw;

		uint8 a[4], b[4], c[4], d[4], e[4], f[4], g[4], h[4], i[4], j[4];

		uint32 pos_y, pos_x;

		uint32 add_y = (sh<<16)/dh;
		uint32 end_y = 1<<16;
		uint32 dst_y = 0;

		uint8* blockline_start = 0;
		uint8* next_block = 0;

		bool clip_x = true;
		if (sw+sx < tex->width && clamp_src == false)
		{
			clip_x = false;
			tline_end = texel + (sw+1);
			tex_diff--;
		}

		bool clip_y = true;
		if (sh+sy < tex->height && clamp_src == false)
		{
			clip_y = false;
			tex_end = texel + (sh)*tex->width;
		}

		// Src Loop Y
		do {
			Read5(a,b,c,d,e);
			texel++;

			uint32 add_x = (sw<<16)/dw;
			uint32 end_x = 1<<16;
			uint32 dst_x = 0;

			next_block = pixel;

			// Src Loop X
			do {
				pos_y = dst_y;

				Read5(f,g,h,i,j);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(a, b, f, g);
				ArbInnerLoop(b, c, g, h);
				ArbInnerLoop(c, d, h, i);
				ArbInnerLoop(d, e, i, j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
				pos_y = dst_y;

				Read5(a,b,c,d,e);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(f,g,a,b);
				ArbInnerLoop(g,h,b,c);
				ArbInnerLoop(h,i,c,d);
				ArbInnerLoop(i,j,d,e);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				pos_y = dst_y;

				Read5(f,g,h,i,j);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(a, b, f, g);
				ArbInnerLoop(b, c, g, h);
				ArbInnerLoop(c, d, h, i);
				ArbInnerLoop(d, e, i, j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
				pos_y = dst_y;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(f,g,f,g);
				ArbInnerLoop(g,h,g,h);
				ArbInnerLoop(h,i,h,i);
				ArbInnerLoop(i,j,i,j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
			};

			pixel += pitch - sizeof(uintX)*(dw);

			dst_y = pos_y;
			end_y += 4<<16;

			texel += tex_diff;
			tline_end += tpitch*4;
		} while (texel != tex_end);


		//
		// Final Rows - Clipping
		//

		// Src Loop Y
		if (clip_y) {
			Read5_Clipped(a,b,c,d,e);
			texel++;

			uint32 add_x = (sw<<16)/dw;
			uint32 end_x = 1<<16;
			uint32 dst_x = 0;

			next_block = pixel;

			// Src Loop X
			do {
				pos_y = dst_y;

				Read5_Clipped(f,g,h,i,j);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(a, b, f, g);
				ArbInnerLoop(b, c, g, h);
				ArbInnerLoop(c, d, h, i);
				ArbInnerLoop(d, e, i, j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
				pos_y = dst_y;

				Read5_Clipped(a,b,c,d,e);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(f,g,a,b);
				ArbInnerLoop(g,h,b,c);
				ArbInnerLoop(h,i,c,d);
				ArbInnerLoop(i,j,d,e);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
			} while (texel != tline_end);

			// Final X (clipping)
			if (clip_x) {
				pos_y = dst_y;

				Read5_Clipped(f,g,h,i,j);
				texel++;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(a, b, f, g);
				ArbInnerLoop(b, c, g, h);
				ArbInnerLoop(c, d, h, i);
				ArbInnerLoop(d, e, i, j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
				pos_y = dst_y;

				blockline_start = next_block;
				next_block = 0;

				ArbInnerLoop(f,g,f,g);
				ArbInnerLoop(g,h,g,h);
				ArbInnerLoop(h,i,h,i);
				ArbInnerLoop(i,j,i,j);

				end_y -= 4<<16;
				dst_x = pos_x;
				end_x += 1<<16;
			};
		}


		return true;
	}

};


namespace Pentagram {

BilinearScaler::BilinearScaler() : Scaler()
{
	Scale16Nat = BilinearScalerInternal<uint16, Manip_Nat2Nat<uint16>, uint16>::Scale;
	Scale16Sta = BilinearScalerInternal<uint16, Manip_Sta2Nat<uint16>, uint32>::Scale;

	Scale32Nat = BilinearScalerInternal<uint32, Manip_Nat2Nat<uint32>, uint16>::Scale;
	Scale32Sta = BilinearScalerInternal<uint32, Manip_Sta2Nat<uint32>, uint32>::Scale;
	Scale32_A888 = BilinearScalerInternal<uint32, Manip_32_A888, uint32>::Scale;
	Scale32_888A = BilinearScalerInternal<uint32, Manip_32_888A, uint32>::Scale;
}

const uint32 BilinearScaler::ScaleBits() { return 0xFFFFFFFF; }
const bool BilinearScaler::ScaleArbitrary() { return true; }

const char *BilinearScaler::ScalerName() { return "bilinear"; }
const char *BilinearScaler::ScalerDesc() { return "Bilinear Filtering Scaler"; }
const char *BilinearScaler::ScalerCopyright() { return "Copyright (c) 2005 The Pentagram Team"; }

BilinearScaler bilinear_scaler;

};
