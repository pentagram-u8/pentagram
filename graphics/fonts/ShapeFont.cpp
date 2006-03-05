/*
Copyright (C) 2003-2006 The Pentagram team

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
#include "ShapeFont.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "ShapeRenderedText.h"

DEFINE_RUNTIME_CLASSTYPE_CODE_MULTI2(ShapeFont,Pentagram::Font,Shape);


ShapeFont::ShapeFont(const uint8* data,uint32 size,
					 const ConvertShapeFormat *format,
					 const uint16 flexId, const uint32 shapenum)
	: Font(), Shape(data,size,format, flexId, shapenum),
	  height(0), baseline(0), vlead(-1), hlead(0)
{

}

ShapeFont::~ShapeFont()
{

}


int ShapeFont::getWidth(char c)
{
	return getFrame(static_cast<unsigned char>(c))->width;
}

int ShapeFont::getHeight()
{
	if (height == 0)
	{
		for (uint32 i = 0; i < frameCount(); i++) 
		{
			int h = getFrame(i)->height;
			
			if (h > height) height = h;
		}
	}

	return height;
}

int ShapeFont::getBaseline()
{
	if (baseline == 0)
	{
		for (uint32 i = 0; i < frameCount(); i++) 
		{
			int b = getFrame(i)->yoff;
			
			if (b > baseline) baseline = b;
		}
	}

	return baseline;
}

int ShapeFont::getBaselineSkip()
{
	return getHeight() + getVlead();
}

void ShapeFont::getStringSize(const std::string& text, int& width, int& height)
{
	width = hlead;
	height = getHeight();

	for (unsigned int i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n' || text[i] == '\r') {
			// ignore
		} else {
			width += getWidth(text[i])-hlead;
		}
	}
}

RenderedText* ShapeFont::renderText(const std::string& text,
									unsigned int& remaining,
									int width, int height, TextAlign align,
									bool u8specials,
									std::string::size_type cursor)
{
	int resultwidth, resultheight;
	std::list<PositionedText> lines;
	lines = typesetText<Traits>(this, text, remaining,
								width, height, align, u8specials,
								resultwidth, resultheight, cursor);

	return new ShapeRenderedText(lines, resultwidth, resultheight,
								 getVlead(), this);
}
