/*
 *  Copyright (C) 2005  The Pentagram Team
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

#ifndef FASTAREAVISGUMP_H_INCLUDED
#define FASTAREAVISGUMP_H_INCLUDED

#include "Gump.h"

class FastAreaVisGump : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	FastAreaVisGump(void);
	virtual ~FastAreaVisGump(void);

	virtual void		PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled);
	virtual uint16		TraceObjId(int mx, int my);

	static void			ConCmd_toggle(const Console::ArgvType &argv);	//!< "FastAreaVisGump::toggle" console command

};

#endif // FASTAREAVISGUMP_H_INCLUDED
