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

#ifndef U8INTRINSICS_H
#define U8INTRINSICS_H

#include "intrinsics.h"


#include "Item.h"
#include "UCMachine.h"
#include "Container.h"
#include "Actor.h"
#include "Kernel.h"
#include "CameraProcess.h"
#include "GUIApp.h"
#include "Egg.h"
#include "MonsterEgg.h"

// Ultima 8 Intrinsics
Intrinsic U8Intrinsics[] = {
	// 0x000
	UCMachine::I_target,
	0, //U
	0, // Item::touch
	Item::I_getX,
	Item::I_getY,
	Item::I_getZ,
	Item::I_getCX,
	Item::I_getCY,
	Item::I_getCZ,
	0, //U
	0, //U
	0, //U
	Item::I_getPoint,
	Item::I_getShape,
	Item::I_setShape,
	Item::I_getFrame,
	// 0x010
	Item::I_setFrame,
	Item::I_getQuality,
	Item::I_getUnkEggType,
	Item::I_getQuantity,
	0, //U
	Item::I_getContainer,
	Item::I_getRootContainer,
	0, //U
	0, //U
	Item::I_getQ,
	Item::I_setQ,
	Item::I_setQuality,
	0, //U
	Item::I_setQuantity,
	Item::I_getFamily,
	Item::I_getTypeFlag,
	// 0x020
	Item::I_getStatus,
	Item::I_orStatus,
	Item::I_andStatus,
	Item::I_getFootpad,
	0, //U
	Item::I_overlaps,
	Item::I_overlapsXY,
	Item::I_isOn,
	0, //U
	0, //U
	0, //U
	0, // Item::ascend
	Item::I_getWeight,
	Item::I_getWeightIncludingContents,
	0, // Item::getSurfaceWeight
	0, //U
	// 0x030
	0, //U
	Item::I_legalCreateAtCoords,
	Item::I_create,
	Item::I_legalCreateAtPoint,
	Item::I_legalCreateInCont,
	Item::I_push,
	Item::I_popToCoords,
	Item::I_popToContainer,
	Item::I_pop,
	Item::I_popToEnd,
	Item::I_destroy,
	Container::I_removeContents,
	Container::I_destroyContents,
	0, // Item::isExplosive
	Item::I_move,
	0, //U
	// 0x040
	0, // Item::legal_move
	0, // Item::legal_move
	Actor::I_isNPC,
	0, //U
	0, // Item::hurl
	Item::I_shoot,
	0, // Item::fall
	0, // Item::grab
	0, //U
	Item::I_bark,
	Item::I_ask,
	0, // Item::getSliderInput
	Item::I_openGump,
	Item::I_closeGump,
	0, //U
	0, //U
	// 0x050
	Item::I_getMapArray,
	0, //U
	Item::I_setMapArray,
	0, // Item::receiveHit
	0, // Item::explode
	0, // Item::canReach
	0, // Item::getRange
	0, //U
	Item::I_getDirToCoords,
	Item::I_getDirFromCoords,
	Item::I_getDirToItem,
	Item::I_getDirFromItem,
	Item::I_look,
	Item::I_use,
	0, //U
	0, //U
	// 0x060
	0, //U
	UCMachine::I_dummyProcess, //!! process Item::gotHit(uword, word)
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	Item::I_enterFastArea,
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	UCMachine::I_dummyProcess, //!! process Item::guardianBark(word)
	UCMachine::I_dummyProcess, //!! process Book::read(char*)
	UCMachine::I_dummyProcess, //!! process Scroll::read(char*)
	// 0x070
	UCMachine::I_dummyProcess, //!! process Grave::read(word,char*)
	UCMachine::I_dummyProcess, //!! process Plaque::read(word,char*)
	Egg::I_getEggXRange,
	Egg::I_getEggYRange,
	Egg::I_setEggXRange,
	Egg::I_setEggYRange,
	Egg::I_getEggId,
	Egg::I_setEggId,
	0, //U
	MonsterEgg::I_monsterEggHatch,
	MonsterEgg::I_getMonId,
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	// 0x080
	Actor::I_isBusy,
	0, // Npc::areEnemiesNear
	Actor::I_isInCombat,
	0, // Npc::setInCombat
	0, // Npc::clrInCombat
	0, // Npc::setTarget
	0, // Npc::getTarget
	0, // Npc::setAlignment
	0, // Npc::getAlignment
	0, // Npc::setEnemyAlignment
	0, // Npc::getEnemyAlignment
	0, // Npc::isEnemy
	Actor::I_isDead,
	0, // Npc::setDead
	0, // Npc::clrDead
	Actor::I_isImmortal,
	// 0x090
	0, // Npc::setImmortal
	0, // Npc::clrImmortal
	Actor::I_isWithstandDeath,
	0, // Npc::setWithstandDeath
	0, // Npc::clrWithstandDeath
	Actor::I_isFeignDeath,
	0, // Npc::setFeignDeath
	0, // Npc::clrFeignDeath
	0, //U
	0, //U
	0, //U
	0, //U
	Actor::I_getDir,
	Actor::I_getMap,
	Actor::I_teleport,
	Actor::I_doAnim,
	// 0x0A0
	Actor::I_getLastAnimSet,
	UCMachine::I_dummyProcess, //!! process Npc::pathfind(uword, uword, uword, uword)
	Actor::I_pathfindToItem,
	Actor::I_getStr,
	Actor::I_getInt,
	Actor::I_getDex,
	Actor::I_getHp,
	Actor::I_getMana,
	Actor::I_setStr,
	Actor::I_setInt,
	Actor::I_setDex,
	Actor::I_setHp,
	Actor::I_setMana,
	Actor::I_createActor,
	UCMachine::I_dummyProcess, //!! process Npc::cSetActivity(Activity)
	0, // Npc::setAirWalkEnabled
	// 0x0B0
	0, // Npc::getAirWalkEnabled
	0, // Npc::schedule
	0, // Npc::getEquip
	0, // Npc::setEquip
	0, // closeAllGumps
	CameraProcess::I_scrollTo, //!! process Camera::scrollTo(uword, uword, ubyte, word)
	UCMachine::I_urandom,
	UCMachine::I_rndRange,
	0, // castGrantPeaceSpell
	UCMachine::I_numToStr,
	0, //U
	0, // playMusic
	UCMachine::I_getName, //temp
	0, // igniteChaos
	CameraProcess::I_setCenterOn,
	CameraProcess::I_move_to,
	// 0x0C0
	0, // .........
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Kernel::I_getNumProcesses,
	Kernel::I_resetRef,
	UCMachine::I_dummyProcess, //!! process teleportToEgg(word, word, ubyte)
	Kernel::I_resetRef,
	0,
	GUIApp::I_getAvatarInStasis,
	// 0x0D0
	GUIApp::I_setAvatarInStasis,
	Item::I_getEtherealTop,
	GUIApp::I_getCurrentTimerTick,
	0,
	0,
	0,
	0,
	Item::I_getFamilyOfType,
	0,
	0,
	0,
	0,
	0,
	0,
	UCMachine::I_dummyProcess, //!! process FadeToBlack()
	UCMachine::I_dummyProcess, //!! process FadeFromBlack()
	// 0x0E0
	UCMachine::I_dummyProcess, //!! process FadeToPalette(word, word)
	UCMachine::I_dummyProcess, //!! process LightningBolt()
	UCMachine::I_dummyProcess, //!! process FadeToWhite()
	UCMachine::I_dummyProcess, //!! process FadeFromWhite()
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	// 0x0F0
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	UCMachine::I_AvatarCanCheat,
	0,
	UCMachine::I_isGameRunning,
	0,
	// 0x100
	0,
	0
};

#endif
