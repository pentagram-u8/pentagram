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
#include "ItemSorter.h"

#include "Item.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "ShapeInfo.h"
#include "MainShapeFlex.h"
#include "RenderSurface.h"
#include "Rect.h"
#include "GameData.h"

// This does NOT need to be in the header

struct SortItem
{
	SortItem() : item_num(0), shape(0), order(-1), depends() { }

	sint32					item_num;	// Owner item number

	Shape					*shape;
	uint32					frame;
	uint32					flags;		// Item flags flags

	int						sx, sx2;	// Screenspace X coords
	int						sy, sy2;	// Screenspace Y coords

	/*
				Bounding Box layout

		   1    
		 /   \      
	   /       \     1 = Left  Far  Top LFT --+
	 2           3   2 = Left  Near Top LNT -++
	 | \       / |   3 = Right Far  Top RFT +-+
	 |   \   /   |   4 = Right Near Top RNT +++
	 |     4     |   5 = Left  Near Bot LNB -+-
	 |     |     |   6 = Right Far  Bot RFB +--
	 5     |     6   7 = Right Near Bot LNB ++- 
	   \   |   /     8 = Left  Far  Bot LFB --- (not shown)
		 \ | /    
		   7    

	*/

	sint32	x, xleft;	// Worldspace bounding box x (xright = x)
	sint32	y, yfar;	// Worldspace bounding box y (ynear = y)
	sint32	z, ztop;	// Worldspace bounding box z (ztop = z)

	sint32	sxleft;		// Screenspace bounding box left extent    (LNT x coord)
	sint32	sxright;	// Screenspace bounding box right extent   (RFT x coord)

	sint32	sxtop;		// Screenspace bounding box top x coord    (LFT x coord)
	sint32	sytop;		// Screenspace bounding box top extent     (LFT y coord)

	sint32	sxbot;		// Screenspace bounding box bottom x coord (RNB x coord) ss origin
	sint32	sybot;		// Screenspace bounding box bottom extent  (RNB y coord) ss origin

	bool	f32x32 : 1;			// Needs 1 bit	0
	bool	flat : 1;			// Needs 1 bit	1
	bool	occl : 1;			// Needs 1 bit	2
	bool	solid : 1;			// Needs 1 bit	3
	bool	draw : 1;			// Needs 1 bit	4
	bool	roof : 1;			// Needs 1 bit	5
	bool	noisy : 1;			// Needs 1 bit	6
	bool	anim : 1;			// Needs 1 bit	7
	bool	trans : 1;			// Needs 1 bit	8

	bool	occluded : 1;		// Set true if occluded

	sint16	clipped;			// Clipped to RenderSurface

	sint32	order;		// Rendering order. -1 is not yet drawn

	std::vector<SortItem *>	depends;	// All this Items dependencies (i.e. all objects behind)

	// Functions

	// Check to see if we overlap si2
	inline bool overlap(const SortItem &si2) const;

	// Check to see if we occlude si2
	inline bool occludes(const SortItem &si2) const;

	// Operator less than
	bool operator<(const SortItem& si2) const;

};

typedef std::vector<SortItem *> SortItemVector;

// Check to see if we overlap si2
inline bool SortItem::overlap(const SortItem &si2) const
{
	const int point_top_diff[2] = {	sxtop - si2.sxbot, sytop - si2.sybot };
	const int point_bot_diff[2] = {	sxbot - si2.sxtop, sybot - si2.sytop };

	// This function is a bit of a hack. It uses dot products between
	// points and the lines. Nothing is normalized since that isn't 
	// important

	// 'normal' of top  left line ( 2,-1) of the bounding box
	sint32 dot_top_left = point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of top right line ( 2, 1) of the bounding box
	sint32 dot_top_right = -point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of bot  left line (-2,-1) of the bounding box
	sint32 dot_bot_left =  point_bot_diff[0] - point_bot_diff[1] * 2;

	// 'normal' of bot right line (-2, 1) of the bounding box
	sint32 dot_bot_right = -point_bot_diff[0] - point_bot_diff[1] * 2;

	bool right_clear = sxright <= si2.sxleft;
	bool left_clear = sxleft >= si2.sxright;
	bool top_left_clear = dot_top_left >= 0;
	bool top_right_clear = dot_top_right >= 0;
	bool bot_left_clear = dot_bot_left >= 0;
	bool bot_right_clear = dot_bot_right >= 0;

	bool clear = right_clear | left_clear | 
		bot_right_clear | bot_left_clear |
		top_right_clear | top_left_clear;

	return !clear;
}

