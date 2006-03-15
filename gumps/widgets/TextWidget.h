/*
 *  Copyright (C) 2003-2006  The Pentagram Team
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

#ifndef TEXTWIDGET_H_INCLUDED
#define TEXTWIDGET_H_INCLUDED

//
// TextWidget. Displays text in either a fixed-size or a fit-to-text rectangle.
//

#include "Gump.h"

#include "Font.h"

using Pentagram::Font;

class RenderedText;

class TextWidget : public Gump
{
protected:
	std::string		text;
	bool			gamefont;
	int				fontnum;
	uint32 			blendColour;
	int				tx, ty;

	unsigned int	current_start; //!< start of currently displaying text
	unsigned int	current_end;   //!< start of remaining text

	int targetwidth, targetheight;

	RenderedText* cached_text;
	Font::TextAlign	textalign;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	TextWidget();
	TextWidget(int X, int Y, std::string txt, bool gamefont, int fontnum,
			   int width = 0, int height = 0,
			   Font::TextAlign align = Font::TEXT_LEFT);
	virtual ~TextWidget(void);

	// Init the gump, call after construction
	virtual void			InitGump(Gump* newparent, bool take_focus=true);

	// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
	virtual void			PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	virtual void			PaintComposited(RenderSurface* surf, sint32 lerp_factor, sint32 scalex, sint32 scaley);

	virtual Gump* OnMouseMotion(int mx, int my);

	//! display the next part of the text
	//! \return false if there is no more text to display
	bool setupNextText();

	//! reset the widget to the start of the text
	void rewind();

	//! get the text that's currently being displayed
	//! \param start Returns the start of the text
	//! \param end Returns the start of the remaining text
	void getCurrentText(unsigned int& start, unsigned int& end) const {
		start = current_start; end = current_end;
	}

	//! set the colour to blend the rendered text with. (0 to disable)
	void setBlendColour(uint32 col) { blendColour = col; }

	//! get the RenderedText's vlead
	int getVlead();

protected:
	void renderText();

	Pentagram::Font* getFont() const;

public:
	bool loadData(IDataSource* ids, uint32 version);

protected:
	virtual void saveData(ODataSource* ods);
};

#endif
