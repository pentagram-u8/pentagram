/*
 *  Copyright (C) 2002  Ryan Nunn and The Pentagram Team
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

#ifndef TEXTURE_H
#define TEXTURE_H

#include "databuf.h"

//
// Basic 32 Bit Texture
//
struct Texture
{
	uint32			*buffer;
	uint32			width;
	uint32			height;

	// Use CalcLOG2s to calculate these
	uint32			wlog2;
	uint32			hlog2;

	// For OpenGL
	uint32			gl_tex;
	Texture			*next;

	Texture() : buffer(0), gl_tex(0), next(0)
	{
	}

	virtual ~Texture();

	// Clear all texture data
	virtual bool Clear();

	// Calc texture log2's 
	void CalcLOG2s() {
		for (int i = 0; i < 32; i++) {
			if (width & (1 << i))
				wlog2 = i;

			if (height & (1 << i))
				hlog2 = i;
		}
	}

	// Create a texture from a Data Source (filename is use to help detection of type)
	static Texture * Create(DataSource &ds, const char *filename = NULL);

protected:

	// Read from a File. No filetype supported by this class
	virtual bool Read(DataSource &ds) { return false; }
};

#endif //TEXTURE_H
