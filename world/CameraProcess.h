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
	CameraProcess();
	CameraProcess(uint16 itemnum);							// Follow item/Do nothing
	CameraProcess(sint32 x, sint32 y, sint32 z);				// Goto location
	CameraProcess(sint32 x, sint32 y, sint32 z, sint32 time);	// Scroll to location

	virtual ~CameraProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	// You will notice that this isn't the same as how Item::GetLerped works
	void GetLerped(sint32 &x, sint32 &y, sint32 &z, sint32 factor, bool noupdate=false);

	//! Find the roof above the camera. 
	//! \param factor Interpolation factor for this frame
	//! \return 0 if no roof found, objid of roof if found
	uint16 FindRoof(sint32 factor);

	INTRINSIC(I_setCenterOn);
	INTRINSIC(I_move_to);
	INTRINSIC(I_scrollTo);
	INTRINSIC(I_startQuake);
	INTRINSIC(I_stopQuake);

	static void				GetCameraLocation(sint32 &x, sint32 &y, sint32 &z);
	static CameraProcess*	GetCameraProcess() { return camera; }
	static uint16			SetCameraProcess(CameraProcess *);	// Set the current camera process. Adds process. Return PID
	static void				ResetCameraProcess();

	static void				SetEarthquake(sint32 e) { 
		earthquake = e; 
		if (!e)  eq_x = eq_y = 0;
	}

	void					ItemMoved();

	virtual void terminate();	// Terminate NOW!

	bool loadData(IDataSource* ids, uint32 version);
private:
	virtual void saveData(ODataSource* ods);

	sint32 sx, sy, sz;
	sint32 ex, ey, ez;
	sint32 time;
	sint32 elapsed;
	uint16 itemnum;

	sint32 last_framenum;

	static CameraProcess	*camera;
	static sint32 earthquake;
	static sint32 eq_x, eq_y;
};

#endif //CAMERAPROCESS_H
