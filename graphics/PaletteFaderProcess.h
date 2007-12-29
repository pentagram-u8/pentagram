/*
 *  Copyright (C) 2003-2004  Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
	sint32						counter;
	sint32						max_counter;
	sint16						old_matrix[12];	// Fixed point -4.11
	sint16						new_matrix[12];
public:
	static PaletteFaderProcess	*fader;

	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();
	PaletteFaderProcess();
	PaletteFaderProcess(Pentagram::PalTransforms trans, int priority, int frames);
	PaletteFaderProcess(uint32 rgba, bool from, int priority, int frames, bool current);
	PaletteFaderProcess(sint16 from[12], sint16 to[12], int priority, int frames);
	virtual ~PaletteFaderProcess(void);

	virtual void run();

	INTRINSIC(I_fadeToPaletteTransform);
	INTRINSIC(I_fadeToBlack);
	INTRINSIC(I_fadeFromWhite);
	INTRINSIC(I_fadeToWhite);
	INTRINSIC(I_fadeFromBlack);
	INTRINSIC(I_lightningBolt);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};


#endif //PALETTEFADERPROCESS_H_INCLUDED
