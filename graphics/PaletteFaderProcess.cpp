/*
 *  Copyright (C) 2003  Pentagram Team
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
#include "PaletteFaderProcess.h"
#include "Kernel.h"
#include "Palette.h"

#define PALETTEFADER_COUNTER	30

PaletteFaderProcess *PaletteFaderProcess::fader = 0;

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(PaletteFaderProcess,Process)

PaletteFaderProcess::PaletteFaderProcess(PaletteManager::PalTransforms trans,
		int priority_, int frames) : priority(priority_), 
		counter(frames), max_counter(frames)
{
	PaletteManager	*pm = PaletteManager::get_instance();
	Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) old_matrix[i] = pal->matrix[i];
	pm->getTransformMatrix(new_matrix,trans);
}

PaletteFaderProcess::PaletteFaderProcess(uint32 col32, bool from,
		int priority_, int frames) : priority(priority_), 
		counter(frames), max_counter(frames)
{
	PaletteManager	*pm = PaletteManager::get_instance();
	if (!from)
	{
	    Palette *pal = pm->getPalette(PaletteManager::Pal_Game);
		for (int i = 0; i < 12; i++) old_matrix[i] = pal->matrix[i];
		pm->getTransformMatrix(new_matrix,col32);
	}
	else
	{
		pm->getTransformMatrix(old_matrix,col32);
		pm->getTransformMatrix(new_matrix,PaletteManager::Transform_None);
	}
}

PaletteFaderProcess::PaletteFaderProcess(float from[12], float to[12],
		int priority_, int frames) : priority(priority_), 
		counter(frames), max_counter(frames)
{
	for (int i = 0; i < 12; i++) old_matrix[i] = from[i];
	for (int i = 0; i < 12; i++) new_matrix[i] = to[i];
}

PaletteFaderProcess::~PaletteFaderProcess(void)
{
	fader = 0;
}

bool PaletteFaderProcess::run(const uint32)
{
	float	matrix[12];
	float	factor;
	
	factor = static_cast<float>(counter)/static_cast<float>(max_counter);

	for (int i = 0; i < 12; i++)
		matrix[i] = old_matrix[i]*factor + new_matrix[i]*(1-factor);

	PaletteManager::get_instance()->transformPalette(
			PaletteManager::Pal_Game, 
			matrix);
	
	if (!counter--) terminate();
	return true;
}

uint32 PaletteFaderProcess::I_fadeToPaletteTransform(const uint8* args,
										unsigned int /*argsize*/)
{
	ARG_UINT16(transform);
	ARG_UINT16(priority);	
	
	// If current fader has higher priority, we do nothing
	if (fader && fader->priority > priority) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(static_cast<PaletteManager::PalTransforms>(transform),
				priority, 30);

	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeToBlack(const uint8* args,
										unsigned int /*argsize*/)
{
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(0x00000000, false, 0x7FFF, 30);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeFromBlack(const uint8* args,
										unsigned int /*argsize*/)
{
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(0x00000000, true, 0x7FFF, 30);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeToWhite(const uint8* args,
										unsigned int /*argsize*/)
{
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(0x00FFFFFF, false, 0x7FFF, 30);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_fadeFromWhite(const uint8* args,
										unsigned int /*argsize*/)
{
	if (fader && fader->priority > 0x7FFF) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(0x00FFFFFF, true, 0x7FFF, 30);
	return Kernel::get_instance()->addProcess(fader);
}

uint32 PaletteFaderProcess::I_lightningBolt(const uint8* args,
										unsigned int /*argsize*/)
{
	if (fader && fader->priority > 0xFFFF) return 0;
	else if (fader) delete fader;

	fader = new PaletteFaderProcess(0x3FCFCFCF, true, 0xFFFF, 10);
	return Kernel::get_instance()->addProcess(fader);
}

