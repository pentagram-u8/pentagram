/*
Copyright (C) 2003 The Pentagram team

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

#include "PaletteManager.h"
#include "Palette.h"
#include "IDataSource.h"
#include "RenderSurface.h"

PaletteManager::PaletteManager(RenderSurface *rs)
	: rendersurface(rs)
{

}

PaletteManager::~PaletteManager()
{
	for (unsigned int i = 0; i < palettes.size(); ++i)
		delete palettes[i];
	palettes.clear();
}

void PaletteManager::load(PalIndex index, IDataSource& ds, const xformBlendFuncType *xff)
{
	if (palettes.size() <= static_cast<unsigned int>(index))
		palettes.resize(index+1);

	if (palettes[index])
		delete palettes[index];

	Palette* pal = new Palette;
	pal->load(ds,xff);
	rendersurface->CreateNativePalette(pal); // convert to native format

	palettes[index] = pal;
}

Palette* PaletteManager::getPalette(PalIndex index)
{
	if (static_cast<unsigned int>(index) > palettes.size())
		return 0;

	return palettes[index];
}
