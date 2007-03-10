/*
Copyright (C) 2004-2006 The Pentagram team

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

#ifndef WEAPONOVERLAY_H
#define WEAPONOVERLAY_H

#include <vector>

struct WeaponOverlayFrame {
	sint32 xoff;
	sint32 yoff;
	uint32 frame;
};

struct WeaponOverlay {
	unsigned int dircount;
	std::vector<WeaponOverlayFrame>* frames; // 8 or 16 directions

        WeaponOverlay() { frames = NULL; }
	~WeaponOverlay() { delete[] frames; }
};

struct AnimWeaponOverlay {
	//! get the weapon overlay info for a specific animation frame
	//! \param type the overlay type
	//! \param direction the direction
	//! \param frame the animation frame
	//! \return 0 if invalid, or pointer to a frame; don't delete it.
	const WeaponOverlayFrame* getFrame(unsigned int type,
									   unsigned int direction,
									   unsigned int frame) const {
		if (type >= overlay.size()) return 0;
		if (direction >= overlay[type].dircount) return 0;
		if (frame >= overlay[type].frames[direction].size()) return 0;
		return &(overlay[type].frames[direction][frame]);
	}

	std::vector<WeaponOverlay> overlay;
};


#endif
