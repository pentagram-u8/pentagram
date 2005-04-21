/*
Copyright (C) 2004 The Pentagram team

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

#ifndef FONT_H
#define FONT_H

#include <list>
#include "Rect.h"

class RenderedText;

struct PositionedText {
	std::string text;
	Pentagram::Rect dims;
	std::string::size_type cursor;
};

namespace Pentagram
{

class Font
{
public:
	Font();
	virtual ~Font();

	ENABLE_RUNTIME_CLASSTYPE();

	enum TextAlign {
		TEXT_LEFT,
		TEXT_CENTER,
		TEXT_RIGHT
	};

	//! get the height of the font
	virtual int getHeight()=0;

	//! get the baseline of the font (relative from the top)
	virtual int getBaseline()=0;

	//! get the baselineskip of the font (distance between two baselines)
	virtual int getBaselineSkip()=0;

	//! get the dimensions of a string (not containing any newlines)
	//! \param text The string
	//! \param width Returns the width
	//! \param height Returns the height
	virtual void getStringSize(std::string& text, int& width, int& height)=0;

	//! render a string
	//! \param text The text
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param align Alignment of the text (left, right, center)
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \return the rendered text in a RenderedText object
	virtual RenderedText* renderText(std::string text,
									 unsigned int& remaining,
									 int width=0, int height=0,
									 TextAlign align=TEXT_LEFT,
									 bool u8specials=false,
									 std::string::size_type cursor=std::string::npos)=0;

	//! get the dimensions of a rendered string
	//! \param text The text
	//! \param resultwidth Returns the resulting width
	//! \param resultheight Returns the resulting height
	//! \param remaining Returns index of the first character not printed
	//! \param width The width of the target rectangle, or 0 for unlimited
	//! \param height The height of the target rectangle, or 0 for unlimited
	//! \param u8specials If true, interpret the special characters U8 uses
	//! \param align Alignment of the text (left, right, center)
	void getTextSize(std::string text,
					 int& resultwidth, int& resultheight,
					 unsigned int& remaining,
					 int width=0, int height=0,
					 TextAlign align=TEXT_LEFT, bool u8specials=false);
	

protected:
	std::list<PositionedText> typesetText(std::string& text,
										  unsigned int& remaining,
										  int width, int height,
										  TextAlign align, bool u8specials,
										  int& resultwidth, int& resultheight,
										  std::string::size_type cursor
												=std::string::npos);
};

}

#endif
