/*
Copyright (C) 2002 The Pentagram team

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

#include "util.h"

std::string to_uppercase(std::string s)
{
	std::string str = s;
	std::string::iterator X;
	for(X = str.begin(); X != str.end(); ++X) {
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = static_cast<char>(std::toupper(*X));
#endif
	}
	return str;
}
