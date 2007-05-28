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

#include "pent_include.h"
#include "Animation.h"

namespace Animation
{

bool isCombatAnim(const Sequence anim)
{
	switch (anim) {
		case combatStand:
		case readyWeapon:
		case advance:
		case retreat:
		case attack:
		case kick:
		case startBlock:
		case stopBlock:
			return true;
		default:
			return false;
	}
}

/** determines if we need to ready or unready our weapon */
Sequence checkWeapon(const Sequence nextanim,
const Sequence lastanim)
{
	Sequence anim = nextanim;
	if (isCombatAnim(nextanim) && ! isCombatAnim(lastanim))
	{
		anim = readyWeapon;
	}
	else if (! isCombatAnim(nextanim) && isCombatAnim(lastanim))
	{
		anim = unreadyWeapon;
	}
	return anim;
}

};