// Check to see if we occlude si2
inline bool SortItem::occludes(const SortItem &si2) const
{
	const int point_top_diff[2] = {	sxtop - si2.sxtop, sytop - si2.sytop };
	const int point_bot_diff[2] = {	sxbot - si2.sxbot, sybot - si2.sybot };

	// This function is a bit of a hack. It uses dot products between
	// points and the lines. Nothing is normalized since that isn't 
	// important

	// 'normal' of top left line ( 2, -1) of the bounding box
	sint32 dot_top_left = point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of top right line ( 2, 1) of the bounding box
	sint32 dot_top_right = -point_top_diff[0] + point_top_diff[1] * 2;

	// 'normal' of bot  left line (-2,-1) of the bounding box
	sint32 dot_bot_left =  point_bot_diff[0] - point_bot_diff[1] * 2;

	// 'normal' of bot right line (-2, 1) of the bounding box
	sint32 dot_bot_right = -point_bot_diff[0] - point_bot_diff[1] * 2;


	bool right_res = sxright >= si2.sxright;
	bool left_res = sxleft <= si2.sxleft;
	bool top_left_res = dot_top_left <= 0;
	bool top_right_res = dot_top_right <= 0;
	bool bot_left_res = dot_bot_left <= 0;
	bool bot_right_res = dot_bot_right <= 0;

	bool occluded = right_res & left_res & 
		bot_right_res & bot_left_res &
		top_right_res & top_left_res;

	return occluded;
}

