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

#ifndef CREDITSGUMP_H
#define CREDITSGUMP_H

#include "ModalGump.h"

class RenderedText;

class CreditsGump : public ModalGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	CreditsGump();
	CreditsGump(const std::string& text, int parskip=24,
				uint32 _Flags = 0, sint32 layer = LAYER_MODAL);
	virtual ~CreditsGump(void);

	// Init the gump, call after construction
	virtual void InitGump(Gump* newparent, bool take_focus=true);

	// Set a configuration option to true when user watches entire sequence
	void SetFlagWhenFinished(std::string configkey_) { configkey=configkey_; }

	virtual void Close(bool no_del = false);

	virtual void run();

	// Paint the Gump
	virtual void PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	virtual bool OnKeyDown(int key, int mod);

protected:

	void extractLine(std::string& text, char& modifier, std::string& line);

	std::string text;
	int parskip;

	enum CreditsState {
		CS_PLAYING,
		CS_FINISHING,
		CS_CLOSING
	} state;

	int timer;

	RenderedText* title;
	RenderedText* nexttitle;
	int nexttitlesurf;

	RenderSurface* scroll[4];
	int scrollheight[4];
	int currentsurface;
	int currenty;

	std::string configkey;
};

#endif
