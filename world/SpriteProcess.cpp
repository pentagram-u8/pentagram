/*
Copyright (C) 2003-2005 The Pentagram team

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
#include "SpriteProcess.h"
#include "ItemFactory.h"
#include "Item.h"
#include "CurrentMap.h"
#include "Kernel.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(SpriteProcess,Process);

SpriteProcess::SpriteProcess()
	: Process()
{

}

SpriteProcess::SpriteProcess(int Shape, int Frame, int LastFrame, 
							 int Repeats, int Delay, int X, int Y, int Z,
							 bool delayed_init) :
	shape(Shape), frame(Frame), first_frame(Frame), last_frame(LastFrame), 
	repeats(Repeats), delay(Delay*2), x(X), y(Y), z(Z), delay_counter(0),
	initialized(false)
{
	if (!delayed_init)
		init();
}

void SpriteProcess::init()
{
	Item *item = ItemFactory::createItem(shape, frame, 
						0, Item::FLG_DISPOSABLE, 0, 0, Item::EXT_SPRITE, true);
	item->move(x,y,z);
	setItemNum(item->getObjId());
	initialized = true;
}

SpriteProcess::~SpriteProcess(void)
{
	Item *item = getItem(item_num);
	if (item) item->destroy();
}

void SpriteProcess::run()
{
	if (!initialized) init();

	Item *item = getItem(item_num);
	
	if (!item || (frame > last_frame && repeats==1 && !delay_counter)) 
	{
		terminate();
		return;
	}

	if (delay_counter)
	{
		delay_counter = (delay_counter+1)%delay;
		return;
	}

	if (frame > last_frame ) 
	{
		frame = first_frame;
		repeats--;
	}

	item->setFrame(frame);
	frame++;
	delay_counter = (delay_counter+1)%delay;
}

// createSprite(shape, frame, end,               delay, x, y, z);
// createSprite(shape, frame, end, unk, repeats, delay, x, y, z);
uint32 SpriteProcess::I_createSprite(const uint8* args, unsigned int argsize)
{
	int repeats = 1;
	ARG_SINT16(shape);
	ARG_SINT16(frame);
	ARG_SINT16(last_frame);

	if (argsize == 18)
	{
		ARG_SINT16(unknown);
		ARG_SINT16(repeats_count);
		repeats = repeats_count;
	}

	ARG_SINT16(delay);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT8(z);
	Process *p = new SpriteProcess(shape, frame, last_frame, repeats, delay, x, y, z);
	return Kernel::get_instance()->addProcess(p);
}

void SpriteProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(shape));
	ods->write4(static_cast<uint32>(frame));
	ods->write4(static_cast<uint32>(first_frame));
	ods->write4(static_cast<uint32>(last_frame));
	ods->write4(static_cast<uint32>(repeats));
	ods->write4(static_cast<uint32>(delay));
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));
	ods->write4(static_cast<uint32>(delay_counter));
	ods->write1(initialized ? 1 : 0);
}

bool SpriteProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	shape = static_cast<int>(ids->read4());
	frame = static_cast<int>(ids->read4());
	first_frame = static_cast<int>(ids->read4());
	last_frame = static_cast<int>(ids->read4());
	repeats = static_cast<int>(ids->read4());
	delay = static_cast<int>(ids->read4());
	x = static_cast<int>(ids->read4());
	y = static_cast<int>(ids->read4());
	z = static_cast<int>(ids->read4());
	delay_counter = static_cast<int>(ids->read4());
	initialized = (ids->read1() != 0);

	return true;
}
