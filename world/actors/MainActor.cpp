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
#include "Animation.h"

#include "SettingManager.h"
#include "CoreApp.h"
#include "GameData.h"
#include "WpnOvlayDat.h"
#include "ShapeInfo.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainActor,Actor);

MainActor::MainActor() : justTeleported(false), accumStr(0), accumDex(0),
						 accumInt(0)
{

}

MainActor::~MainActor()
{

}

bool MainActor::CanAddItem(Item* item, bool checkwghtvol)
{
	const unsigned int backpack_shape = 529; //!! *cough* constant

	if (!Actor::CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == objid) return true; // already in here

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack

	uint32 equiptype = item->getShapeInfo()->equiptype;
	bool backpack = (item->getShape() == backpack_shape);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		uint32 cet = (*iter)->getShapeInfo()->equiptype;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		// already have an item with the same equiptype
		if (cet == equiptype || (cbackpack && backpack)) return false;
	}

	return true;
}

bool MainActor::addItem(Item* item, bool checkwghtvol)
{
	if (!Actor::addItem(item, checkwghtvol)) return false;

	item->setFlag(FLG_EQUIPPED);

	uint32 equiptype = item->getShapeInfo()->equiptype;
	item->setZ(equiptype);

	return true;
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
	int oldmap = getMapNum();
	sint32 oldx, oldy, oldz;
	getLocation(oldx, oldy, oldz);

	World* world = World::get_instance();
	CurrentMap* currentmap = world->getCurrentMap();

	pout << "MainActor::teleport(): teleporting to map " << mapnum
		 << ", egg " << teleport_id << std::endl;

	// (attempt to) load the new map
	if (!world->switchMap(mapnum)) {
		perr << "MainActor::teleport(): switchMap() failed!" << std::endl;
		return;
	}

	// find destination
	TeleportEgg* egg = currentmap->findDestination(teleport_id);
	if (!egg) {
		perr << "MainActor::teleport(): destination egg not found!"
			 << std::endl;
		teleport(oldmap, oldx, oldy, oldz);
		return;
	}
	sint32 x,y,z;
	egg->getLocation(x,y,z);

	pout << "Found destination: " << x << "," << y << "," << z << std::endl;
	egg->dumpInfo();

	Actor::teleport(mapnum, x, y, z);
	justTeleported = true;
}

