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

#ifndef PALETTE_H
#define PALETTE_H

#include "XFormBlend.h"

class IDataSource;

struct Palette
{
	void load(IDataSource& ds, const xformBlendFuncType *xff);

	// 256 rgb entries
	uint8 palette[768];

	// native format palette. Created by the RenderSurface
	uint32 native[256];

	// xform functions
	const xformBlendFuncType	*xform_funcs;
};



#endif