bool SortItem::operator<(const SortItem& si2) const
{
	const SortItem& si1 = *this;
	
	// Specialist z flat handling
	if (si1.flat && si2.flat)
	{
		// Differing z is easy for flats
		if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;

		// Equal z

		// Animated always gets drawn after
		if (si1.anim != si2.anim) return si1.anim < si2.anim;

		// Trans always gets drawn after
		if (si1.trans != si2.trans) return si1.trans < si2.trans;

		// Draw always gets drawn first
		if (si1.draw != si2.draw) return si1.draw > si2.draw;

		// Solid always gets drawn first
		if (si1.solid != si2.solid) return si1.solid > si2.solid;

		// Occludes always get drawn first
		if (si1.occl != si2.occl) return si1.occl > si2.occl;

		// 32x32 flats get drawn first
		if (si1.f32x32 != si2.f32x32) return si1.f32x32 > si2.f32x32;

		// Clearly in x and y?
		//if (si1.x <= si2.xleft && si1.y <= si2.yfar) return true;
		//else if (si1.xleft >= si2.x && si1.yfar >= si2.y) return false;

		// Clearly in x?
		if (si1.x <= si2.xleft) return true;
		else if (si1.xleft >= si2.x) return false;

		// Clearly in y?
		if (si1.y <= si2.yfar) return true;
		else if (si1.yfar >= si2.y) return false;
	}
	// Mixed, or non flat
	else {

		// Clearly X, Y and Z
		//if (si1.x <= si2.xleft && si1.y <= si2.yfar && si1.ztop <= si2.z) return true;
		//else if (si1.xleft >= si2.x && si1.yfar >= si2.y && si1.z >= si2.ztop) return false;

		//  Clearly X and Y
		//if (si1.x <= si2.xleft && si1.y <= si2.yfar) return true;
		//else if (si1.xleft >= si2.x && si1.yfar >= si2.y) return false;

		int front1 = si1.x + si1.y;
		int rear1 = si1.xleft + si1.yfar;
		int front2 = si2.x + si2.y;
		int rear2 = si2.xleft + si2.yfar;

		// Rear of object is infront of other's front
		if (front1 <= rear2) return true;
		else if (rear1 >= front2) return false;

		// Clearly in z
		if (si1.ztop <= si2.z) return true;
		else if (si1.z >= si2.ztop) return false;

		// Clearly X
		if (si1.x <= si2.xleft) return true;
		else if (si1.xleft >= si2.x) return false;

		// Clearly Y
		if (si1.y <= si2.yfar) return true;
		else if (si1.yfar >= si2.y) return false;
/*

		// Clearly in x?
		if (si1.xd || si2.xd) {
			if (si1.x <= si2.xleft) return true;
			else if (si1.xleft >= si2.x) return false;
		}
		else {
			if (si1.x != si2.x) return si1.x < si2.x;
		}

		// Clearly in y?
		if (si1.yd || si2.yd) {
		//	if (si1.y <= si2.yfar) return true;
		//	else if (si1.yfar >= si2.y) return false;
			if (si1.y != si2.y) return si1.y < si2.y;
		}
		else {
			if (si1.y != si2.y) return si1.y < si2.y;
		}
*/
		{
			// Clearly in x and y?
			//if (si1.x <= si2.xleft && si1.y <= si2.yfar) return true;
			//else if (si1.xleft >= si2.x && si1.yfar >= si2.y) return false;

		}

		// Partial in z
		//if (si1.ztop != si2.ztop) return si1.ztop < si2.ztop;
	}

	// Biased Clearly in z
	if (si1.ztop-4 <= si2.z) return true;
	else if (si1.z >= si2.ztop-4) return false;

	// Biased Clearly X
	if (si1.x-4 <= si2.xleft) return true;
	else if (si1.xleft >= si2.x-4) return false;

	// Biased Clearly Y
	if (si1.y-4 <= si2.yfar) return true;
	else if (si1.yfar >= si2.y-4) return false;

	// Partial in X + Y front
//	if (si1.x + si1.y != si1.x + si2.y) return (si1.x + si1.y < si2.x + si2.y);

	// Partial in X + Y back
//	if (si1.xleft + si1.yfar != si1.xleft + si2.yfar) return (si1.xleft + si1.yfar < si2.xleft + si2.yfar);

	// Partial in x?
//	if (si1.x != si2.x) return si1.x < si2.x;

	// Partial in y?
//	if (si1.y != si2.y) return si1.y < si2.y;

	// Just sort by shape number - not a number any more (is a pointer)
	if (si1.shape != si2.shape) return si1.shape < si2.shape;

	// And then by frame
	return si1.frame < si2.frame;
}

// 
// ItemSorter
//

ItemSorter::ItemSorter(int Max_Items) : 
		shapes(0), surf(0), max_items(Max_Items), num_items(0)
{
	if (max_items <= 0) max_items = Max_Items = 2048;
	items = new SortItem [max_items];
}

ItemSorter::~ItemSorter()
{
	delete [] items;
}

void ItemSorter::BeginDisplayList(RenderSurface *rs, const Palette *palette)
{
	// Get the shapes, if required
	if (!shapes) shapes = GameData::get_instance()->getMainShapes();

	// Set the RenderSurface, and reset the item list
	surf = rs;
	pal = palette;
	num_items = 0;
	order_counter = 0;
}

