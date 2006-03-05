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

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "Gump.h"

struct GameInfo;

class GameWidget : public Gump
{
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	GameWidget(int X, int Y, Pentagram::istring& game);
	virtual ~GameWidget();

	Pentagram::istring getGameName();

	virtual void InitGump(Gump* newparent, bool take_focus=true);

	virtual uint16 TraceObjId(int mx, int my);

	virtual void PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	virtual Gump* OnMouseDown(int button, int mx, int my);
	virtual void OnMouseOver();
	virtual void OnMouseLeft();

	virtual void ChildNotify(Gump *child, uint32 message);

	enum Message
	{
		GAME_PLAY     = 1,
		GAME_LOAD     = 2,
		GAME_SETTINGS = 3,
		GAME_REMOVE   = 4
	};

protected:
	GameInfo* info;

	bool highlight;
};

#endif
