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

#include "pent_include.h"

#include "CameraProcess.h"
#include "Application.h"
#include "World.h"
#include "Item.h"
#include "Actor.h"
#include "UCMachine.h"
#include "ShapeInfo.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(CameraProcess,Process);

// Do nothing, or follow avatar, reletive
CameraProcess::CameraProcess() : 
	time(0), elapsed(0), itemnum(0)
{
	Application::get_instance()->GetCamera(sx,sy,sz);
	ex = sx; ey = sy; ez = sz;
}

// Track item
CameraProcess::CameraProcess(uint16 _itemnum) : 
	time(0), elapsed(0), itemnum(_itemnum)
{
	Application::get_instance()->GetCamera(sx,sy,sz);
	Object *obj = World::get_instance()->getObject(itemnum);

	// Got it
	if (obj) 
	{
		Item *item = p_dynamic_cast<Item*>(obj);

		// Got it
		if (item)
		{
			item->getLocation(ex,ey,ez);
			ez += 20;
		}
	}
}

// Stay over point
CameraProcess::CameraProcess(sint32 _x, sint32 _y, sint32 _z) : 
	ex(_x), ey(_y), ez(_z), time(0), elapsed(0), itemnum(0)
{
	Application::get_instance()->GetCamera(sx,sy,sz);
}

// Scroll
CameraProcess::CameraProcess(sint32 _x, sint32 _y, sint32 _z, sint32 _time) : 
	ex(_x), ey(_y), ez(_z), time(_time), elapsed(0), itemnum(0)
{
	Application::get_instance()->GetCamera(sx,sy,sz);
	pout << "Scrolling from (" << sx << "," << sy << "," << sz << ") to (" <<
		ex << "," << ey << "," << ez << ") in " << time << " frames" << std::endl;
}

bool CameraProcess::run(const uint32 /* framenum */)
{
	if (time && elapsed>time)
	{
		result = 0; // do we need this

		// This is a bit of a hack. If we are looking at avatar,
		// then follow avatar
		Actor *av = World::get_instance()->getNPC(1);
		sint32 ax, ay, az;
		av->getLocation(ax,ay,az);

		if (ex == ax && ey == ay && ez == (az+20))
		{
			pout << "Hack to scroll with Avatar" << std::endl;
			Application::get_instance()->SetCameraProcess(new CameraProcess(1));
		}
		else 
			Application::get_instance()->SetCameraProcess(0);	// This will terminate us
		return false;
	}

	elapsed++;

	if (time == 0)
	{
		sx = ex;
		sy = ey;
		sz = ez;
		return true;
	}

	return true;
}

void CameraProcess::GetLerped(sint32 &x, sint32 &y, sint32 &z, sint32 factor)
{
	if (itemnum)
	{
		Object *obj = World::get_instance()->getObject(itemnum);

		// Got it
		if (obj) 
		{
			Item *item = p_dynamic_cast<Item*>(obj);

			// Got it
			if (item)
			{
				item->getLocation(ex,ey,ez);
				ez += 20;
			}
		}
	}

	if (time == 0)
	{
		if (factor == 256)
		{
			x = ex; y = ey; z = ez;
		}
		else if (factor == 0)
		{
			x = sx; y = sy; z = sz;
		}
		else
		{
			// This way while possibly slower is more accurate
			x = ((sx*(256-factor) + ex*factor)>>8);
			y = ((sy*(256-factor) + ey*factor)>>8);
			z = ((sz*(256-factor) + ez*factor)>>8);
		}
	}
	else
	{
		// Do a quadratic interpolation here, but not yet
		sint32 sfactor = elapsed;
		sint32 efactor = elapsed+1;

		if (sfactor > time) sfactor = time;
		if (efactor > time) efactor = time;

		sint32 lsx = ((sx*(time-sfactor) + ex*sfactor)/time);
		sint32 lsy = ((sy*(time-sfactor) + ey*sfactor)/time);
		sint32 lsz = ((sz*(time-sfactor) + ez*sfactor)/time);

		sint32 lex = ((sx*(time-efactor) + ex*efactor)/time);
		sint32 ley = ((sy*(time-efactor) + ey*efactor)/time);
		sint32 lez = ((sz*(time-efactor) + ez*efactor)/time);

		// This way while possibly slower is more accurate
		x = ((lsx*(256-factor) + lex*factor)>>8);
		y = ((lsy*(256-factor) + ley*factor)>>8);
		z = ((lsz*(256-factor) + lez*factor)>>8);
	}
}

//	"Camera::move_to(uword, uword, ubyte, word)",
uint32 CameraProcess::I_move_to(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	ARG_SINT16(unk);
	Application::get_instance()->SetCameraProcess(new CameraProcess(x,y,z));
	return 0;
}

//	"Camera::setCenterOn(uword)",
uint32 CameraProcess::I_setCenterOn(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(itemnum);
	if (!itemnum) Application::get_instance()->SetCameraProcess(new CameraProcess(1));
	else Application::get_instance()->SetCameraProcess(new CameraProcess(itemnum));

	return 0;
}

//	Camera::scrollTo(uword, uword, ubyte, word)
uint32 CameraProcess::I_scrollTo(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	ARG_SINT16(unk);
	return Application::get_instance()->SetCameraProcess(new CameraProcess(x,y,z, 5));
}

