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

#include "pent_include.h"
#include "ConsoleGump.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ConsoleGump,Gump);

ConsoleGump::ConsoleGump(int X, int Y, int Width, int Height) :
	Gump(X,Y,Width,Height, 0, 0, LAYER_CONSOLE), scroll_state(NOT_SCROLLING)
{
	// Resize it
	con.CheckResize(Width);
}

ConsoleGump::~ConsoleGump()
{
}

void ConsoleGump::PaintThis(RenderSurface *surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf,lerp_factor);

	int h = dims.h;
	if (scroll_state == SCROLLING_TO_SHOW_1) 
		h = (h*(000+lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_SHOW_2) 
		h = (h*(256+lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_SHOW_3) 
		h = (h*(512+lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_SHOW_4) 
		h = (h*(768+lerp_factor))/1024;

	else if (scroll_state == SCROLLING_TO_HIDE_1) 
		h = (h*(1024-lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_HIDE_2)
		h = (h*(768-lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_HIDE_3)
		h = (h*(512-lerp_factor))/1024;
	else if (scroll_state == SCROLLING_TO_HIDE_4)
		h = (h*(256-lerp_factor))/1024;

	con.DrawConsole(surf,0,0,dims.w, h);
}

void ConsoleGump::UnhideGump()
{
	if (flags & FLAG_HIDDEN || scroll_state != NOT_SCROLLING)
		scroll_state = WAITING_TO_SHOW;
}

void ConsoleGump::HideGump()
{
	if (!(flags & FLAG_HIDDEN) || scroll_state != NOT_SCROLLING)
		scroll_state = WAITING_TO_HIDE;
}

bool ConsoleGump::Run(const uint32 framenum)
{
	Gump::Run(framenum);

	switch (scroll_state)
	{
	case WAITING_TO_HIDE:
		scroll_state = SCROLLING_TO_HIDE_1;
		break;

	case SCROLLING_TO_HIDE_1:
		scroll_state = SCROLLING_TO_HIDE_2;
		break;

	case SCROLLING_TO_HIDE_2:
		scroll_state = SCROLLING_TO_HIDE_3;
		break;

	case SCROLLING_TO_HIDE_3:
		scroll_state = SCROLLING_TO_HIDE_4;
		break;

	case SCROLLING_TO_HIDE_4:
		Gump::HideGump();
		scroll_state = NOT_SCROLLING;
		break;

	case WAITING_TO_SHOW:
		Gump::UnhideGump();
		scroll_state = SCROLLING_TO_SHOW_1;
		break;

	case SCROLLING_TO_SHOW_1:
		scroll_state = SCROLLING_TO_SHOW_2;
		break;

	case SCROLLING_TO_SHOW_2:
		scroll_state = SCROLLING_TO_SHOW_3;
		break;

	case SCROLLING_TO_SHOW_3:
		scroll_state = SCROLLING_TO_SHOW_4;
		break;

	case SCROLLING_TO_SHOW_4:
		scroll_state = NOT_SCROLLING;
		break;

	default:
		scroll_state = NOT_SCROLLING;
		break;
	}

	return true;	// Always repaint, even though we really could just try to detect it
}

// Colourless Protection
