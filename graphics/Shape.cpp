/*
 *  Copyright (C) 2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "Shape.h"
#include "ShapeFrame.h"

Shape::Shape(const uint8* data)
{
	// NB: U8 style!

	this->data = data;
	this->size = size;
	this->palette = 0;

	unsigned int framecount = data[4] + (data[5]<<8);

	frames.reserve(framecount);

	for (unsigned int i = 0; i < framecount; ++i) {
		uint32 frameoffset = data[6+6*i] + (data[7+6*i]<<8)+(data[8+6*i]<<16);
		uint32 framesize = data[10+6*i] + (data[11+6*i]<<8);
		
		frames.push_back(new ShapeFrame(data + frameoffset, framesize));
	}
}

Shape::~Shape()
{
	delete[] data;
}
