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

#ifndef CAMERAPROCESS_H
#define CAMERAPROCESS_H

#include "Process.h"
#include "intrinsics.h"

//
// The camera process. This works in 3 ways
//
// It can be set to stay where it currently is
// It can be set to follow an item.
// It can be set to scroll to an item
// It can be set to stay at a location
//

class CameraProcess : public Process
{
public:
	CameraProcess();											// Do nothing
	CameraProcess(uint16 itemnum);								// Follow item
	CameraProcess(sint32 x, sint32 y, sint32 z);				// Goto location
	CameraProcess(sint32 x, sint32 y, sint32 z, sint32 time);	// Scroll to location

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(CameraProcess);

	virtual bool run(const uint32 framenum);

	// You will notice that this isn't the same as how Item::GetLerped works
	void GetLerped(sint32 &x, sint32 &y, sint32 &z, sint32 factor);

	INTRINSIC(I_setCenterOn);
	INTRINSIC(I_move_to);
	INTRINSIC(I_scrollTo);

private:
	sint32 sx, sy, sz;
	sint32 ex, ey, ez;
	sint32 time;
	sint32 elapsed;
	uint16 itemnum;
};

#endif //CAMERAPROCESS_H