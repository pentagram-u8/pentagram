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

#ifndef CONSOLEGUMP_H_INCLUDED
#define CONSOLEGUMP_H_INCLUDED

#include "Gump.h"

class ConsoleGump : public Gump
{
	enum ConsoleScrollState {
		NORMAL_DISPLAY,
		WAITING_TO_HIDE,
		SCROLLING_TO_HIDE_1,
		SCROLLING_TO_HIDE_2,
		SCROLLING_TO_HIDE_3,
		SCROLLING_TO_HIDE_4,
		NOTIFY_OVERLAY,
		WAITING_TO_SHOW,
		SCROLLING_TO_SHOW_1,
		SCROLLING_TO_SHOW_2,
		SCROLLING_TO_SHOW_3,
		SCROLLING_TO_SHOW_4
	};

	ConsoleScrollState	scroll_state;

public:
	ENABLE_RUNTIME_CLASSTYPE();

	ConsoleGump();
	ConsoleGump(int x, int y, int w, int h);
	virtual ~ConsoleGump();

	void ToggleConsole();
	void ShowConsole();
	void HideConsole();
	bool ConsoleIsVisible();

	// ConsoleGump doesn't have any 'effective' area
	virtual bool PointOnGump(int mx, int my) { return false; }

	virtual bool Run(const uint32 framenum);

	virtual void PaintThis(RenderSurface *surf, sint32 lerp_factor);

	bool loadData(IDataSource* ids);

protected:
	virtual void saveData(ODataSource* ods);
};

#endif //CONSOLEGUMP_H_INCLUDED

