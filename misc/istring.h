/*
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

// istring.h -- case insensitive stl strings

#ifndef ISTRING_H
#define	ISTRING_H

#include <string>

namespace Pentagram {

//! STL char_traits for case insensitive comparisons
struct ichar_traits : public std::char_traits<char>
{
	//! Case insensitive check for 2 chars being equal
	static bool eq(const char_type & c1, const char_type & c2);

	//! Case insensitive check for the first char being less
	//! than the second char
	static bool lt(const char_type & c1, const char_type & c2);

	//! Case insensitive comparison for 2 c_strings - based off Q_strncasecmp
	static int compare(const char_type * s1, const char_type * s2, size_t length);
};

//! STL case insensitve string
typedef std::basic_string<char, ichar_traits> istring;
};

#endif 

