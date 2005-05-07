/*
 *  Copyright (C) 2005  The Pentagram Team
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
#include "EditWidget.h"
#include "ShapeFont.h"
#include "RenderedText.h"
#include "FontManager.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "TTFont.h"
#include "encoding.h"

#include "SDL.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(EditWidget,Gump);

EditWidget::EditWidget()
	: Gump(), cursor_changed(0), cursor_visible(true), cached_text(0)
{

}

EditWidget::EditWidget(int X, int Y, std::string txt, int font,
					   int w, int h, unsigned int maxlength_, bool multiline_)
	: Gump(X, Y, w, h), text(txt), fontnum(font),
	  maxlength(maxlength_), multiline(multiline_),
	  cursor_changed(0), cursor_visible(true), cached_text(0)
{
	cursor = text.size();
}

EditWidget::~EditWidget(void)
{
	delete cached_text;
	cached_text = 0;
}

// Init the gump, call after construction
void EditWidget::InitGump()
{
	Gump::InitGump();

	Pentagram::Font *font;
	font = FontManager::get_instance()->getGameFont(fontnum, true);

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;
}

void EditWidget::setText(const std::string& t)
{
	text = t;
	cursor = text.size();
	FORGET_OBJECT(cached_text);
}

void EditWidget::ensureCursorVisible()
{
	cursor_visible = true;
	cursor_changed = SDL_GetTicks();
}

bool EditWidget::textFits(std::string& t)
{
	Pentagram::Font *font;
	font = FontManager::get_instance()->getGameFont(fontnum, true);
	
	unsigned int remaining;
	int width, height;
	font->getTextSize(t, width, height, remaining,
					  multiline ? dims.w : 0, dims.h,
					  Pentagram::Font::TEXT_LEFT, false);

	if (multiline)
		return (remaining >= t.size());
	else
		return (width <= dims.w);
}

void EditWidget::renderText()
{
	bool cv = cursor_visible;
	if (!IsFocus()) {
		cv = false;
	} else {
		uint32 now = SDL_GetTicks();
		if (now > cursor_changed + 750) {
			cv = !cursor_visible;
			cursor_changed = now;
		}
	}

	if (cv != cursor_visible) {
		FORGET_OBJECT(cached_text);
		cursor_visible = cv;
	}

	if (!cached_text) {
		Pentagram::Font *font;
		font = FontManager::get_instance()->getGameFont(fontnum, true);

		unsigned int remaining;
		cached_text = font->renderText(text, remaining,
									   multiline ? dims.w : 0, dims.h,
									   Pentagram::Font::TEXT_LEFT,
									   false, cv ? cursor : std::string::npos);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void EditWidget::PaintThis(RenderSurface*surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf,lerp_factor);

	renderText();
	cached_text->draw(surf, 0, 0);
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump* EditWidget::OnMouseMotion(int mx, int my)
{
	return 0;
}

bool EditWidget::OnKeyDown(int key, int mod)
{
	switch (key) {
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		parent->ChildNotify(this, EDIT_ENTER);
		break;
	case SDLK_ESCAPE:
		parent->ChildNotify(this, EDIT_ESCAPE);
		break;
	case SDLK_BACKSPACE:
		if (cursor > 0) {
			text.erase(--cursor, 1);
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case SDLK_DELETE:
		if (cursor != text.size()) {
			text.erase(cursor, 1);
			FORGET_OBJECT(cached_text);
		}
		break;
	case SDLK_LEFT:
		if (cursor > 0) {
			cursor--;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case SDLK_RIGHT:
		if (cursor < text.size()) {
			cursor++;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	default:
		break;
	}

	return true;
}

bool EditWidget::OnKeyUp(int key)
{
	return true;
}


bool EditWidget::OnTextInput(int unicode)
{
	if (maxlength > 0 && text.size() >= maxlength)
		return true;

	char c = 0;
	if (unicode >= 0 && unicode < 256)
		c = reverse_encoding[unicode];
	if (!c) return true;

	std::string newtext = text;
	newtext.insert(cursor, 1, c);

	if (textFits(newtext)) {
		text = newtext;
		cursor++;
		FORGET_OBJECT(cached_text);
	}

	return true;
}

void EditWidget::saveData(ODataSource* ods)
{
	Gump::saveData(ods);

	ods->write4(static_cast<uint32>(fontnum));
	ods->write4(static_cast<uint32>(maxlength));
	ods->write4(static_cast<uint32>(cursor));
	ods->write4(text.size());
	ods->write(text.c_str(), text.size());
}

bool EditWidget::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	fontnum = static_cast<int>(ids->read4());
	maxlength = static_cast<int>(ids->read4());
	cursor = ids->read4();
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

	// HACK ALERT (need to do this since the ttf setting might have changed)
	Pentagram::Font *font;
	font = FontManager::get_instance()->getGameFont(fontnum, true);

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	return true;
}

