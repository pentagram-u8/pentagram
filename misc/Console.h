/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2002 Ryan Nunn and The Pentagram Team

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

#ifndef CONSOLE_H
#define CONSOLE_H

//
// The Console
//
#define		CON_TEXTSIZE	32768

class Console
{
	char	text[CON_TEXTSIZE];
	int		current;				// line where next message will be printed
	int		x;						// offset in current line for next print
	int		display;				// bottom of console displays this line

	int 	linewidth;				// characters across screen
	int		totallines;				// total lines in console scrollback

	int		vislines;

public:
	Console();
	~Console();

	// Clear the buffer
	void	Clear();

	// Dump the buffer to a text file
	void	Dump (const char *name);

	// Resize the console buffer (on screen change)
	void	CheckResize (int scrwidth);


	//
	// STDOUT Methods
	//

	// Print a text string to the console, and output to stdout
	void	Print (const char *txt);

	// printf, and output to stdout
	int		Printf (const char *fmt, ...);

	// putchar, and output to stdout
	void	Putchar (int c);


	//
	// STDERR Methods
	//

	// Print a text string to the console, and output to stderr
	void	Print_err (const char *txt);

	// printf, and output to stderr
	int		Printf_err (const char *fmt, ...);

	// putchar, and output to stderr
	void	Putchar_err (int c);

private:

	// Print a text string to the console
	void	PrintInternal (const char *txt);

	// Add a linefeed to the buffer
	void	Linefeed ();

};

// Console object
extern	Console		con;

//
// Console Ouput Streams
//

#include <ostream>

//
// Standard Output Streambuf
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_streambuf : public std::basic_streambuf<_E, _Tr>
{
public:
	console_streambuf() : std::basic_streambuf<_E, _Tr>() { }
	virtual ~console_streambuf() { }

protected:

	// Output a character
	virtual int_type overflow(int_type _C = _Tr::eof())
	{
		if (!_Tr::eq_int_type(_Tr::eof(), _C)) con.Putchar(_Tr::to_char_type(_C));
		return (_Tr::not_eof(_C));
	}

	// Flush
	virtual int sync()
	{
		return 0;
	}
};

//
// Standard Output Stream
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_ostream : public std::basic_ostream<_E, _Tr>
{
	console_streambuf<_E, _Tr> _Fb;

public:	
	console_ostream() : std::basic_ostream<_E, _Tr>(&_Fb) {}
	virtual ~console_ostream() { }
};

//
// Standard Output Stream Object
//
extern console_ostream<char>		pout;


//
// Error Output Streambuf
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_err_streambuf : public std::basic_streambuf<_E, _Tr>
{
public:
	console_err_streambuf() : std::basic_streambuf<_E, _Tr>() { }
	virtual ~console_err_streambuf() { }

protected:

	// Output a character
	virtual int_type overflow(int_type _C = _Tr::eof())
	{
		if (!_Tr::eq_int_type(_Tr::eof(), _C)) con.Putchar_err(_Tr::to_char_type(_C));
		return (_Tr::not_eof(_C));
	}

	// Flush
	virtual int sync()
	{
		return 0;
	}
};

//
// Error Output Stream
//
template<class _E, class _Tr = std::char_traits<_E> >
class console_err_ostream : public std::basic_ostream<_E, _Tr>
{
	console_err_streambuf<_E, _Tr> _Fb;

public:	
	console_err_ostream() : std::basic_ostream<_E, _Tr>(&_Fb) {}
	virtual ~console_err_ostream() { }
};

//
// Error Output Stream Object
//
extern console_err_ostream<char>	perr;

#endif // CONSOLE_H
