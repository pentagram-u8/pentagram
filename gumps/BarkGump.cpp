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
#include "BarkGump.h"
#include "Font.h"
#include "FontShapeFlex.h"
#include "GameData.h"
#include "RenderSurface.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump,ItemRelativeGump);

// TODO: Remove all the hacks

BarkGump::BarkGump(uint16 owner, std::string msg) :
	ItemRelativeGump(0, 0, 100, 100, owner, 0, LAYER_ABOVE_NORMAL), barked(msg), fontnum(0), counter(100)
{
	// OK, this is a bit of a hack, but it's how it's has to be
	if (owner == 1) fontnum = 6;
	else if (owner > 256) fontnum = 8;
	else switch (owner%3) {
		case 1:
			fontnum = 5;
			break;
			
		case 2:
			fontnum = 7;
			break;

		default:
			fontnum = 0;
			break;
	}
}

BarkGump::~BarkGump(void)
{
}

void BarkGump::InitGump()
{
	ItemRelativeGump::InitGump();

	// Create the TextWidget... ok, I couldn't care less at the moment,
	// This is just a hack

	Font *font = GameData::get_instance()->getFonts()->getFont(fontnum);

	sint32 tx, ty; 
	font->getTextSize(barked.c_str(), tx, ty);
	dims.w = tx;
	dims.h = ty;
	counter = ty*5;
}

bool BarkGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	// Auto close
	if (!--counter) Close();
	return true;	// Always repaint, even though we really could just try to detect it
}

// Overloadable method to Paint just this Gumps (RenderSurface is relative to this)
void BarkGump::PaintThis(RenderSurface*surf, sint32 lerp_factor)
{
	ItemRelativeGump::PaintThis(surf,lerp_factor);
	Font *font = GameData::get_instance()->getFonts()->getFont(fontnum);

	surf->PrintText(font,barked.c_str(), 0, font->getBaseline());
}

// Colourless Protection
