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
#include "FontShapeFlex.h"
#include "GameData.h"
#include "IDataSource.h"
#include "ODataSource.h"

//#undef USE_SDLTTF

#ifdef USE_SDLTTF
// HACK
#include "TTFont.h"
extern TTFont* ttffont;
#endif

DEFINE_RUNTIME_CLASSTYPE_CODE(TextWidget,Gump);

TextWidget::TextWidget()
	: Gump(), cached_text(0)
{

}

TextWidget::TextWidget(int X, int Y, std::string txt, int font, int w, int h) :
	Gump(X, Y, w, h), text(txt), fontnum(font), current_start(0),
	current_end(0), targetwidth(w), targetheight(h), cached_text(0)
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
#ifdef USE_SDLTTF
	// HACK
	if (ttffont)
	  font = ttffont;
	else
#else
	font = GameData::get_instance()->getFonts()->getFont(fontnum);
#endif

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
#ifdef USE_SDLTTF
	// HACK
	if (ttffont)
	  font = ttffont;
	else
#else
	font = GameData::get_instance()->getFonts()->getFont(fontnum);
#endif

	int tx, ty;
	unsigned int remaining;
	font->getTextSize(text.substr(current_start), tx, ty, remaining,
					  targetwidth, targetheight);

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

#ifdef USE_SDLTTF
	// HACK
	Pentagram::Font *font = ttffont;
#else
	Pentagram::Font *font = GameData::get_instance()->
		getFonts()->getFont(fontnum);
#endif

	if (!cached_text) {
		unsigned int remaining;
		cached_text = font->renderText(text.substr(current_start, current_end-current_start), remaining, 180, 0);
	}
	cached_text->draw(surf, 0, 0);
//	surf->PrintText(font, text.c_str(), 0, 0);
}

void TextWidget::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);

	ods->write4(static_cast<uint32>(fontnum));
	ods->write4(static_cast<uint32>(current_start));
	ods->write4(static_cast<uint32>(current_end));
	ods->write4(static_cast<uint32>(targetwidth));
	ods->write4(static_cast<uint32>(targetheight));
	ods->write4(text.size());
	ods->write(text.c_str(), text.size());
}

bool TextWidget::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	fontnum = static_cast<int>(ids->read4());
	current_start = static_cast<int>(ids->read4());
	current_end = static_cast<int>(ids->read4());
	targetwidth = static_cast<int>(ids->read4());
	targetheight = static_cast<int>(ids->read4());

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
