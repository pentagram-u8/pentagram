/*
Copyright (C) 2003-2004 The Pentagram Team

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

#ifndef PALETTE_H
#define PALETTE_H

#include "PaletteManager.h"

class IDataSource;

namespace Pentagram
{

struct Palette
{
	void load(IDataSource& ds, IDataSource& xformds);
	void load(IDataSource& ds);

	// 256 rgb entries
	uint8 palette[768];

	// Untransformed native format palette. Created by the RenderSurface
	uint32 native_untransformed[256];

	// Transformed native format palette. Created by the RenderSurface
	uint32 native[256];

	// Untransformed XFORM ARGB palette
	uint32 xform_untransformed[256];

	// Transformed XFORM ARGB palette. Created by the RenderSurface
	uint32 xform[256];

	// Colour transformation matrix (for fades, hue shifts)
	// Applied by the RenderSurface (fixed -4.11)
	// R = R*matrix[0] + G*matrix[1] + B*matrix[2]  + matrix[3];
	// G = R*matrix[4] + G*matrix[5] + B*matrix[6]  + matrix[7];
	// B = R*matrix[8] + G*matrix[9] + B*matrix[10] + matrix[11];
	// A = A;
	sint16 matrix[12];

	// The current palette transform
	PaletteManager::PalTransforms transform;
};

}

#endif
