/*
 *  Copyright (C) 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ContainerGump.h"

#include "Shape.h"
#include "ShapeFrame.h"
#include "Container.h"
#include "World.h"
#include "RenderSurface.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ContainerGump,ItemRelativeGump);

ContainerGump::ContainerGump(Shape* shape_, uint32 framenum_, uint16 owner,
							 uint32 Flags_, sint32 layer)
	: ItemRelativeGump(0, 0, 5, 5, owner, Flags_, layer)
{
	shape = shape_;
	framenum = framenum_;
}

ContainerGump::~ContainerGump()
{

}

void ContainerGump::InitGump()
{
	ItemRelativeGump::InitGump();

	ShapeFrame* sf = shape->getFrame(framenum);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	// Position isn't like in the original
	// U8 puts a container gump slightly to the left of an object
}

void ContainerGump::Paint(RenderSurface* surf, sint32 lerp_factor)
{
	ItemRelativeGump::Paint(surf, lerp_factor);

	Container* c = p_dynamic_cast<Container*>
		(World::get_instance()->getItem(owner));

	if (!c) {
		// Container gone!?
		Close();
		return;
	}

	std::list<Item*>& contents = c->contents;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		Item* item = *iter;
		sint32 itemx,itemy;
		item->getGumpLocation(itemx,itemy);
		GumpToParent(itemx,itemy);
		Shape* s = item->getShapeObject();
		// Where do we need to paint his item?
		// It looks like(itemx,itemy) isn't entirely correct;
		// it seems to need an extra offset.
		//   (20,14) seems to be correct for a basket, but a barrel
        //   needs (20,18)
		surf->Paint(s, item->getFrame(), itemx + 20, itemy + 18);
	}
	
}
