/*
 *	FileSystem.cpp - The Pentagram File System
 *
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

#include "pent_include.h"

#include "FileSystem.h"

#include "exceptions.h"

#include <string>
using	std::string;

/*
 *	Open a file for input,
 *	trying the original name (lower case), and the upper case version
 *	of the name.
 *
 *	Output: 0 if couldn't open.
 */

bool FileSystem::rawopen
	(
	std::ifstream& in,			// Input stream to open.
	const string &fname,		// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = std::ios::in;
#else
	int mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#endif
	//string name = get_system_path(fname);
	string name(fname);
	switch_slashes(name);
	
	int uppercasecount = 0;
	do {
		// We first "clear" the stream object. This is done to prevent
		// problems when re-using stream objects
		in.clear();
		try {
			in.open(name.c_str(), mode);		// Try to open
		} catch (std::exception &)
		{}
		if (in.good() && !in.fail())
			return true; // found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	//throw (file_open_exception(fname/*get_system_path(fname)*/));
	throw file_open_exception(fname);
	return false;
}

/*
 *	Open a file for output,
 *	trying the original name (lower case), and the upper case version
 *	of the name.
 *
 *	Output: 0 if couldn't open.
 */

bool FileSystem::rawopen
	(
	std::ofstream& out,			// Output stream to open.
	const string &fname,			// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#elif defined(XWIN)
	int mode = std::ios::out | std::ios::trunc;
#else
	int mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#endif
	//string name = get_system_path(fname);
	string name(fname);
	switch_slashes(name);

	// We first "clear" the stream object. This is done to prevent
	// problems when re-using stream objects
	out.clear();

	int uppercasecount = 0;
	do {
		out.open(name.c_str(), mode);		// Try to open
		if (out.good())
			return true; // found it!
		out.clear();	// Forget ye not
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
//	throw (file_open_exception(fname/*get_system_path(fname)*/));
	throw file_open_exception(fname);
	return false;
}

void FileSystem::switch_slashes(string &name)
{
#ifdef WIN32
	for(string::iterator X = name.begin(); X != name.end(); ++X)
	{
		if(*X == '/' )
			*X =  '\\';
	}
#elif defined(MACOS)
	// We use a component-wise algorithm (suggested by Yorick)
	// Basically, split the path along the "/" seperators
	// If a component is empty or '.', remove it. If it's '..', replace it
	// with the empty string. convert all / to :
	string::size_type	begIdx, endIdx;;
	string	component;
	string	new_name;

	if( name.at(0) != '/' )
		new_name = ":";

	begIdx = name.find_first_not_of('/');
	while( begIdx != string::npos )
	{
		endIdx = name.find_first_of('/', begIdx);
		if( endIdx == std::string::npos )
			component = name.substr(begIdx);
		else
			component = name.substr(begIdx, endIdx-begIdx);
		if( component == ".." )
			new_name += ":";
		else if( !component.empty() && component != "." )
		{
			new_name += component;
			if( endIdx != std::string::npos )
				new_name += ":";
		}
		begIdx = name.find_first_not_of('/', endIdx);
	}

	name = new_name;
#else
	// do nothing
#endif
}

/*
 *	Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

bool FileSystem::base_to_uppercase(string& str, const int count)
{
	if (count <= 0) return true;

	int todo = count;
					// Go backwards.
	string::reverse_iterator X;
	for(X = str.rbegin(); X != str.rend(); ++X)
		{
					// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = std::toupper(*X);
#endif
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}



