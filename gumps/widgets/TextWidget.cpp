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
#include "TextWidget.h"
#include "ShapeFont.h"
#include "RenderedText.h"
#include "FontManager.h"
#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(TextWidget,Gump);

TextWidget::TextWidget()
	: Gump(), cached_text(0)
{

}

TextWidget::TextWidget(int X, int Y, std::string txt, int font,
					   int w, int h, Font::TextAlign align) :
	Gump(X, Y, w, h), text(txt), fontnum(font), current_start(0),
	current_end(0), targetwidth(w), targetheight(h),
	cached_text(0), textalign(align)
{

}

TextWidget::~TextWidget(void)
{
	delete cached_text;
	cached_text = 0;
}

// Init the gump, call after construction
void TextWidget::InitGump()
{
	Gump::InitGump();

	Pentagram::Font *font;
	font = FontManager::get_instance()->getFont(fontnum, true);

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;

	setupNextText();
}

bool TextWidget::setupNextText()
{
	current_start = current_end;

	if (current_start >= text.size()) return false;

	Pentagram::Font *font;
	font = FontManager::get_instance()->getFont(fontnum, true);

	int tx, ty;
	unsigned int remaining;
	font->getTextSize(text.substr(current_start), tx, ty, remaining,
					  targetwidth, targetheight, textalign);

	dims.w = tx;
	dims.h = ty;
	current_end = current_start + remaining;

	delete cached_text;
	cached_text = 0;

	return true;
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void TextWidget::PaintThis(RenderSurface*surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf,lerp_factor);

	Pentagram::Font *font;
	font = FontManager::get_instance()->getFont(fontnum, true);

	if (!cached_text) {
		unsigned int remaining;
		cached_text = font->renderText(text.substr(current_start, current_end-current_start),
									   remaining, targetwidth, targetheight, textalign);
	}
	cached_text->draw(surf, 0, 0);
}

void TextWidget::saveData(ODataSource* ods)
{
	Gump::saveData(ods);

	ods->write4(static_cast<uint32>(fontnum));
	ods->write4(static_cast<uint32>(current_start));
	ods->write4(static_cast<uint32>(current_end));
	ods->write4(static_cast<uint32>(targetwidth));
	ods->write4(static_cast<uint32>(targetheight));
	ods->write2(static_cast<uint16>(textalign));
	ods->write4(text.size());
	ods->write(text.c_str(), text.size());
}

bool TextWidget::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	fontnum = static_cast<int>(ids->read4());
	current_start = static_cast<int>(ids->read4());
	current_end = static_cast<int>(ids->read4());
	targetwidth = static_cast<int>(ids->read4());
	targetheight = static_cast<int>(ids->read4());
	textalign = static_cast<Font::TextAlign>(ids->read2());

	uint32 slen = ids->read4();
	if (slen > 0) {
		char* buf = new char[slen+1];
		ids->read(buf, slen);
		buf[slen] = 0;
		text = buf;
		delete[] buf;
	} else {
		text = "";
	}

	return true;
}


// COLOURLESS PROTECTION
