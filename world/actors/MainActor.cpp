/*
Copyright (C) 2003-2004 The Pentagram team

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
#include "CameraProcess.h"

#include "Configuration.h"
#include "CoreApp.h"
#include "GameData.h"
#include "WpnOvlayDat.h"
#include "ShapeInfo.h"

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

uint16 MainActor::getDefenseType()
{
	uint16 type = 0;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		uint32 shape = (*iter)->getShape();
		uint32 frame = (*iter)->getFrame();
		ShapeInfo* si = (*iter)->getShapeInfo();
		if (si->armourinfo) {
			type |= si->armourinfo[frame].defense_type;
		}
	}

	return type;
}

uint32 MainActor::getArmourClass()
{
	uint32 armour = 0;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		uint32 shape = (*iter)->getShape();
		uint32 frame = (*iter)->getFrame();
		ShapeInfo* si = (*iter)->getShapeInfo();
		if (si->armourinfo) {
			armour += si->armourinfo[frame].armour_class;
		}
		if (si->weaponinfo) {
			armour += si->weaponinfo->armour_bonus;
		}
	}

	return armour;
}

void MainActor::ConCmd_teleport(const Pentagram::istring& args)
{
	MainActor* mainactor = World::get_instance()->getMainActor();
	int curmap = mainactor->getMapNum();

	int a[4];
	int n = sscanf(args.c_str(), "%d%d%d%d", &a[0], &a[1], &a[2], &a[3]);

	switch (n) {
	case 1:
		mainactor->teleport(curmap, a[0]);
		break;
	case 2:
		mainactor->teleport(a[0], a[1]);
		break;
	case 3:
		mainactor->teleport(curmap, a[0], a[1], a[2]);
		break;
	case 4:
		mainactor->teleport(a[0], a[1], a[2], a[3]);
		break;
	default:
		pout << "teleport usage:" << std::endl;
		pout << "teleport <mapnum> <x> <y> <z>: teleport to (x,y,z) on map mapnum" << std::endl;
		pout << "teleport <x> <y> <z>: teleport to (x,y,z) on current map" << std::endl;
		pout << "teleport <mapnum> <eggnum>: teleport to target egg eggnum on map mapnum" << std::endl;
		pout << "teleport <eggnum>: teleport to target egg eggnum on current map" << std::endl;
		break;
	}
}

void MainActor::ConCmd_mark(const Pentagram::istring& args)
{
	if (args.empty()) {
		pout << "Usage: mark <mark>: set named mark to this location" << std::endl;
		return;
	}

	Configuration* config = CoreApp::get_instance()->getConfig();	
	MainActor* mainactor = World::get_instance()->getMainActor();
	int curmap = mainactor->getMapNum();
	sint32 x,y,z;
	mainactor->getLocation(x,y,z);

	Pentagram::istring confkey = "config/u8marks/" + args;
	char buf[100]; // large enough for 4 ints
	sprintf(buf, "%d %d %d %d", curmap, x, y, z);

	config->set(confkey, buf);
	config->write(); //!! FIXME: clean this up

	pout << "Set mark \"" << args.c_str() << "\" to " << buf << std::endl;
}

void MainActor::ConCmd_recall(const Pentagram::istring& args)
{
	if (args.empty()) {
		pout << "Usage: recall <mark>: recall to named mark" << std::endl;
		return;
	}

	Configuration* config = CoreApp::get_instance()->getConfig();	
	MainActor* mainactor = World::get_instance()->getMainActor();
	Pentagram::istring confkey = "config/u8marks/" + args;
	std::string target;
	config->value(confkey, target, "");
	if (target.empty()) {
		pout << "recall: no such mark" << std::endl;
		return;
	}

	int t[4];
	int n = sscanf(target.c_str(), "%d%d%d%d", &t[0], &t[1], &t[2], &t[3]);
	if (n != 4) {
		pout << "recall: invalid mark" << std::endl;
		return;
	}

	mainactor->teleport(t[0], t[1], t[2], t[3]);
}

void MainActor::ConCmd_listmarks(const Pentagram::istring& args)
{
	Configuration* config = CoreApp::get_instance()->getConfig();
	std::set<Pentagram::istring> marks;
	marks = config->listKeys("config/u8marks", false);
	for (std::set<Pentagram::istring>::iterator iter = marks.begin();
		 iter != marks.end(); ++iter)
	{
		pout << (*iter).c_str() << std::endl;
	}
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

void MainActor::getWeaponOverlay(const WeaponOverlayFrame*& frame,
								 uint32& shape)
{
	shape = 0;
	frame = 0;

	ObjId weaponid = getEquip(ShapeInfo::SE_WEAPON);
	Item* weapon = World::get_instance()->getItem(weaponid);
	if (!weapon) return;

	ShapeInfo* shapeinfo = weapon->getShapeInfo();
	if (!shapeinfo) return;

	WeaponInfo* weaponinfo = shapeinfo->weaponinfo;
	if (!weaponinfo) return;

	shape = weaponinfo->overlay_shape;

	WpnOvlayDat* wpnovlay = GameData::get_instance()->getWeaponOverlay();
	frame = wpnovlay->getOverlayFrame(lastanim, weaponinfo->overlay_type,
									  direction, animframe);

	if (frame == 0) shape = 0;
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