void ItemSorter::AddItem(Item *add)
{
	//if (add->iz > skip_lift) return;
	//if (Application::tgwds && shape == 538) return;

	// First thing, get a SortItem to use
	
	if (num_items >= max_items) return;

	SortItem *si = items+num_items;

	uint32 shape_num = add->getShape();

	si->item_num = add->getObjId();
	si->shape = shapes->getShape(shape_num);
	si->frame = add->getFrame();
	ShapeFrame *frame = si->shape->getFrame(si->frame);

	ShapeInfo *info = shapes->getShapeInfo(shape_num);

	//if (info->is_editor && !show_editor_items) return;
	//if (info->z > shape_max_height) return;

	// Dimensions
	sint32 xd, yd, zd;
	si->flags = add->getFlags();

	// X and Y are flipped
	if (si->flags & Item::FLG_FLIPPED) 
	{
		xd = info->y * 32;	// Multiply by 32 to get actual world size
		yd = info->x * 32;	// Multiply by 32 to get actual world size
	}
	else
	{
		xd = info->x * 32;	// Multiply by 32 to get actual world size
		yd = info->y * 32;	// Multiply by 32 to get actual world size
	}

	zd = info->z * 8;	// Multiply by 8 to get actual world size

	// Worldspace bounding box
	add->getLerped(si->x, si->y, si->z);
	si->xleft = si->x - xd;
	si->yfar = si->y - yd;
	si->ztop = si->z + zd;

	// Screenspace bounding box left extent    (LNT x coord)
	si->sxleft = (si->xleft - si->y)/4;
	// Screenspace bounding box right extent   (RFT x coord)
	si->sxright= (si->x - si->yfar)/4;

	// Screenspace bounding box top x coord    (LFT x coord)
	si->sxtop = (si->xleft - si->yfar)/4;
	// Screenspace bounding box top extent     (LFT y coord)
	si->sytop = (si->xleft + si->yfar)/8 - si->ztop;

	// Screenspace bounding box bottom x coord (RNB x coord)
	si->sxbot = (si->x - si->y)/4;
	// Screenspace bounding box bottom extent  (RNB y coord)
	si->sybot = (si->x + si->y)/8 - si->z;

//	si->sxleft += swo2;
//	si->sxright += swo2;
//	si->sxbot += swo2;
//	si->sxtop += swo2;

//	si->sytop += sho2;
//	si->sybot += sho2;

	// Real Screenspace coords
	si->sx = si->sxbot - frame->xoff;	// Left
	si->sy = si->sybot - frame->yoff;	// Top
	si->sx2 = si->sx + frame->width;	// Right
	si->sy2 = si->sy + frame->height;	// Bottom

	// Do Clipping here
	si->clipped = surf->CheckClipped(Rect (si->sx,si->sy, frame->width, frame->height));
	if (si->clipped < 0) return;

	// These help out with sorting. We calc them now, so it will be faster
	si->f32x32 = xd==128 && yd==128;
	si->flat = zd == 0;

	/*
	if (Application::tgwds) {
		si->draw = false;
		si->solid = false;
		si->occl = false;
		si->roof = false;
		si->noisy = false;
		si->anim = false;
		si->trans = false;
	}
	else
	*/
	{
		si->draw = info->is_draw();
		si->solid = info->is_solid();
		si->occl = info->is_occl();
		si->roof = info->is_roof();
		si->noisy = info->is_noisy();
		si->anim = info->animtype != 0;
		si->trans = info->is_translucent();
	}

	si->occluded = false;
	si->order = -1;

	// We will clear all the vector memory
	// Stictly speaking the vector will sort of leak memory, since they
	// are never deleted
	si->depends.clear();

	// Iterate the list and compare shapes

	// Ok, 
	for (SortItem * si2 = items; si2 != si; ++si2)
	{
		// Doesn't overlap
		if (si2->occluded || !si->overlap(*si2)) continue;

		// Attempt to find which is infront
		if (*si < *si2)
		{
			// si2 occludes ss
			if (si2->occl && si2->occludes(*si))
			{
				// No need to do any more checks, this isn't visible
				// Also, since it's occluded we don't need to add it to the list
				return;
			}

			// si1 is behind si2, so add it to si2's dependency list
			si2->depends.push_back(si);
		}
		else
		{
			// ss occludes si2. Sadly, we can't remove it from the list.
			if (si->occl && si->occludes(*si2)) si2->occluded = true;
			// si2 is behind si1, so add it to si1's dependency list
			else si->depends.push_back(si2);
		}
	}

	if (!si->shape->getPalette()) si->shape->setPalette(pal);

	// Incrementing num_items adds the Item to the list
	num_items ++;
}

