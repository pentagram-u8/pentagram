/*
 *  Copyright (C) 2002 The Pentagram Team
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

#ifndef CONVERTUSECODEU8_H
#define CONVERTUSECODEU8_H

#include "Convert.h"

class ConvertUsecodeU8 : public ConvertUsecode
{
	public:
		const char* const *intrinsics()  { return _intrinsics;  };
		const char* const *event_names() { return _event_names; };
		void readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset);
		void readevents(IFileDataSource *ucfile, const UsecodeHeader &uch)
		{
			for (uint32 i=0; i<32; ++i)
			{
				uint32 offset = read4(ucfile);
				EventMap[offset] = i;
				#ifdef DISASM_DEBUG
				pout << "Event " << i << ": " << std::hex << std::setw(4) << offset << std::dec << endl;
				#endif
			}
		}

	
	private:
		static const char* const _intrinsics[];
		static const char* const _event_names[];
};

const char* const ConvertUsecodeU8::_intrinsics[] = {
	// 0000
	"target()",
	"Item::getNext()",							// Unused
	"Item::touch()",
	"short Item::getX()",
	"short Item::getY()",
	"short Item::getZ()",
	"short Item::getCX()",
	"short Item::getCY()",
	"short Item::getCZ()",
	"short Item::getGumpX()",					// Unused
	"short Item::getGumpY()",					// Unused
	"void Item::setGumpXY(short x, short y)",	// Unused
	"Item::getPoint(WorldPoint&)",
	"ushort Item::getType()",
	"void Item::setType(ushort type)",
	"ushort Item::getFrame()",
	// 0010
	"void Item::setFrame(ushort frame)",
	"ushort Item::getQuality()",				// Get Q (Not generic familiy)
	"ushort Item::getUnkEggType()",				// Get Q (UnkEgg family)
	"ushort Item::getQuantity()",				// Get Q (Quantity or Reagent family)
	"Item::getContents()",						// Unused
	"Item::getContainer()",
	"Item::getRootContainer()",
	"ushort Item::getGlobNum()",				// Get Q (GlobEgg family) - Unused
	"void Item::setGlobNum(uint)",				// Set Q (GlobEgg family) - Unused
	"ushort Item::getQ()",						// Get Q
	"void Item::setQ(uint)",					// Set Q
	"ushort Item::setQuality(short value)",		// Get Q (Not generic familiy)
	"void Item::setUnkEggType(short value)",	// Set Q (UnkEgg family) - Unused
	"void Item::setQuantity(short value)",		// Set Q (Quantity and Reagent families)
	"short Item::getFamily()",
	"bool Item::getTypeFlag(short bit)",		// Get TypeFlag (0 to 63 - not a mask!)
	// 0020
	"short Item::getStatus()",					// Get Status Flags
	"void Item::orStatus(short mask)",			// Status |= mask;
	"void Item::andStatus(short mask)",			// Status &= mask;
	"Item::getFootpad(short&, short&, short&)",
	"Item::touches(ushort)",					// Unused
	"Item::overlaps(ushort)",
	"Item::overlapsXY(ushort)",					// Unused
	"Item::isOn(ushort)",
	"Item::isCompletelyOn(ushort)",				// Unused
	"Item::isAbove(ushort)",					// Unused
	"Item::isUnder(ushort)",					// Unused
	"Item::ascend(short)",
	"Item::getWeight()",
	"Item::getWeightIncludingContents()",
	"Item::getSurfaceWeight()",
	"Item::getVolume()",						// Unused
	// 0030
	"Item::getCapacity()",						// Unuses
	"Item::legal_create(ushort, ushort, ushort, ushort, ushort)",
	"Item::create(ushort, ushort)",
	"Item::legal_create(ushort, ushort, WorldPoint&)",
	"Item::legal_create(ushort, ushort, ushort, short)",
	"Item::push()",
	"Item::pop(ushort, ushort, uchar)",
	"Item::pop(ushort)",
	"Item::pop()",
	"Item::popToEnd(ushort)",
	"Item::destroy()",
	"Item::removeContents()",
	"Item::destroyContents()",
	"Item::isExplosive()",
	"Item::move(ushort, ushort, uchar)",
	"Item::move(WorldPoint&)",					// Unused
	// 0040
	"Item::legal_move(WorldPoint&, ushort, ushort)",
	"Item::legal_move(ushort&, ushort)",
	"Item::isNpc()",
	"Item::isInNpc()",							// Unused
	"Item::hurl(short, short, short, short)",
	"Item::shoot(WorldPoint&, short, short)",
	"Item::fall()",
	"Item::grab()",
	"Item::findTarget(int, int)",				// Unused
	"process Item::bark(char* str)",
	"strptr process Item::ask(ushort slist)",
	"int Item::getSliderInput(int min, int max, int step)",
	"Item::openGump(short)",
	"Item::closeGump()",
	"Item::isGumpOpen()",						// Unused
	"Item::getNpcArray()",						// Unused
	// 0050
	"Item::getMapArray()",
	"Item::setNpcArray(short)",					// Unused
	"Item::setMapArray(short)",
	"Item::receiveHit(ushort, char, short, ushort)",
	"Item::explode()",
	"Item::canReach(ushort, short)",
	"Item::getRange(ushort)",
	"Item::getRange(ushort, ushort, ushort)",	// Unused
	"Item::getDirToCoords(ushort, ushort)",
	"Item::getDirFromCoords(ushort, ushort)",
	"Item::getDirToItem(ushort)",
	"Item::getDirFromItem(ushort)",
	"process Item::look()",						// Call event 0x0
	"process Item::use()",						// Call event 0x1
	"Item::anim()",								// Unused
	"Item::cachein()",							// Unused
	// 0060
	"Item::hit(ushort, short)",					// Unused
	"process Item::gotHit(ushort, short)",		// Call event ???
	"Item::release()",							// Unused
	"Item::equip()",							// Unused
	"Item::unequip()",							// Unused
	"Item::combine()",							// Unused
	"Item::calledFromAnim()",					// Unused
	"process Item::enterFastArea()",			// Call event 0xF
	"Item::leaveFastArea()",					// Unused
	"Item::cast(ushort)",						// Unused
	"Item::justMoved()",						// Unused
	"Item::AvatarStoleSomething(ushort)",		// Unused
	"Item::animGetHit(ushort)",					// Unused
	"process Item::guardianBark(int)",			// Call even 0x15
	"Book::read(char*)",
	"Scroll::read(char*)",
	// 0070
	"Grave::read(short,char*)",
	"Plaque::read(short,char*)",
	"Egg::getEggXRange()",
	"Egg::getEggYRange()",
	"Egg::setEggXRange(ushort)",
	"Egg::setEggYRange(ushort)",				// Unused
	"Egg::getEggId()",
	"Egg::setEggId(ushort)",
	"Egg::hatch()",								// Unused
	"MonsterEgg::hatch()",
	"MonsterEgg::getMonId()",
	"MonsterEgg::getActivity()",				// Unused
	"MonsterEgg::getShapeType()",				// Unused
	"MonsterEgg::setMonId(short)",				// Unused
	"MonsterEgg::setActivity(short)",			// Unused
	"MonsterEgg::setShapeType(short)",			// Unused
	// 0080
	"Npc::isBusy()",
	"Npc::areEnemiesNear()",
	"Npc::isInCombat()",
	"Npc::setInCombat()",
	"Npc::clrInCombat()",
	"Npc::setTarget(ushort)",
	"Npc::getTarget()",
	"Npc::setAlignment(uchar)",
	"Npc::getAlignment()",						// Unused
	"Npc::setEnemyAlignment(uchar)",
	"Npc::getEnemyAlignment()",					// Unused
	"Npc::isEnemy(ushort)",
	"Npc::isDead()",
	"Npc::setDead()",
	"Npc::clrDead()",
	"Npc::isImmortal()",						// Unused
	// 0090
	"Npc::setImmortal()",
	"Npc::clrImmortal()",
	"Npc::isWithstandDeath()",					// Unused
	"Npc::setWithstandDeath()",
	"Npc::clrWithstandDeath()",
	"Npc::isFeignDeath()",						// Unused
	"Npc::setFeignDeath()",
	"Npc::clrFeignDeath()",						// Unused
	"Npc::freeEquip(ushort)",					// Unused
	"Npc::clearEquip()",						// Unused
	"Npc::getNpcSlot()",						// Unused
	"Npc::freeNpcSlot()",						// Unused
	"Npc::getDir()",
	"Npc::getMap()",
	"Npc::teleport(ushort, ushort, uchar, uchar)",
	"Npc::doAnim(AnimSet, short, short, uchar)",
	// 00A0
	"Npc::getLastAnimSet()",
	"Npc::pathfind(ushort, ushort, ushort, ushort)",
	"Npc::pathfind(ushort, ushort)",
	"char Npc::getStr()",
	"char Npc::getInt()",
	"char Npc::getDex()",
	"uchar Npc::getHp()",
	"short Npc::getMana()",
	"void Npc::setStr(char str)",
	"void Npc::setInt(char int)",
	"void Npc::setDex(char dex)",
	"void Npc::setHp(uchar hp)",
	"void Npc::setMana(short mana)",
	"Npc::create(ushort, ushort)",
	"Npc::cSetActivity(Activity)",
	"Npc::setAirWalkEnabled(uchar)",
	// 00B0
	"Npc::getAirWalkEnabled()",
	"Npc::schedule(ulong)",
	"Npc::getEquip(short)",
	"Npc::setEquip(short, ushort)",
	"closeAllGumps()",
	"Camera::scrollTo(ushort, ushort, uchar, int)",
	"urandom(int)",
	"rndRange(short,short)",
	"castGrantPeaceSpell()",
	"numToStr(ushort)",
	"strToNum(char*)",							// Unused
	"playMusic(char)",
	"getName()",								// Returns the Avatar's name
	"igniteChaos(ushort, ushort, uchar)",
	"Camera::setCenterOn(ushort)",
	"Camera::move_to(ushort, ushort, uchar, short)",
	// 00C0
	"Camera::move_rel(int, int, int)",			// Unused
	"Camera::set_roof(short)",					// Unused
	"Camera::roof()",							// Unused
	"Camera::getX()",							// Unused
	"Camera::getY()",							// Unused
	"Camera::getZ()",							// Unused
	"Camera::startQuake(short)",
	"Camera::stopQuake()",
	"Camera::invertScreen(uchar)",
	"U8MousePointer::getDir()",					// Unused
	"Kernel::getNumProcesses(ushort, ProcessType)",
	"Kernel::resetRef(ushort, ProcessType)",
	"teleportToEgg(short, int, uchar)",
	"resetRef(ushort, ushort)",
	"setRef(ushort, ushort, ushort)",
	"getAvatarInStasis()",
	// 00D0
	"setAvatarInStasis(short)",
	"getEtherealTop()",
	"getCurrentTimerTick()",
	"canGetThere(ushort, ushort, ushort)",		// Unused
	"canExistAt(ushort, ushort, ushort, ushort, uchar, ushort, short)",
	"createSprite(short, short, short, short, short, short, ushort, ushort, uchar)",
	"createSprite(short, short, short, short, ushort, ushort, uchar)",
	"short getFamilyOfType(ushort type)",
	"TimeInGameHours()",
	"TimeInMinutes()",
	"TimeInSeconds()",							// Unused
	"SetTimeInGameHours(short)",
	"SetTimeInMinutes(long)",					// Unused
	"SetTimeInSeconds(long)",					// unused
	"FadeToBlack()",
	"FadeFromBlack()",
	// 00E0
	"FadeToPalette(short, short)",
	"LightningBolt()",
	"FadeToWhite()",
	"FadeFromWhite()",
	"playEndgame()",
	"FeedAvatar(short)",
	"AccumulateStrength(short)",				// Unused
	"AccumulateIntelligence(short)",
	"AccumulateDexterity(short)",
	"ClrAvatarInCombat()",
	"SetAvatarInCombat()",
	"IsAvatarInCombat()",						// Unused
	"playSFX(int)",
	"playSFX(int, uchar)",
	"playSFX(int, int, ushort)",
	"playAmbientSFX(int)",						// Unused
	// 00F0
	"playAmbientSFX(int, int)",					// Unused
	"playAmbientSFX(int, int, ushort)",
	"isSFXPlaying(int)",
	"setVolumeSFX(int, int)",
	"stopSFX(int)",
	"stopSFX(int, ushort)",
	"soundInit(int, int, int)",					// Unused
	"soundDeInit()",							// Unused
	"musicStop()",
	"musicSlowStop()",							// Unused
	"musicPlay(int)",							// Unused
	"TonysBalls(short, short, ushort, ushort, ushort)",
	"AvatarCanCheat()",
	"MakeAvatarACheater()",
	"isGameRunning()",
	"unused",
	// 0100
	"unused",
	0
};

const char * const ConvertUsecodeU8::_event_names[] = {
	"look()",						// 0x00
	"use()",						// 0x01
	"anim()",						// 0x02
	"setActivity()",				// 0x03
	"cachein()",					// 0x04
	"hit(ushort,short)",			// 0x05
	"gotHit(ushort,short)",			// 0x06
	"hatch()",						// 0x07
	"schedule()",					// 0x08
	"release()",					// 0x09
	"equip()",						// 0x0A
	"unequip()",					// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"cast(ushort)",					// 0x11
	"justMoved()",					// 0x12
	"AvatarStoleSomething(ushort)",	// 0x13
	"animGetHit()",					// 0x14
	"guardianBark(int)",			// 0x15
	"func16",						// 0x16
	"func17",						// 0x17
	"func18",						// 0x18
	"func19",						// 0x19
	"func1A",						// 0x1A
	"func1B",						// 0x1B
	"func1C",						// 0x1C
	"func1D",						// 0x1D
	"func1E",						// 0x1E
	"func1F",						// 0x1F
	0
};

void ConvertUsecodeU8::readheader(IFileDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)
{
	#ifdef DISASM_DEBUG
	perr << std::setfill('0') << std::hex;
	perr << "unknown1: " << std::setw(4) << read4(ucfile) << endl; // unknown
	uch.maxOffset = read4(ucfile) - 0x0C; // file size
	perr << "maxoffset: " << std::setw(4) << maxOffset << endl;
	perr << "unknown2: " << std::setw(4) << read4(ucfile) << endl; // unknown
	curOffset = 0;
	#else
	read4(ucfile); // unknown
	uch.maxOffset = read4(ucfile) - 0x0C; // file size
	read4(ucfile); // unknown
	curOffset = 0;
	#endif
};

#endif
