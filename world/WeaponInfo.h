/*
Copyright (C) 2004-2005 The Pentagram team

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

#ifndef WEAPONINFO_H
#define WEAPONINFO_H


struct WeaponInfo {
	uint32 shape;
	uint8 overlay_type;
	uint32 overlay_shape;
	uint8 damage_modifier;
	uint8 base_damage;
	uint8 dex_attack_bonus;
	uint8 dex_defend_bonus;
	uint8 armour_bonus;
	uint16 damage_type;
	int treasure_chance;

	enum DmgType {
		DMG_NORMAL = 0x0001,
		DMG_BLADE  = 0x0002,
		DMG_BLUNT  = 0x0004,
		DMG_FIRE   = 0x0008,
		DMG_UNDEAD = 0x0010,
		DMG_MAGIC  = 0x0020,
		DMG_SLAYER = 0x0040,
		DMG_PIERCE = 0x0080,
		DMG_FALLING= 0x0100
	};
};


#endif
