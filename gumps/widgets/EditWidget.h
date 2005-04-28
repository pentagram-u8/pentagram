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

#ifndef EDITWIDGET_H
#define EDITWIDGET_H

//
// EditWidget. Widget for text input (single or multi-line)
//

#include "Gump.h"

#include "Font.h"

using Pentagram::Font;

class RenderedText;

class EditWidget : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	EditWidget();
	EditWidget(int X, int Y, std::string txt, int fontnum,
			   int width, int height, unsigned int maxlength=0,
			   bool multiline=false);
	virtual ~EditWidget(void);

	virtual void InitGump();

	virtual void PaintThis(RenderSurface*, sint32 lerp_factor);

	virtual Gump* OnMouseMotion(int mx, int my);
	virtual bool OnKeyDown(int key, int mod);
	virtual bool OnKeyUp(int key);
	virtual bool OnTextInput(int unicode);

	//! get the current text
	std::string getText() const { return text; }
	void setText(const std::string& t) { text = t; cursor = 0; }

	enum Message
	{
		EDIT_ENTER = 16,
		EDIT_ESCAPE = 17
	};

	bool loadData(IDataSource* ids, uint32 version);

protected:
	virtual void saveData(ODataSource* ods);

	std::string text;
	std::string::size_type cursor;
	int fontnum;
	unsigned int maxlength;
	bool multiline;

	uint32 cursor_changed;
	bool cursor_visible;

	void ensureCursorVisible();
	bool textFits(std::string& t);
	void renderText();

	RenderedText* cached_text;

};

#endif
