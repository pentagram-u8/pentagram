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

#include "Item.h"
#include "Application.h"
#include "Usecode.h"
#include "GameData.h"
#include "UCMachine.h"
#include "World.h"
#include "DelayProcess.h"

/*
My current idea on how to construct items: an ItemFactory class that
creates the right kind of Item (Item, Container, ???) based on
the item that it's supposed to create. (This would be a friend of the Item
classes)

This is why the Item (and Container) constructors are currently rather empty.
-wjp
*/

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(Item,Object);

Item::Item()
	: shape(0), frame(0), x(0), y(0), z(0),
	  flags(0), quality(0), npcnum(0), mapnum(0),
	  extendedflags(0), parent(0), glob_next(0)
{

}


Item::~Item()
{

}

void Item::setLocation(sint32 X, sint32 Y, sint32 Z)
{
	x = X;
	y = Y;
	z = Z;
}

void Item::getLocation(sint32& X, sint32& Y, sint32 &Z) const
{
	X = x;
	Y = y;
	Z = z;
}

uint32 Item::callUsecodeEvent(uint32 event)
{
	uint32	class_id = shape;

	// Ok, for Permanent npcs we do it differently. they use NPC num + 1024
	if (npcnum && !(flags & FLG_DISPOSABLE)) class_id = npcnum + 1024;

	Usecode* u = GameData::get_instance()->getMainUsecode();
	uint32 offset = u->get_class_event(class_id, event);
	if (!offset) return 0;

	return callUsecode(class_id, offset, u);
}


//
// Item::setupLerp(uint32 factor)
//
// Desc: Setup the lerped info for this frame
//
// factor: Range 0 (prev) to 256 (next)
// camera: Camera object
//
void Item::setupLerp(sint32 cx, sint32 cy, sint32 cz)
{
	// Setup prev values
	l_prev = l_next;

	l_next.x = ix = x - cx;
	l_next.y = iy = y - cy;
	l_next.z = iz = z - cz;
	l_next.shape = shape;
	l_next.frame = frame;
}



uint32 Item::I_getX(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->x;
}

uint32 Item::I_getY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->y;
}

uint32 Item::I_getZ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->z;
}

uint32 Item::I_getShape(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->shape;
}

uint32 Item::I_getFrame(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->frame;
}

uint32 Item::I_getQ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	return item->quality;
}

uint32 Item::I_bark(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_STRING(str);
	if (!item) return 0;

	pout << std::endl << std::endl << str  << std::endl << std::endl;
	
	// wait 4 ticks
	return UCMachine::get_instance()->addProcess(new DelayProcess(4));

	// of course, in the final version of bark, we'll have to actually do
	// something after the timeout occurs.
	// Some kind of 'callback-delay-process' maybe?
}

uint32 Item::I_look(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(0);
}

uint32 Item::I_use(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(1);
}

uint32 Item::I_enterFastArea(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	if (!item) return 0;

	// constant?
	return item->callUsecodeEvent(15);
}

//!!!!! major hack
UCList* answerlist;
uint32 userchoice;
class UserChoiceProcess : public Process
{
public:
	virtual bool run(const uint32 /*framenum*/) {
		if (userchoice >= 0 && userchoice < answerlist->getSize()) {
			result = answerlist->getStringIndex(userchoice);
			// we're leaking strings and memory here... (not that I care)
			pout << "User answer = " << result << ": " << UCMachine::get_instance()->getString(result) << std::endl;
			userchoice = -1;
			terminate();
		}
		return false;
	}
};


uint32 Item::I_ask(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM(item);
	ARG_LIST(answers);

	if (!answers) return 0;

	// display answers and spawn new process (return pid)
	// process waits for user input and returns the selected answer (as string)

	pout << std::endl << std::endl;
	for (unsigned int i = 0; i < answers->getSize(); ++i) {
		pout << i << ": " << UCMachine::get_instance()->getString(answers->getStringIndex(i)) << std::endl;
	}

	userchoice = -1;
	answerlist = new UCList(2);
	answerlist->copyStringList(*answers);

	return UCMachine::get_instance()->addProcess(new UserChoiceProcess());
}