void ItemSorter::PaintDisplayList()
{
	SortItem *it = items;
	SortItem *end = items + num_items;
	order_counter = 0;	// Reset the order_counter
	while (it != end)
	{
		if (it->order == -1) if (PaintSortItem(it)) return;

		++it;
	}
}

bool ItemSorter::PaintSortItem(SortItem	*si)
{
	// Don't paint this, or dependencies if occluded
	if (si->occluded) return false;

	// Resursion, detection
	si->order = -2;
	
	// Iterate through our dependancies, and paint them, if possible
	SortItemVector::iterator it = si->depends.begin();
	SortItemVector::iterator end = si->depends.end();
	while (it != end)
	{
		// Well, it can't. Implies recursive sorting. Can happen though so
		// you had best leave this commented out
		//if ((*it)->order == -2) CANT_HAPPEN_MSG("Recursive item sorting");

		if ((*it)->order == -1) if (PaintSortItem((*it))) return true;

		++it;
	}

	// Set our painting order
	si->order = order_counter;
	order_counter++;

	// Now paint us!

//	if (wire) si->info->draw_box_back(s, dispx, dispy, 255);

	if (si->flags & Item::FLG_FLIPPED)
		surf->PaintMirrored(si->shape, si->frame, si->sxbot, si->sybot, si->trans);
	else if (si->trans)
		surf->PaintTranslucent(si->shape, si->frame, si->sxbot, si->sybot);
	else if (!si->clipped)
		surf->PaintNoClip(si->shape, si->frame, si->sxbot, si->sybot);
	else
		surf->Paint(si->shape, si->frame, si->sxbot, si->sybot);
		
//	if (wire) si->info->draw_box_front(s, dispx, dispy, 255);

	return false;
}

bool ItemSorter::NullPaintSortItem(SortItem	*si)
{
	// Don't paint this, or dependencies if occluded
	if (si->occluded) return false;

	// Resursion, detection
	si->order = -2;
	
	// Iterate through our dependancies, and paint them, if possible
	SortItemVector::iterator it = si->depends.begin();
	SortItemVector::iterator end = si->depends.end();
	while (it != end)
	{
		// Well, it can't. Implies recursive sorting. Can happen though so
		// you had best leave this commented out
		//if ((*it)->order == -2) CANT_HAPPEN_MSG("Recursive item sorting");

		if ((*it)->order == -1) if (NullPaintSortItem((*it))) return true;

		++it;
	}

	// Set our painting/sorting order
	si->order = order_counter;
	order_counter++;

	return false;
}

uint16 ItemSorter::Trace(sint32 x, sint32 y)
{
	SortItem *it;
	SortItem *end;
	SortItem *selected;

	if (!order_counter)	// If no order_counter we need to sort the items
	{
		it = items;
		end = items + num_items;
		order_counter = 0;	// Reset the order_counter
		while (it != end)
		{
			if (it->order == -1) if (NullPaintSortItem(it)) break;

			++it;
		}
	}

	// Ok, this is all pretty simple. We iterate all the items.
	// We then check to see if the item has a point where the trace goes.
	// Finally we then set the selected SortItem if it's 'order' is highest
	it = items;
	selected = 0;
	end = items + num_items;

	for (;it != end;++it)
	{
		// Doesn't Overlap
		if (x < it->sx || x >= it->sx2 || y < it->sy || y >= it->sy2) continue;

		// Now check the frame itself
		ShapeFrame *frame = it->shape->getFrame(it->frame);

		// Nope, doesn't have a point
		if (!frame->hasPoint(x-it->sxbot, y-it->sybot)) continue;

		// Ok now check against selected
		if (!selected|| (it->order > selected->order)) selected = it;
	}

	if (selected) return selected->item_num;
	
	return 0;
}


// End of file
