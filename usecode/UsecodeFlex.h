/*
Copyright (C) 2002-2006 The Pentagram team

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

#ifndef USECODEFLEX_H
#define USECODEFLEX_H

#include "Usecode.h"
#include "RawArchive.h"

// multiple inheritance. um, yes :-)
class UsecodeFlex : public Usecode, protected RawArchive {
 public:
	UsecodeFlex(IDataSource* ds) : RawArchive(ds) { }
	virtual ~UsecodeFlex() { }

	virtual const uint8* get_class(uint32 classid);
	virtual uint32 get_class_size(uint32 classid);
	virtual const char* get_class_name(uint32 classid);
	virtual uint32 get_class_base_offset(uint32 classid);
	virtual uint32 get_class_event_count(uint32 classid);
};

#endif
