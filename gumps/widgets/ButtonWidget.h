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

#ifndef BUTTONWIDGET_H_INCLUDED
#define BUTTONWIDGET_H_INCLUDED

#include "SimpleTextWidget.h"

class ButtonWidget : public SimpleTextWidget
{
	//Shape *shape_up, uint32 framenum_up;
	//Shape *shape_down, uint32 framenum_down;
	bool down;
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	ButtonWidget(int X, int Y, std::string txt, int font, int width = 0, int height = 0);
	virtual ~ButtonWidget(void);

	// Init the gump, call after construction
	virtual void			InitGump();

	virtual Gump *			OnMouseDown(int button, int mx, int my);
	virtual void			OnMouseUp(int  button, int mx, int my);
	virtual void			OnMouseClick(int button, int mx, int my);
	virtual void			OnMouseDouble(int button, int mx, int my);

	//void					SetShapeDown(Shape *_shape, uint32 _framenum);
	//void					SetShapeUp(Shape *_shape, uint32 _framenum);

	enum Message
	{
		BUTTON_DOWN			= 0,
		BUTTON_UP			= 1,
		BUTTON_DOUBLE		= 2
	};
};

#endif // BUTTONWIDGET_H_INCLUDED
