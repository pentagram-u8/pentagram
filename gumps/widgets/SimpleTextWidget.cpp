/*
 *  Copyright (C) 2003  The Pentagram Team
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
#include "SimpleTextWidget.h"
#include "Font.h"
#include "FontShapeFlex.h"
#include "GameData.h"
#include "RenderSurface.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(SimpleTextWidget,Gump);

SimpleTextWidget::SimpleTextWidget(int X, int Y, std::string txt, int font, int w, int h) :
	Gump(X, Y, w, h), text(txt), fontnum(font)
{
}

SimpleTextWidget::~SimpleTextWidget(void)
{
}

// Init the gump, call after construction
void SimpleTextWidget::InitGump()
{
	Gump::InitGump();

	Font *font = GameData::get_instance()->getFonts()->getFont(fontnum);

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;

	// resize
	if (!dims.w || !dims.h)
	{
		sint32 tx, ty; 
		font->getTextSize(text.c_str(), tx, ty);

		if (!dims.w) dims.w = tx;
		if (!dims.h) dims.h = ty;
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void SimpleTextWidget::PaintThis(RenderSurface*surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf,lerp_factor);

	Font *font = GameData::get_instance()->getFonts()->getFont(fontnum);
	surf->PrintText(font, text.c_str(), 0, 0);
}

// COLOURLESS PROTECTION
