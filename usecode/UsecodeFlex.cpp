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

#include "pent_include.h"

#include "UsecodeFlex.h"
#include "CoreApp.h"
#include "GameInfo.h"

const uint8* UsecodeFlex::get_class(uint32 classid)
{
	const uint8* obj = get_object_nodel(classid+2);
	return obj;
}

uint32 UsecodeFlex::get_class_size(uint32 classid)
{
	uint32 size = get_size(classid+2);
	return size;
}

const char* UsecodeFlex::get_class_name(uint32 classid)
{
	if (get_size(classid+2) > 0) {
		const uint8* name_object = get_object_nodel(1);
		return reinterpret_cast<const char*>(name_object+4+(13 * classid));
	} else {
		return 0;
	}
}

uint32 UsecodeFlex::get_class_base_offset(uint32 classid)
{
	if (get_size(classid+2) == 0) return 0;

	if (GAME_IS_U8) {
		return 0x0C;
	} else if (GAME_IS_CRUSADER) {
		const uint8* obj = get_object_nodel(classid+2);
		uint32 offset = obj[8];
		offset += obj[9] << 8;
		offset += obj[10] << 16;
		offset += obj[11] << 24;
		offset--;
		return offset;
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
		return 0;
	}
}

uint32 UsecodeFlex::get_class_event_count(uint32 classid)
{
	if (get_size(classid+2) == 0) return 0;

	if (GAME_IS_U8) {
		return 32;
	} else if (GAME_IS_CRUSADER) {
		return (get_class_base_offset(classid)+19)/6;
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
		return 0;
	}
}
