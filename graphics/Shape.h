/*
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

#ifndef SHAPE_H
#define SHAPE_H

#include <vector>

class ShapeFrame;
struct Palette;

class Shape 
{
 public:
	// Parse data, create frames.
	// NB: Shape uses data without copying it. It is deleted on destruction
	Shape(const uint8* data);
	~Shape();
	void setPalette(const Palette* pal) { palette = pal; }
	const Palette* getPalette() const { return palette; }

	unsigned int frameCount() const { return frames.size(); }

	ShapeFrame* getFrame(unsigned int frame) { return frames[frame]; }

 protected:
	std::vector<ShapeFrame*> frames;

	const Palette* palette;

	const uint8* data;
	uint32 size;
};


#endif
