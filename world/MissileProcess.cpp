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

#include "pent_include.h"

#include "MissileProcess.h"
#include "Item.h"
#include "World.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MissileProcess,Process);

MissileProcess::MissileProcess()
	: Process()
{

}

MissileProcess::MissileProcess(Item* item_, sint32 to_x_, sint32 to_y_,
								 sint32 to_z_, int speed_, bool curve_)
{
	assert(item_);
	item_num = item_->getObjId();
	
	to_x = to_x_;
	to_y = to_y_;
	to_z = to_z_;

	item_->getLocation(from_x, from_y, from_z);

	speed = speed_*5;
	curve = curve_;

	currentpos = 0;
}

bool MissileProcess::run(const uint32 framenum)
{
	// some values to make items use a nice arc :-)
	// (No, this isn't permanent, but it makes the fish fly nicely ;-) )
	static const int sinvals[21] = 
		{0, 156, 309, 454, 588, 705, 809, 891, 952, 987, 1000,
		 987, 952, 891, 809, 705, 588, 454, 309, 156, 0};

	Item *it = World::get_instance()->getItem(item_num);

	if (!it) {
		// item gone
		terminate();
		return false;
	}

	sint32 x, y, z;

	currentpos += 1;
	if (currentpos > (2500/speed)) currentpos = (2500/speed);

	//!! speed doesn't work the way it (probably) should...
	x = from_x + (((to_x - from_x) * currentpos) * speed) / 2500;
	y = from_y + (((to_y - from_y) * currentpos) * speed) / 2500;
	z = from_z + (((to_z - from_z) * currentpos) * speed) / 2500;
	if (curve) z += sinvals[(20*currentpos*speed)/2500]/25;

	it->collideMove(x,y,z,false,true);	// Ick, should be false for force 

	if (currentpos >= (2500/speed)) {
		result = 1; // do we need this? For now it's for the pathfinding hack
		terminate();
	}

	return true;
}

void MissileProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(from_x));
	ods->write4(static_cast<uint32>(from_y));
	ods->write4(static_cast<uint32>(from_z));
	ods->write4(static_cast<uint32>(to_x));
	ods->write4(static_cast<uint32>(to_y));
	ods->write4(static_cast<uint32>(to_z));
	ods->write4(static_cast<uint32>(speed));
	ods->write4(static_cast<uint32>(currentpos));

	uint8 c = (curve ? 1 : 0);
	ods->write1(c);

}

bool MissileProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	from_x = static_cast<int>(ids->read4());
	from_y = static_cast<int>(ids->read4());
	from_z = static_cast<int>(ids->read4());
	to_x = static_cast<int>(ids->read4());
	to_y = static_cast<int>(ids->read4());
	to_z = static_cast<int>(ids->read4());
	speed = static_cast<int>(ids->read4());
	currentpos = static_cast<int>(ids->read4());

	curve = (ids->read1() != 0);

	return true;
}
