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

#ifndef ANIMATION_H
#define ANIMATION_H

struct Animation
{
	enum {
		walk = 0,
		run = 1,
		stand = 2,
		jumpUp = 3,
		standUp = 4,
		readyWeapon = 5,
		unreadyWeapon = 6,
		attack = 7,
		advance = 8,
		retreat = 9,
		runningJump = 10,
		shakeHead = 11,
		step = 12,
		//stepBackReachingWeapon = 13,
		// I'm fairly sure this is the animation when avatar is hit.
		stumbleBackwards = 13,
		fall = 14,
		combat_stand = 15,
		prepareJump = 16,
		jump = 17,
		airwalkJump = 18,
		//19-26: climbing up on increasingly high objects
		//27-31: casting magic
		lookLeft = 32,
		lookRight = 33,
		startKneeling = 34,
		kneel = 35,
		//36: Vividos only: magic?
		//37: Mythran only: magic?
		//38: Vividos only: ?
		//39: unused
		//40: ?
		//41: unused
		keepBalance = 42,
		//43: unused
		fallBackwards = 44,
		hang = 45,
		climb = 46,
		magicallyGlowingHands = 47,
		idle = 48,
		kneel2 = 49,
		stopKneeling = 50,
		sitDownInChair = 51,
		standUpFromChair = 52,
		talk = 53,
		//54: Mythran and Vividos only: magic?
		work = 55,
		drown = 56,
		burn = 57,
		kick = 58
		//59: blocking?
		//60: blocking?
		//61: unused
		//62: unused
		//63: unused
	};

	static bool isCombatAnim(const unsigned int anim);
	static int checkWeapon(const unsigned int nextanim,
		const unsigned int lastanim);
};

#endif
