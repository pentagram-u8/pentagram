/*
Copyright (C) 2003-2005 The Pentagram team

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

#ifndef SHAPEFONT_H
#define SHAPEFONT_H

#include "Font.h"
#include "Shape.h"

class ShapeFont : public Pentagram::Font, public Shape
{
	int height;
	int baseline;
	int vlead;
	int hlead;

public:
	ShapeFont(const uint8* data,uint32 size, const ConvertShapeFormat *format,
			  const uint16 flexId, const uint32 shapenum);
	virtual ~ShapeFont();

	virtual int getHeight();
	virtual int getBaseline();
	virtual int getBaselineSkip();

	int getWidth(char c);
	int getVlead() const { return vlead; }
	int getHlead() const { return hlead; }

	void setVLead(int vl) { vlead = vl; }
	void setHLead(int hl) { hlead = hl; }

	virtual void getStringSize(const std::string& text,
							   int& width, int& height);

	virtual RenderedText* renderText(const std::string& text,
									 unsigned int& remaining,
									 int width=0, int height=0,
									 TextAlign align=TEXT_LEFT,
									 bool u8specials=false,
									 std::string::size_type cursor
											=std::string::npos);

	ENABLE_RUNTIME_CLASSTYPE();
};

#endif
