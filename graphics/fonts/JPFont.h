/*
Copyright (C) 2006 The Pentagram team

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

#ifndef JPFONT_H
#define JPFONT_H

#include "Font.h"

class ShapeFont;

class JPFont : public Pentagram::Font
{
public:
	JPFont(ShapeFont* jpfont, unsigned int fontnum);
	virtual ~JPFont();

	int getWidth(int c);

	virtual int getHeight();
	virtual int getBaseline();
	virtual int getBaselineSkip();

	virtual void getStringSize(const std::string& text,
							   int& width, int& height);
	virtual void getTextSize(const std::string& text,
							 int& resultwidth, int& resultheight,
							 unsigned int& remaining,
							 int width=0, int height=0,
							 TextAlign align=TEXT_LEFT, bool u8specials=false);

	virtual RenderedText* renderText(const std::string& text,
									 unsigned int& remaining,
									 int width=0, int height=0,
									 TextAlign align=TEXT_LEFT,
									 bool u8specials=false,
									 std::string::size_type cursor
											=std::string::npos);

	ENABLE_RUNTIME_CLASSTYPE();
protected:

	unsigned int fontnum;
	ShapeFont* shapefont;
};


#endif
