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

// Ultima 8 Intrinsics
Intrinsic U8Intrinsics[] = {
	// 0x000
	UCMachine::I_target,
	0,
	0,
	Item::I_getX,
	Item::I_getY,
	Item::I_getZ,
	Item::I_getCX,
	Item::I_getCY,
	Item::I_getCZ,
	0,
	0,
	0,
	Item::I_getPoint,
	Item::I_getShape,
	Item::I_setShape,
	Item::I_getFrame,
	// 0x010
	Item::I_setFrame,
	Item::I_getQuality,
	Item::I_getUnkEggType,
	Item::I_getQuantity,
	0,
	Item::I_getContainer,
	Item::I_getRootContainer,
	0,
	0,
	Item::I_getQ,
	Item::I_setQ,
	Item::I_setQuality,
	0,
	Item::I_setQuantity,
	Item::I_getFamily,
	Item::I_getTypeFlag,
	// 0x020
	Item::I_getStatus,
	Item::I_orStatus,
	Item::I_andStatus,
	Item::I_getFootpad,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Item::I_getWeight,
	Item::I_getWeightIncludingContents,
	0,
	0,
	// 0x030
	0,
	Item::I_legalCreateAtCoords,//the 'legalCreate's are just educated guesses
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
	0,
	0,
	0,
	// 0x040
	0,
	0,
	Actor::I_isNPC,
	0,
	0,
	0,
	0,
	0,
	0,
	Item::I_bark,
	Item::I_ask,
	0,
	0,
	0,
	0,
	0,
	// 0x050
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
	Item::I_look,
	Item::I_use,
	0,
	0,
	// 0x060
	0,
	UCMachine::I_dummyProcess, //!! process Item::gotHit(uword, word)
	0,
	0,
	0,
	0,
	0,
	Item::I_enterFastArea,
	0,
	0,
	0,
	0,
	0,
	UCMachine::I_dummyProcess, //!! process Item::guardianBark(word)
	UCMachine::I_dummyProcess, //!! process Book::read(char*)
	UCMachine::I_dummyProcess, //!! process Scroll::read(char*)
	// 0x070
	UCMachine::I_dummyProcess, //!! process Grave::read(word,char*)
	UCMachine::I_dummyProcess, //!! process Plaque::read(word,char*)
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
	0,
	0,
	// 0x080
	0,
	0,
	Actor::I_isInCombat,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Actor::I_isDead,
	0,
	0,
	Actor::I_isImmortal,
	// 0x090
	0,
	0,
	Actor::I_isWithstandDeath,
	0,
	0,
	Actor::I_isFeignDeath,
	0,
	0,
	0,
	0,
	0,
	0,
	Actor::I_getDir,
	Actor::I_getMap,
	0,
	Actor::I_doAnim,
	// 0x0A0
	Actor::I_getLastAnimSet,
	UCMachine::I_dummyProcess, //!! process Npc::pathfind(uword, uword, uword, uword)
	UCMachine::I_dummyProcess, //!! process Npc::pathfind(uword, uword)
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
	0,
	UCMachine::I_dummyProcess, //!! process Npc::cSetActivity(Activity)
	0,
	// 0x0B0
	0,
	0,
	0,
	0,
	0,
	UCMachine::I_dummyProcess, //!! process Camera::scrollTo(uword, uword, ubyte, word)
	UCMachine::I_urandom,
	UCMachine::I_rndRange,
	0,
	UCMachine::I_numToStr,
	0,
	0,
	UCMachine::I_getName, //temp
	0,
	0,
	0,
	// 0x0C0
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
	Kernel::I_getNumProcesses,
	Kernel::I_resetRef,
	UCMachine::I_dummyProcess, //!! process teleportToEgg(word, word, ubyte)
	Kernel::I_resetRef,
	0,
	0,
	// 0x0D0
	0,
	Item::I_getEtherealTop,
	0,
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
	UCMachine::I_AvatarCanCheat, // temp.
	0,
	0,
	0,
	// 0x100
	0,
	0
};

#endif