uint16 MainActor::getDefenseType()
{
	uint16 type = 0;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
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

sint16 MainActor::getDefendingDex()
{
	sint16 dex = getDex();

	Item* weapon = World::get_instance()->getItem(
		getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		ShapeInfo* si = weapon->getShapeInfo();
		assert(si->weaponinfo);
		dex += si->weaponinfo->dex_defend_bonus;
	}

	if (dex <= 0) dex = 1;

	return dex;
}

sint16 MainActor::getAttackingDex()
{
	sint16 dex = getDex();

	Item* weapon = World::get_instance()->getItem(
		getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		ShapeInfo* si = weapon->getShapeInfo();
		assert(si->weaponinfo);
		dex += si->weaponinfo->dex_attack_bonus;
	}

	return dex;
}

uint16 MainActor::getDamageType()
{
	Item* weapon = World::get_instance()->getItem(
		getEquip(ShapeInfo::SE_WEAPON));

	if (weapon) {
		// weapon equipped?

		ShapeInfo* si = weapon->getShapeInfo();
		assert(si->weaponinfo);

		return si->weaponinfo->damage_type;
	}

	return Actor::getDamageType();
}

int MainActor::getDamageAmount()
{
	int damage = 0;

	if (getLastAnim() == Animation::kick) {
		// kick

		int kick_bonus = 0;
		Item* legs = World::get_instance()->getItem(
			getEquip(ShapeInfo::SE_LEGS));
		if (legs) {
			ShapeInfo* si = legs->getShapeInfo();
			assert(si->armourinfo);
			kick_bonus = si->armourinfo[legs->getFrame()].kick_attack_bonus;
		}

		damage = (std::rand() % (getStr()/2 + 1)) + kick_bonus;

		return damage;

	}

	Item* weapon = World::get_instance()->getItem(
		getEquip(ShapeInfo::SE_WEAPON));
	
	
	if (weapon) {
		// weapon equipped?
		
		ShapeInfo* si = weapon->getShapeInfo();
		assert(si->weaponinfo);
			
		int base = si->weaponinfo->base_damage;
		int mod = si->weaponinfo->damage_modifier;
		
		damage = (std::rand() % (mod + 1)) + base + getStr()/5;
		
		return damage;
	}
	
	// no weapon?
	
	damage = (std::rand() % (getStr()/2 + 1)) + 1;
	
	return damage;
}

void MainActor::receiveHit(uint16 other, int dir, int damage,
						   uint16 damage_type)
{
	Actor::receiveHit(other, dir, damage, damage_type);

	if (damage > 3 && other)
	{
		// TODO: spray blood
	}
}

void MainActor::die(uint16 damageType)
{
	Actor::die(damageType);

	// TODO: implement this
}

void MainActor::ConCmd_teleport(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	MainActor* mainactor = World::get_instance()->getMainActor();
	int curmap = mainactor->getMapNum();

	switch (argv.size() - 1) {
	case 1:
		mainactor->teleport(curmap, 
							strtol(argv[1].c_str(), 0, 0));
		break;
	case 2:
		mainactor->teleport(strtol(argv[1].c_str(), 0, 0), 
							strtol(argv[2].c_str(), 0, 0));
		break;
	case 3:
		mainactor->teleport(curmap, 
							strtol(argv[1].c_str(), 0, 0), 
							strtol(argv[2].c_str(), 0, 0), 
							strtol(argv[3].c_str(), 0, 0));
		break;
	case 4:
		mainactor->teleport(strtol(argv[1].c_str(), 0, 0), 
							strtol(argv[2].c_str(), 0, 0), 
							strtol(argv[3].c_str(), 0, 0), 
							strtol(argv[4].c_str(), 0, 0));
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

void MainActor::ConCmd_mark(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size() == 1) {
		pout << "Usage: mark <mark>: set named mark to this location" << std::endl;
		return;
	}

	SettingManager* settings = SettingManager::get_instance();
	MainActor* mainactor = World::get_instance()->getMainActor();
	int curmap = mainactor->getMapNum();
	sint32 x,y,z;
	mainactor->getLocation(x,y,z);

	Pentagram::istring confkey = "marks/" + argv[1];
	char buf[100]; // large enough for 4 ints
	sprintf(buf, "%d %d %d %d", curmap, x, y, z);

	settings->set(confkey, buf);
	settings->write(); //!! FIXME: clean this up

	pout << "Set mark \"" << argv[1].c_str() << "\" to " << buf << std::endl;
}

void MainActor::ConCmd_recall(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (args.size() == 1) {
		pout << "Usage: recall <mark>: recall to named mark" << std::endl;
		return;
	}

	SettingManager* settings = SettingManager::get_instance();
	MainActor* mainactor = World::get_instance()->getMainActor();
	Pentagram::istring confkey = "marks/" + argv[1];
	std::string target;
	if (!settings->get(confkey, target)) {
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

void MainActor::ConCmd_listmarks(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	SettingManager* settings = SettingManager::get_instance();
	std::vector<Pentagram::istring> marks;
	marks = settings->listDataKeys("marks");
	for (std::vector<Pentagram::istring>::iterator iter = marks.begin();
		 iter != marks.end(); ++iter)
	{
		pout << (*iter) << std::endl;
	}
}

void MainActor::ConCmd_maxstats(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	MainActor* mainactor = World::get_instance()->getMainActor();

	// constants!!
	mainactor->setStr(25);
	mainactor->setDex(25);
	mainactor->setInt(25);
	mainactor->setHP(mainactor->getMaxHP());
	mainactor->setMana(mainactor->getMaxMana());
}

void MainActor::accumulateStr(int n)
{
	// already max?
	if (strength == 25) return; //!! constant

	accumStr += n;
	if (accumStr >= 650 || std::rand() % (650 - accumStr) == 0) { //!! constant
		strength++;
		accumStr = 0;
	}
}

void MainActor::accumulateDex(int n)
{
	// already max?
	if (dexterity == 25) return; //!! constant

	accumDex += n;
	if (accumDex >= 650 || std::rand() % (650 - accumDex) == 0) { //!! constant
		dexterity++;
		accumDex = 0;
	}
}

void MainActor::accumulateInt(int n)
{
	// already max?
	if (intelligence == 25) return; //!! constant

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
