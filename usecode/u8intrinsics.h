/*
Copyright (C) 2003-2007 The Pentagram team

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
#include "MainActor.h"
#include "Kernel.h"
#include "CameraProcess.h"
#include "GUIApp.h"
#include "Egg.h"
#include "MonsterEgg.h"
#include "CurrentMap.h"
#include "PaletteFaderProcess.h"
#include "SpriteProcess.h"
#include "MusicProcess.h"
#include "ScrollGump.h"
#include "BookGump.h"
#include "ReadableGump.h"
#include "TargetGump.h"
#include "GrantPeaceProcess.h"
#include "FireballProcess.h"
#include "HealProcess.h"
#include "InverterProcess.h"
#include "AudioProcess.h"
#include "Game.h"

// Ultima 8 Intrinsics
Intrinsic U8Intrinsics[] = {
	// 0x000
	TargetGump::I_target,
	0, //U
	Item::I_touch,
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
	Item::I_getFootpadData,
	0, //U
	Item::I_overlaps,
	Item::I_overlapsXY,
	Item::I_isOn,
	0, //U
	0, //U
	0, //U
	Item::I_ascend,
	Item::I_getWeight,
	Item::I_getWeightIncludingContents,
	Item::I_getSurfaceWeight,
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
	Item::I_isExplosive,
	Item::I_move,
	0, //U
	// 0x040
	Item::I_legalMoveToPoint,
	Item::I_legalMoveToContainer,
	Actor::I_isNPC,
	0, //U
	Item::I_hurl,
	Item::I_shoot,
	Item::I_fall,
	Item::I_grab,
	0, //U
	Item::I_bark,
	Item::I_ask,
	Item::I_getSliderInput,
	Item::I_openGump,
	Item::I_closeGump,
	0, //U
	0, //U
	// 0x050
	Item::I_getMapArray,
	0, //U
	Item::I_setMapArray,
	Item::I_receiveHit,
	Item::I_explode,
	Item::I_canReach,
	Item::I_getRange,
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
	Item::I_gotHit,
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
	Item::I_guardianBark,
	BookGump::I_readBook,
	ScrollGump::I_readScroll,
	// 0x070
	ReadableGump::I_readGrave,
	ReadableGump::I_readPlaque,
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
	Actor::I_areEnemiesNear,
	Actor::I_isInCombat,
	Actor::I_setInCombat,
	Actor::I_clrInCombat,
	Actor::I_setTarget,
	Actor::I_getTarget,
	Actor::I_setAlignment,
	Actor::I_getAlignment,
	Actor::I_setEnemyAlignment,
	Actor::I_getEnemyAlignment,
	Actor::I_isEnemy,
	Actor::I_isDead,
	Actor::I_setDead,
	Actor::I_clrDead,
	Actor::I_isImmortal,
	// 0x090
	Actor::I_setImmortal,
	Actor::I_clrImmortal,
	Actor::I_isWithstandDeath,
	Actor::I_setWithstandDeath,
	Actor::I_clrWithstandDeath,
	Actor::I_isFeignDeath,
	Actor::I_setFeignDeath,
	Actor::I_clrFeignDeath,
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
	Actor::I_pathfindToPoint,
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
	Actor::I_cSetActivity,
	Actor::I_setAirWalkEnabled,
	// 0x0B0
	Actor::I_getAirWalkEnabled,
	Actor::I_schedule,
	Actor::I_getEquip,
	Actor::I_setEquip,
	GUIApp::I_closeItemGumps,
	CameraProcess::I_scrollTo,
	UCMachine::I_urandom,
	UCMachine::I_rndRange,
	GrantPeaceProcess::I_castGrantPeace,
	UCMachine::I_numToStr,
	0, //U
	MusicProcess::I_playMusic,
	UCMachine::I_getName, //temp
	Item::I_igniteChaos,
	CameraProcess::I_setCenterOn,
	CameraProcess::I_move_to,
	// 0x0C0
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	CameraProcess::I_startQuake,
	CameraProcess::I_stopQuake,
	InverterProcess::I_invertScreen,
	0, //U
	Kernel::I_getNumProcesses,
	Kernel::I_resetRef,
	MainActor::I_teleportToEgg,
	Kernel::I_resetRef,
	0, // setRef
	GUIApp::I_getAvatarInStasis,
	// 0x0D0
	GUIApp::I_setAvatarInStasis,
	Item::I_getEtherealTop,
	GUIApp::I_getCurrentTimerTick,
	0, //U (canGetThere)
	CurrentMap::I_canExistAt,
	SpriteProcess::I_createSprite,
	SpriteProcess::I_createSprite,
	Item::I_getFamilyOfType,
	GUIApp::I_getTimeInGameHours,
	GUIApp::I_getTimeInMinutes,
	GUIApp::I_getTimeInSeconds,
	GUIApp::I_setTimeInGameHours,
	0, // U (SetTimeInMinutes)
	0, // U (SetTimeInSeconds)
	PaletteFaderProcess::I_fadeToBlack,
	PaletteFaderProcess::I_fadeFromBlack,
	// 0x0E0
	PaletteFaderProcess::I_fadeToPaletteTransform,
	PaletteFaderProcess::I_lightningBolt,
	PaletteFaderProcess::I_fadeToWhite,
	PaletteFaderProcess::I_fadeFromWhite,
	Game::I_playEndgame,
	HealProcess::I_feedAvatar,
	MainActor::I_accumulateStrength,
	MainActor::I_accumulateIntelligence,
	MainActor::I_accumulateDexterity,
	MainActor::I_clrAvatarInCombat,
	MainActor::I_setAvatarInCombat,
	MainActor::I_isAvatarInCombat,
	AudioProcess::I_playSFX,		// sint16
	AudioProcess::I_playSFX,		// sint16 uint8
	AudioProcess::I_playSFX,		// sint16 uint16 ObjID
	AudioProcess::I_playAmbientSFX, // sint16              Unused (but implemented)
	// 0x0F0
	AudioProcess::I_playAmbientSFX, // sint16 uint8        Unused (but implemented)
	AudioProcess::I_playAmbientSFX,	// sint16 uint16 ObjID
	AudioProcess::I_isSFXPlaying,
	AudioProcess::I_setVolumeSFX,
	AudioProcess::I_stopSFX,		// sint16
	AudioProcess::I_stopSFX,		// sint16 ObjID
	0, //U
	0, //U
	MusicProcess::I_musicStop,
	0, //U
	0, //U
	FireballProcess::I_TonysBalls,
	GUIApp::I_avatarCanCheat,
	GUIApp::I_makeAvatarACheater,
	UCMachine::I_true, // isGameRunning
	0, //U
	// 0x100
	0, //U
	0  //U
};

#endif
