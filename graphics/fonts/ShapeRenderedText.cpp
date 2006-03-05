/*
Copyright (C) 2004-2005 The Pentagram team

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
#include "ShapeRenderedText.h"

#include "ShapeFont.h"
#include "RenderSurface.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ShapeRenderedText,RenderedText);


ShapeRenderedText::ShapeRenderedText(std::list<PositionedText>& lines_,
									 int width_, int height_, int vlead_,
									 ShapeFont* font_)
	: lines(lines_), font(font_)
{
	width = width_;
	height = height_;
	vlead = vlead_;
}

ShapeRenderedText::~ShapeRenderedText()
{

}

void ShapeRenderedText::draw(RenderSurface* surface, int x, int y, bool /*destmasked*/)
{
	// TODO support masking here???

	std::list<PositionedText>::iterator iter;

	for (iter = lines.begin(); iter != lines.end(); ++iter)
	{
		int line_x = x + iter->dims.x;
		int line_y = y + iter->dims.y;

		std::size_t textsize = iter->text.size();

		for (std::size_t i = 0; i < textsize; ++i) {
			surface->Paint(font, static_cast<unsigned char>(iter->text[i]),
						   line_x, line_y);

			if (i == iter->cursor) {
				surface->Fill32(0xFF000000, line_x, line_y-font->getBaseline(),
								1, iter->dims.h);
			}

			line_x += font->getWidth(iter->text[i])-font->getHlead();
		}

		if (iter->cursor == textsize) {
			surface->Fill32(0xFF000000, line_x, line_y-font->getBaseline(),
							1, iter->dims.h);
		}
	}
}

void ShapeRenderedText::drawBlended(RenderSurface* surface, int x, int y,
									uint32 col, bool /*destmasked*/)
{
	// TODO Support masking here ????

	std::list<PositionedText>::iterator iter;

	for (iter = lines.begin(); iter != lines.end(); ++iter)
	{
		int line_x = x + iter->dims.x;
		int line_y = y + iter->dims.y;

		std::size_t textsize = iter->text.size();

		for (std::size_t i = 0; i < textsize; ++i) {
			surface->PaintHighlight(font,
									static_cast<unsigned char>(iter->text[i]),
									line_x, line_y, false, false, col);
			line_x += font->getWidth(iter->text[i])-font->getHlead();
		}
		
	}
}
