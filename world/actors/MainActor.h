/*
Copyright (C) 2003 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MAINACTOR_H
#define MAINACTOR_H

#include "Actor.h"

class MainActor : public Actor
{
public:
	MainActor();
	virtual ~MainActor();

	virtual void teleport(int mapnum, sint32 x, sint32 y, sint32 z);
	void teleport(int mapnum, int teleport_id); // to teleportegg

	bool hasJustTeleported() const { return justTeleported; }
	void setJustTeleported(bool t) { justTeleported = t; }

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

private:
	bool justTeleported;
};

#endif
