/*
 *  Copyright (C) 2002 The Pentagram Team
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

#ifndef CONVERT_H
#define CONVERT_H

#include "IDataSource.h"
#include "common_types.h"

class ConvertUsecode
{
	// just an 'empty' base class
	public:
		virtual const char* const *intrinsics()=0;
		virtual const char* const *event_names()=0;
		virtual void readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)=0;
		virtual void readevents(IFileDataSource *ucfile)=0;
};

#endif
