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

#ifndef FONT_H
#define FONT_H

#include "Shape.h"

#include <string>

namespace Pentagram
{

class Font : public Shape
{
	sint32 height;
	sint32 baseline;
	sint32 vlead;
	sint32 hlead;
public:
	Font(const uint8* data, uint32 size, const ConvertShapeFormat *format)
		: Shape(data, size, format), height(0), baseline(0), vlead(1), hlead(0) { }
	virtual ~Font() { }

	sint32 getWidth(char c);
	sint32 getHeight();
	sint32 getBaseline();
	sint32 getVlead() { return vlead; }
	sint32 getHlead() { return hlead; }

	void getTextSize(const char *text, sint32 &x, sint32 &y);

	ENABLE_RUNTIME_CLASSTYPE();
};

}

#endif
