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
#include "Texture.h"

PaletteManager* PaletteManager::palettemanager = 0;

PaletteManager::PaletteManager(RenderSurface *rs)
	: rendersurface(rs)
{
	assert(palettemanager == 0);
	palettemanager = this;
}

PaletteManager::~PaletteManager()
{
	for (unsigned int i = 0; i < palettes.size(); ++i)
		delete palettes[i];
	palettes.clear();

	palettemanager = 0;
}

void PaletteManager::load(PalIndex index, IDataSource& ds, IDataSource &xformds)
{
	if (palettes.size() <= static_cast<unsigned int>(index))
		palettes.resize(index+1);

	if (palettes[index])
		delete palettes[index];

	Palette* pal = new Palette;
	pal->load(ds,xformds);
	rendersurface->CreateNativePalette(pal); // convert to native format

	palettes[index] = pal;
}

Palette* PaletteManager::getPalette(PalIndex index)
{
	if (static_cast<unsigned int>(index) > palettes.size())
		return 0;

	return palettes[index];
}

void PaletteManager::transformPalette(PalIndex index, float matrix[12])
{
	Palette *pal = getPalette(index);

	if (!pal) return;

	for (int i = 0; i < 12; i++) pal->matrix[i] = matrix[i];
	rendersurface->CreateNativePalette(pal); // convert to native format
}

void PaletteManager::getTransformMatrix(float matrix[12], PalTransforms trans)
{
	switch (trans)
	{
		// Normal untransformed palette
	case Transform_None:
		{
			matrix[0] = 1;	matrix[1] = 0;	matrix[2]  = 0;	matrix[3]  = 0;
			matrix[4] = 0;	matrix[5] = 1;	matrix[6]  = 0;	matrix[7]  = 0;
			matrix[8] = 0;	matrix[9] = 0;	matrix[10] = 1;	matrix[11] = 0;
		}
		break;

		// O[i] = I[r]*0.375 + I[g]*0.5 + I[b]*0.125;
	case Transform_Greyscale:
		{
			for (int i = 0; i < 3; i++)
			{
				matrix[i*4+0] = 0.375F;	
				matrix[i*4+1] = 0.5F;	
				matrix[i*4+2] = 0.125F;	
				matrix[i*4+3] = 0;
			}
		}
		break;

		// O[r] = 0;
	case Transform_NoRed:
		{
			matrix[0] = 0;	matrix[1] = 0;	matrix[2]  = 0;	matrix[3]  = 0;
			matrix[4] = 0;	matrix[5] = 1;	matrix[6]  = 0;	matrix[7]  = 0;
			matrix[8] = 0;	matrix[9] = 0;	matrix[10] = 1;	matrix[11] = 0;
		}
		break;

		// O[i] = (I[i] + Grey)*0.25 + 0.1875;
	case Transform_RainStorm:
		{
			for (int i = 0; i < 3; i++)
			{
				matrix[i*4+0] = 0.375F*0.25F;
				matrix[i*4+1] = 0.5F  *0.25F;	
				matrix[i*4+2] = 0.125F*0.25F;	

				matrix[i*4+i]+= 0.25;

				matrix[i*4+3] = 0.1875F;
			}
		}
		break;

		// O[r] = (I[r] + Grey)*0.5 + 0.1875; 
		// O[g] = I[g]*0.5 + Grey*0.25; 
		// O[b] = I[b]*0.5;
	case Transform_FireStorm:
		{
			// O[r] = (I[r] + Grey)*0.5 + 0.1875; 
			matrix[0]  = 0.375F*0.5F + 0.5F;	
			matrix[1]  = 0.5F  *0.5F;	
			matrix[2]  = 0.125F*0.5F;	
			matrix[3]  = 0.1875F;

			// O[g] = I[g]*0.5 + Grey*0.25; 
			matrix[4]  = 0.375F*0.25F;	
			matrix[5]  = 0.5F  *0.25F + 0.5F;	
			matrix[6]  = 0.125F*0.25F;	
			matrix[7]  = 0;

			// O[b] = I[b]*0.5;
			matrix[8]  = 0;	
			matrix[9]  = 0;	
			matrix[10] = 0.5F;	
			matrix[11] = 0;
		}
		break;

		// O[i] = I[i]*2 -Grey;
	case Transform_Saturate:
		{
			for (int i = 0; i < 3; i++)
			{
				matrix[i*4+0] = -0.375F;	
				matrix[i*4+1] = -0.5F;	
				matrix[i*4+2] = -0.125F;	
				matrix[i*4+3] = 0;
				matrix[i*4+i] += 2;	
			}
		}
		break;

		// O[b] = I[r]; O[r] = I[g]; O[g] = I[b];
	case Transform_BRG:
		{
			matrix[0] = 0;	matrix[1] = 1;	matrix[2]  = 0;	matrix[3]  = 0;
			matrix[4] = 0;	matrix[5] = 0;	matrix[6]  = 1;	matrix[7]  = 0;
			matrix[8] = 1;	matrix[9] = 0;	matrix[10] = 0;	matrix[11] = 0;
		}
		break;

		// O[g] = I[r]; O[b] = I[g]; O[r] = I[b];
	case Transform_GBR:
		{
			matrix[0] = 0;	matrix[1] = 0;	matrix[2]  = 1;	matrix[3]  = 0;
			matrix[4] = 1;	matrix[5] = 0;	matrix[6]  = 0;	matrix[7]  = 0;
			matrix[8] = 0;	matrix[9] = 1;	matrix[10] = 0;	matrix[11] = 0;
		}
		break;

		// Unknown
	default:
		{
			perr << "Unknown Palette Transformation: " << trans << std::endl;
			matrix[0] = 1;	matrix[1] = 0;	matrix[2]  = 0;	matrix[3]  = 0;
			matrix[4] = 0;	matrix[5] = 1;	matrix[6]  = 0;	matrix[7]  = 0;
			matrix[8] = 0;	matrix[9] = 0;	matrix[10] = 1;	matrix[11] = 0;
		}
		break;
	}
}


void PaletteManager::getTransformMatrix(float matrix[12], uint32 col32)
{
	matrix[0]  = TEX32_A(col32)/255.0F;	
	matrix[1]  = 0;	
	matrix[2]  = 0;	
	matrix[3]  = TEX32_R(col32)/255.0F;

	matrix[4]  = 0;	
	matrix[5]  = TEX32_A(col32)/255.0F;	
	matrix[6]  = 0;	
	matrix[7]  = TEX32_G(col32)/255.0F;

	matrix[8]  = 0;	
	matrix[9]  = 0;	
	matrix[10] = TEX32_A(col32)/255.0F;	
	matrix[11] = TEX32_B(col32)/255.0F;
}
