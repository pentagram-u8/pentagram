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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SIMPLETEXTWIDGET_H_INCLUDED
#define SIMPLETEXTWIDGET_H_INCLUDED

//
// SimpleTextWidget. Just paints the text, resizes itself to fit if desired
//
// Alignment is always upper left
//
// For word wrapping multipage text widgets (such as used by BarkGump) use
// WrappingTextWidget
//
// For Book like facing page text widgets use FacingPageTextWidget
//

#include "Gump.h"

class SimpleTextWidget : public Gump
{
protected:
	std::string		text;
	int				fontnum;

public:
	ENABLE_RUNTIME_CLASSTYPE();

	SimpleTextWidget();
	SimpleTextWidget(int X, int Y, std::string txt, int fontnum, int width = 0, int height = 0);
	virtual ~SimpleTextWidget(void);

	// Init the gump, call after construction
	virtual void			InitGump();

	// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
	virtual void			PaintThis(RenderSurface*, sint32 lerp_factor);

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif SIMPLETEXTWIDGET_H_INCLUDED
