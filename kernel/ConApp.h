/*
 *	CoreApp.h - Base application class that contains the minimal functality to
 *	            support an instance of the pentagram engine
 *
 *  Copyright (C) 2002-2003 The Pentagram Team
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

#ifndef CONAPP_H
#define CONAPP_H

#include "CoreApp.h"

#include <SDL.h>

class ConApp : public CoreApp
{
public:
	ConApp(int argc, const char* const* argv);
	virtual ~ConApp();

	static ConApp* get_instance()
		{ return p_dynamic_cast<ConApp*>(application); }

	void startup();

	virtual void helpMe();
	
	virtual void run();
	virtual void paint() {}; // should be removed from CoreApp
	virtual void handleEvent(const SDL_Event&event) {}; // should this be in CoreApp?
	
protected:
	virtual void DeclareArgs();

private:
	// various temporary state flags, to be moved somewhere more appropriate in time.
	bool weAreDisasming;
	bool weAreCompiling;
	bool weAreWeAreWeAreTheMany; // false for the time being. *grin*
};

#endif

