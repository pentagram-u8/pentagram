/*
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

#ifndef RECT_H_INCLUDED
#define RECT_H_INCLUDED

struct Rect {
	// DO NOT Manually Set these!
	sint32		x, y;
	sint32		w, h;
	sint32		xr, yb;

	Rect() : x(0), y(0), w(0), h(0), xr(0), yb(0) {}
	Rect(int nx, int ny, int nw, int nh) : x(nx), y(ny), w(nw), h(nh), xr(nx+nw), yb(ny+nh) {}
	Rect(const Rect& o) : x(o.x), y(o.y), w(o.w), h(o.h), xr(o.x+o.w), yb(o.y+o.h) {}
	
	void	Set(int nx, int ny, int nw, int nh) { x=nx; y=ny; w=nw; h=nh; xr=x+w; yb=y+h; }
	void	Set(Rect &o) { *this = o; }

	// Check to see if a Rectangle is 'valid'
	bool	IsValid() { return w > 0 && h > 0; }

	// Move the Rect (Relative)
	void	MoveRel(sint32 dx, sint32 dy) { x=x+dx; y=y+dy; xr=x+w; yb=y+h; }

	// Move the Rect (Absolute)
	void	MoveAbs(sint32 nx, sint32 ny) { x=nx; y=ny; xr=x+w; yb=y+h; }

	// Resize the Rect (Relative)
	void	ResizeRel(sint32 dw, sint32 dh) { w=w+dw; h=h+dh; xr=x+w; yb=y+h; }

	// Resize the Rect (Absolute)
	void	ResizeAbs(sint32 nw, sint32 nh) { w=nw; h=nh; xr=x+w; yb=y+h; }

	// Intersect/Clip this rect with another
	void	Intersect(int ox, int oy, int ow, int oh)
	{
		int x2 = x + w,		y2 = y + h;
		int ox2 = ox + ow,	oy2 = oy + oh;

		if (x < ox) x = ox;
		else if (x > ox2) x = ox2;

		if (x2 < ox) x2 = ox;
		else if (x2 > ox2) x2 = ox2;

		if (y < oy) y = oy;
		else if (y > oy2) y = oy2;

		if (y2 < oy) y2 = oy;
		else if (y2 > oy2) y2 = oy2;

		xr = x2;
		yb = y2;
		w = x2 - x;
		h = y2 - y;

	}

	// Intersect/Clip this another with this
	void	IntersectOther(int &ox, int &oy, int &ow, int &oh) const
	{
		int x2 = x + w,		y2 = y + h;
		int ox2 = ox + ow,	oy2 = oy + oh;

		if (ox < x) ox = x;
		else if (ox > x2) ox = x2;

		if (ox2 < x) ox2 = x;
		else if (ox2 > x2) ox2 = x2;

		if (oy < y) oy = y;
		else if (oy > y2) oy = y2;

		if (oy2 < y) oy2 = y;
		else if (oy2 > y2) oy2 = y2;

		ow = ox2 - ox;
		oh = oy2 - oy;
	}

	// Intersect/Clip this rect with another
	void	Intersect(const Rect& o)
	{
		Intersect(o.x, o.y, o.w, o.h);
	}

	// Union/Add this rect with another
	void	Union(int ox, int oy, int ow, int oh)
	{
		int x2 = x + w,		y2 = y + h;
		int ox2 = ox + ow,	oy2 = oy + oh;

		if (ox < x) x = ox;
		else if (ox2 > x2) x2= ox2;

		if (oy < y) y = ox;
		else if (oy2 > y2) y2 = ox2;

		xr = x2;
		yb = y2;
		w = x2 - x;
		h = y2 - y;
	}

	// Union/Add this rect with another
	void	Union(const Rect& o)
	{
		Union(o.x, o.y, o.w, o.h);
	}

	// Operator +=
	Rect &operator += (const Rect& o)
	{
		Union(o.x, o.y, o.w, o.h);
		return *(this);
	}

	// Operator +
	Rect &operator + (const Rect& o) const
	{
		Rect result(*this);
		return (result+=o);
	}

	bool operator == (const Rect& o) const
	{
		return x == o.x && y == o.y && w == o.w && h == o.h;
	}

};

#endif // RECT_H_INCLUDED
