/*
Copyright (C) 2002-2003 The Pentagram team

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

#include "UsecodeFlex.h"
#include <cstring>

const uint8* UsecodeFlex::get_class(uint32 classid)
{
	const uint8* obj = get_object_nodel(classid+2);
	if (obj) {
		// 0x0C = header size. Clean up.
		return obj + 0x0C;
	} else {
		return 0;
	}
}

uint32 UsecodeFlex::get_class_size(uint32 classid)
{
	// 0x0C = header size. Clean up.
	uint32 size = get_size(classid+2);
	if (size >= 0x0C)
		return size - 0x0C;
	else
		return 0;
}

const char* UsecodeFlex::get_class_name(uint32 classid)
{
	if (get_size(classid) > 0) {
		const uint8* name_object = get_object_nodel(1);
		return reinterpret_cast<const char*>(name_object+4+(13 * classid));
	} else {
		return 0;
	}
}

