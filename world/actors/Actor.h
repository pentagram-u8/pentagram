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

#ifndef ACTOR_H
#define ACTOR_H

#include "Container.h"
#include "intrinsics.h"

class ActorAnimProcess;

class Actor : public Container
{
	friend class ActorAnimProcess;
public:
	Actor();
	~Actor();

	uint16 getStr() const { return strength; }
	void setStr(uint16 str) { strength = str; }
	uint16 getDex() const { return dexterity; }
	void setDex(uint16 dex) { dexterity = dex; }
	uint16 getInt() const { return intelligence; }
	void setInt(uint16 int_) { intelligence = int_; }
	uint16 getHP() const { return hitpoints; }
	void setHP(uint16 hp) { hitpoints = hp; }
	uint16 getMana() const { return mana; }
	void setMana(uint16 mp) { mana = mp; }

	uint16 getLastAnim() const { return lastanim; }
	void setLastAnim(uint16 anim) { lastanim = anim; }
	uint16 getDir() const { return direction; }
	void setDir(uint16 dir) { direction = dir; }

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(Actor);

	INTRINSIC(I_isNPC);
	INTRINSIC(I_getDir);
	INTRINSIC(I_getLastAnimSet);
	INTRINSIC(I_getStr);
	INTRINSIC(I_getDex);
	INTRINSIC(I_getInt);
	INTRINSIC(I_getHp);
	INTRINSIC(I_getMana);
	INTRINSIC(I_setStr);
	INTRINSIC(I_setDex);
	INTRINSIC(I_setInt);
	INTRINSIC(I_setHp);
	INTRINSIC(I_setMana);
	INTRINSIC(I_getMap);
	INTRINSIC(I_doAnim);

private:
	bool animating;

	uint16 strength;
	uint16 dexterity;
	uint16 intelligence;
	uint16 hitpoints;
	uint16 mana;

	uint16 lastanim;
	uint16 direction;
};


#endif
