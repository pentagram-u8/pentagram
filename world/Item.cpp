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
	  extendedflags(0), parent(0)
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

void Item::callUsecodeEvent(uint32 event)
{
	Usecode* u = Application::get_instance()->getMainUsecode();

	callUsecode(shape, u->get_class_event(shape, event), u);
}


//
// Item::setupLerp(uint32 factor)
//
// Desc: Setup the lerped info for this frame
//
// factor: Range 0 (prev) to 256 (next)
// camera: Camera object
//
void Item::setupLerp(/* Camera &camera */)
{
	// Setup prev values
	l_prev = l_next;

	l_next.x = ix = x /*- camera.x*/;
	l_next.y = iy = y /*- camera.y*/;
	l_next.z = iz = z /*- camera.z*/;
	l_next.shape = shape;
	l_next.frame = frame;
}

