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

#include "pent_include.h"

#include "MainActor.h"
#include "World.h"
#include "TeleportEgg.h"
#include "CurrentMap.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainActor,Actor);

MainActor::MainActor() : justTeleported(false)
{

}

MainActor::~MainActor()
{

}

void MainActor::teleport(int mapnum, sint32 x, sint32 y, sint32 z)
{
	World* world = World::get_instance();

	// (attempt to) load the new map
	if (!world->switchMap(mapnum)) {
		perr << "MainActor::teleport(): switchMap() failed!" << std::endl;
		return;
	}

	Actor::teleport(mapnum, x, y, z);

	justTeleported = true;
}

// teleport to TeleportEgg
//NB: This is _not_ suitable for the teleportToEgg intrinsic,
// as it will kill all running UCProcesses
// (maybe we shouldn't kill currently running processes in switchMap()?)
void MainActor::teleport(int mapnum, int teleport_id)
{
	World* world = World::get_instance();
	CurrentMap* currentmap = world->getCurrentMap();

	// (attempt to) load the new map
	if (!world->switchMap(mapnum)) {
		perr << "MainActor::teleport(): switchMap() failed!" << std::endl;
		return;
	}

	// find destination
	TeleportEgg* egg = currentmap->findDestination(teleport_id);
	sint32 x,y,z;
	egg->getLocation(x,y,z);

	perr << "Found destination: " << x << "," << y << "," << z << std::endl;

	Actor::teleport(mapnum, x, y, z);

	justTeleported = true;
}
