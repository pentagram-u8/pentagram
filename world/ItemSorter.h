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

#ifndef ITEMSORTER_H
#define ITEMSORTER_H

#include <vector>

class MainShapeFlex;
class Item;
class RenderSurface;
struct SortItem;
struct Palette;

class ItemSorter
{
	MainShapeFlex	*shapes;
	RenderSurface	*surf;
	const Palette	*pal;

	uint32		max_items;
	uint32		num_items;
	SortItem	*items;
	sint32		sort_limit;

	sint32		order_counter;

public:
	ItemSorter(int Max_Items = 2048);
	~ItemSorter();

	void BeginDisplayList(RenderSurface*,const Palette *);// Begin's creating the display list
	
	void AddItem(sint32 x, sint32 y, sint32 z, uint32 shape_num, uint32 frame_num, uint32 item_flags, uint16 item_num=0);
	void AddItem(Item *);					// Add an Item. SetupLerp() MUST have been called

	void PaintDisplayList();				// Finishes the display list and Paints
	uint16 Trace(sint32 x, sint32 y);		// Trace and find an object. Returns objid 

	void IncSortLimit() { sort_limit++; }
	void DecSortLimit() { if (sort_limit > 0) sort_limit--; }

private:
	bool PaintSortItem(SortItem	*);
	bool NullPaintSortItem(SortItem	*);
};


#endif //ITEMSORTER_H
