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

template<class T> void StringToArgv(const T &args, std::vector<T> &argv)
{
	// Clear the vector
	argv.clear();

	bool quoted = false;
	typename T::const_iterator it;
	int ch;
	T arg;

	for(it = args.begin(); it != args.end(); ++it) 
	{
		ch = *it;

		// Toggle quoted string handling
		if (ch == '\"')
		{
			quoted = !quoted;
			continue;
		}

		// Handle \\, \", \', \n, \r, \t
		if (ch == '\\')
		{
			typename T::const_iterator next = it+1;
			if (next != args.end())
			{
				if (*next == '\\' || *next == '\"' || *next == '\'')
				{
					ch = *next;
					++it;
				}
				else if (*next == 'n')
				{
					ch = '\n';
					++it;
				}
				else if (*next == 'r')
				{
					ch = '\r';
					++it;
				}
				else if (*next == 't')
				{
					ch = '\t';
					++it;
				}
			}
		}

		// A space, a tab, line feed, carriage return
		if (!quoted && (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
		{
			// If we are not empty then we are at the end of the arg
			// otherwise we will ignore the extra chars
			if (!arg.empty())
			{
				argv.push_back(arg);
				arg.clear();
			}

			continue;
		}

		// Add the charater to the string
		arg += ch;
	}

	// Push any arg if it's left 
	if (!arg.empty()) argv.push_back(arg);
}

template void StringToArgv<std::string>(const std::string &args, std::vector<std::string> &argv);
template void StringToArgv<Pentagram::istring>(const Pentagram::istring &args, std::vector<Pentagram::istring> &argv);

