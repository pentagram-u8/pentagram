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

#ifndef WPNOVLAYDAT_H
#define WPNOVLAYDAT_H

class RawArchive;
struct AnimWeaponOverlay;
struct WeaponOverlayFrame;

#include <vector>

class WpnOvlayDat
{
public:
	WpnOvlayDat();
	~WpnOvlayDat();

	//! load weapon overlay data from wpnovlay.dat
	//! NB: anim.dat must have already been read
	void load(RawArchive* overlaydat);

	const AnimWeaponOverlay* getAnimOverlay(uint32 action) const;
	const WeaponOverlayFrame* getOverlayFrame(uint32 action, int type,
										int direction, int frame) const;

private:
	std::vector<AnimWeaponOverlay*> overlay;
};


#endif
