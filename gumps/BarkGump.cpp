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
#include "SimpleTextWidget.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump,ItemRelativeGump);

// TODO: Remove all the hacks

BarkGump::BarkGump(uint16 owner, std::string msg) :
	ItemRelativeGump(0, 0, 100, 100, owner, 0, LAYER_ABOVE_NORMAL), barked(msg), counter(100)
{
}

BarkGump::~BarkGump(void)
{
}

void BarkGump::InitGump()
{
	ItemRelativeGump::InitGump();

	// OK, this is a bit of a hack, but it's how it's has to be
	int	fontnum;
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

	// Create the WrappedTextWidget
	Gump *widget = new SimpleTextWidget(0,0,barked,fontnum);

	widget->InitGump();

	// Add it to us
	AddChild(widget);

	// This is just a hack
	Pentagram::Rect d;
	widget->GetDims(d);
	counter = d.h*5; //! constant
	dims.h = d.h;
	dims.w = d.w;
}

bool BarkGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	// Auto close
	if (!--counter) Close();
	return true;	// Always repaint, even though we really could just try to detect it
}

Gump *BarkGump::OnMouseDown(int button, int mx, int my)
{
	Gump *g = ItemRelativeGump::OnMouseDown(button,mx,my);
	if (g) return g;

	// Close us.
	Close();
	return this;
}

// Colourless Protection
