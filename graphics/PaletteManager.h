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

#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include "Palette.h"
#include <vector>

class IDataSource;
class RenderSurface;

class PaletteManager
{
public:
	explicit PaletteManager(RenderSurface* rs);
	~PaletteManager();

	static PaletteManager* get_instance() { return palettemanager; }

	enum PalIndex {
		Pal_Game = 0,
		Pal_Movie = 1
	};

	void load(PalIndex index, IDataSource& ds, IDataSource &xformds);
	void load(PalIndex index, IDataSource& ds);
	Pentagram::Palette* getPalette(PalIndex index);

	// Apply a transform matrix to a palette (-4.11 fixed)
	void transformPalette(PalIndex index, sint16 matrix[12]);

	// Get a TransformMatrix from a PalTransforms value (-4.11 fixed)
	static void getTransformMatrix(sint16 matrix[12], Pentagram::PalTransforms trans);

	// Create a custom Transform Matrix from RGBA col32. (-4.11 fixed)
	// Alpha will set how much of original palette to keep. 0 = keep none
	static void getTransformMatrix(sint16 matrix[12], uint32 col32);	

private:
	std::vector<Pentagram::Palette*> palettes;
	RenderSurface *rendersurface;

	static PaletteManager* palettemanager;
};

#endif
