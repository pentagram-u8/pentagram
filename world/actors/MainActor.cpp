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
#include "Process.h"
#include "Kernel.h"
#include "TeleportToEggProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

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
// NB: be careful when calling this from a process, as it might kill
// all running processes
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

void MainActor::accumulateStr(int n)
{
	// already max?
	if (strength == 35) return; //!! constant

	accumStr += n;
	if (accumStr >= 650 || std::rand() % (650 - accumStr) == 0) { //!! constant
		strength++;
		accumStr = 0;
	}
}

void MainActor::accumulateDex(int n)
{
	// already max?
	if (dexterity == 35) return; //!! constant

	accumDex += n;
	if (accumDex >= 650 || std::rand() % (650 - accumDex) == 0) { //!! constant
		dexterity++;
		accumDex = 0;
	}
}

void MainActor::accumulateInt(int n)
{
	// already max?
	if (intelligence == 35) return; //!! constant

	accumInt += n;
	if (accumInt >= 650 || std::rand() % (650 - accumInt) == 0) { //!! constant
		intelligence++;
		accumInt = 0;
	}
}

void MainActor::saveData(ODataSource* ods)
{
	ods->write2(1); // version
	Actor::saveData(ods);
	uint8 jt = justTeleported ? 1 : 0;
	ods->write1(jt);
	ods->write4(accumStr);
	ods->write4(accumDex);
	ods->write4(accumInt);
}

bool MainActor::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Actor::loadData(ids)) return false;

	justTeleported = (ids->read1() != 0);
	accumStr = static_cast<sint32>(ids->read4());
	accumDex = static_cast<sint32>(ids->read4());
	accumInt = static_cast<sint32>(ids->read4());

	return true;
}

uint32 MainActor::I_teleportToEgg(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(mapnum);
	ARG_UINT16(teleport_id);
	ARG_UINT16(unknown); // 0/1

	return Kernel::get_instance()->addProcess(
		new TeleportToEggProcess(mapnum, teleport_id));
}

uint32 MainActor::I_accumulateStrength(const uint8* args,
									   unsigned int /*argsize*/)
{
	ARG_SINT16(n);
	MainActor* av = p_dynamic_cast<MainActor*>(
		World::get_instance()->getNPC(1));
	av->accumulateStr(n);

	return 0;
}

uint32 MainActor::I_accumulateDexterity(const uint8* args,
									   unsigned int /*argsize*/)
{
	ARG_SINT16(n);
	MainActor* av = p_dynamic_cast<MainActor*>(
		World::get_instance()->getNPC(1));
	av->accumulateDex(n);

	return 0;
}

uint32 MainActor::I_accumulateIntelligence(const uint8* args,
									   unsigned int /*argsize*/)
{
	ARG_SINT16(n);
	MainActor* av = p_dynamic_cast<MainActor*>(
		World::get_instance()->getNPC(1));
	av->accumulateInt(n);

	return 0;
}
