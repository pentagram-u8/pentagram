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

#ifndef MAINACTOR_H
#define MAINACTOR_H

#include "Actor.h"

struct WeaponOverlayFrame;

class MainActor : public Actor
{
public:
	MainActor();
	virtual ~MainActor();

	//! teleport to the given location on the given map
	virtual void teleport(int mapnum, sint32 x, sint32 y, sint32 z);

	//! teleport to a teleport-destination egg
	//! \param mapnum The map to teleport to
	//! \param teleport_id The ID of the egg to teleport to
	void teleport(int mapnum, int teleport_id); // to teleportegg

	bool hasJustTeleported() const { return justTeleported; }
	void setJustTeleported(bool t) { justTeleported = t; }

	//! accumulate a little bit of strength. When you reach 650 you gain
	//! one strength point. (There's a chance you gain strength sooner)
	void accumulateStr(int n);

	//! accumulate a little bit of dexterity. When you reach 650 you gain
	//! one dex. point. (There's a chance you gain dex. sooner)
	void accumulateDex(int n);

	//! accumulate a little bit of intelligence. When you reach 650 you gain
	//! one int. point. (There's a chance you gain int. sooner)
	void accumulateInt(int n);

	bool loadData(IDataSource* ids);

	//! "teleport" console command
	static void MainActor::ConCmd_teleport(const Pentagram::istring& args);
	//! "mark" console command
	static void MainActor::ConCmd_mark(const Pentagram::istring& args);
	//! "recall" console command
	static void MainActor::ConCmd_recall(const Pentagram::istring& args);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	INTRINSIC(I_teleportToEgg);
	INTRINSIC(I_accumulateStrength);
	INTRINSIC(I_accumulateDexterity);
	INTRINSIC(I_accumulateIntelligence);

protected:
	virtual void saveData(ODataSource* ods);

	void getWeaponOverlay(const WeaponOverlayFrame*& frame, uint32& shape);

	bool justTeleported;

	int accumStr;
	int accumDex;
	int accumInt;
};

#endif
