/*
 *  Copyright (C) 2003-2004  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "BarkGump.h"
#include "TextWidget.h"
#include "Kernel.h"
#include "GUIApp.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump,ItemRelativeGump);

// TODO: Remove all the hacks

BarkGump::BarkGump()
	: ItemRelativeGump()
{

}

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

	// OK, this is a bit of a hack, but it's how it has to be
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

	// Create the TextWidget
	Gump *widget = new TextWidget(0,0,barked,fontnum,194,55);
	widget->InitGump();

	textwidget = widget->getObjId();

	// Add it to us
	AddChild(widget);

	// This is just a hack
	Pentagram::Rect d;
	widget->GetDims(d);
	counter = d.h*5; //! constant
	dims.h = d.h;
	dims.w = d.w;
}

void BarkGump::NextText()
{
	TextWidget *widget = dynamic_cast<TextWidget*>
		(GUIApp::get_instance()->getGump(textwidget));
	assert(widget);
	if (widget->setupNextText()) {
		// This is just a hack
		Pentagram::Rect d;
		widget->GetDims(d);
		counter = d.h*5; //! constant
		dims.h = d.h;
		dims.w = d.w;
	} else {
		Close();
	}
}

bool BarkGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	// Auto close
	if (!Kernel::get_instance()->isPaused()) {
		if (!--counter) NextText();
	}
	return true;	// Always repaint, even though we really could just try to detect it
}

Gump *BarkGump::OnMouseDown(int button, int mx, int my)
{
	Gump *g = ItemRelativeGump::OnMouseDown(button,mx,my);
	if (g) return g;

	// Scroll to next text, if possible
	NextText();
	return this;
}

void BarkGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	ItemRelativeGump::saveData(ods);

	ods->write4(static_cast<uint32>(counter));
	ods->write2(textwidget);
	ods->write4(barked.size());
	ods->write(barked.c_str(), barked.size());
}

bool BarkGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ItemRelativeGump::loadData(ids)) return false;

	counter = static_cast<sint32>(ids->read4());
	textwidget = ids->read2();
	uint32 slen = ids->read4();
	if (slen > 0) {
		char* buf = new char[slen+1];
		ids->read(buf, slen);
		buf[slen] = 0;
		barked = buf;
		delete[] buf;
	} else {
		barked = "";
	}

	return true;
}

// Colourless Protection
