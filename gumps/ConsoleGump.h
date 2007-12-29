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

#ifndef CONSOLEGUMP_H_INCLUDED
#define CONSOLEGUMP_H_INCLUDED

#include "Gump.h"

class ConsoleGump : public Gump
{
	enum ConsoleScrollState {
		NORMAL_DISPLAY,
		WAITING_TO_HIDE,
		SCROLLING_TO_HIDE,
		NOTIFY_OVERLAY,
		WAITING_TO_SHOW,
		SCROLLING_TO_SHOW
	};

	ConsoleScrollState	scroll_state;
	uint32 scroll_frame;

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

	virtual void run();

	virtual void PaintThis(RenderSurface *surf, sint32 lerp_factor, bool scaled);

	virtual void RenderSurfaceChanged();

	virtual bool		OnTextInput(int unicode);
	virtual void		OnFocus(bool /*gain*/);
	virtual bool		OnKeyDown(int key, int mod);

	static void			ConCmd_toggle(const Console::ArgvType &argv);	//!< "ConsoleGump::toggle" console command

	bool	loadData(IDataSource* ids, uint32 version);

protected:
	virtual void saveData(ODataSource* ods);

private:
};

#endif //CONSOLEGUMP_H_INCLUDED

