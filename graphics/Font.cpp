/*
Copyright (C) 2003 The Pentagram team

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
#include "Font.h"
#include "ShapeFrame.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(Font,Shape);

sint32 Font::getWidth(char c)
{
	// just return framewidth, or do we need to use hlead too?
	return getFrame(static_cast<uint32>(c))->width;
}

sint32 Font::getHeight()
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

void Font::getTextSize(const char *text, sint32 &x, sint32 &y)
{
	sint32 lines = 2;
	sint32 width = 0;

	x = 0;
	y = 0;

	// HACK We force a line feed at the next space after we are 160 pixels across

	for (;*text; ++text)
	{
		if (*text == '\n' || (*text == ' ' && width > 160 && *(text+1) != '\n') )
		{
			++lines;
			if (width > x) x = width;
			width = 0;
		}
		else if (*text != '\r')
			width += getWidth(*text);
	}
	if (width > x) x = width;
	y = lines * getHeight();
}
