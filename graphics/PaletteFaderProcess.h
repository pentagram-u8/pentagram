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

#ifndef PALETTEFADERPROCESS_H_INCLUDED
#define PALETTEFADERPROCESS_H_INCLUDED

#include "Process.h"
#include "PaletteManager.h"
#include "intrinsics.h"

class PaletteFaderProcess : public Process
{
	int							priority;
	int							counter;
	int							max_counter;
	float						old_matrix[12];
	float						new_matrix[12];
public:
	static PaletteFaderProcess	*fader;

	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();
	PaletteFaderProcess(PaletteManager::PalTransforms trans, int priority, int frames);
	PaletteFaderProcess(uint32 rgba, bool from, int priority, int frames);
	PaletteFaderProcess(float from[12], float to[12], int priority, int frames);
	virtual ~PaletteFaderProcess(void);

	virtual bool run(const uint32 framenum);

	INTRINSIC(I_fadeToPaletteTransform);
	INTRINSIC(I_fadeToBlack);
	INTRINSIC(I_fadeFromWhite);
	INTRINSIC(I_fadeToWhite);
	INTRINSIC(I_fadeFromBlack);
	INTRINSIC(I_lightningBolt);
};


#endif //PALETTEFADERPROCESS_H_INCLUDED
