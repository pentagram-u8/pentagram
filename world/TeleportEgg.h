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

#ifndef TELEPORTEGG_H
#define TELEPORTEGG_H

#include "Egg.h"

class TeleportEgg : public Egg
{
	friend class ItemFactory;
public:
	TeleportEgg();
	virtual ~TeleportEgg();

	bool isTeleporter() const { return (quality & 0xFF00) != 0; }
	int getTeleportId() const { return (quality & 0xFF); }

	ENABLE_RUNTIME_CLASSTYPE();


	virtual uint16 hatch();
};


#endif