/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// istring.cpp -- case insensitive stl strings

#include "pent_include.h"
#include "istring.h"
#include "Q_strcasecmp.h"

#ifndef UNDER_CE

namespace Pentagram {

bool ichar_traits::eq(const char_type & c1, const char_type & c2)
{
	uint32 c3 = c1;
	uint32 c4 = c2;
	if (c3 >= 'a' && c3 <= 'z')
		c3 -= ('a' - 'A');
	if (c4 >= 'a' && c4 <= 'z')
		c4 -= ('a' - 'A');
	return c3 == c4;
}

bool ichar_traits::lt(const char_type & c1, const char_type & c2)
{
	uint32 c3 = c1;
	uint32 c4 = c2;
	if (c3 >= 'a' && c3 <= 'z')
		c3 -= ('a' - 'A');
	if (c4 >= 'a' && c4 <= 'z')
		c4 -= ('a' - 'A');
	return c3 < c4;		
}

int ichar_traits::compare(const char_type * s1, const char_type * s2, size_t length)
{
	return Q_strncasecmp(s1, s2, length);
}

};

#endif
