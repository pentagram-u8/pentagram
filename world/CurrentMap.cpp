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

#include "CurrentMap.h"
#include "Map.h"
#include "Item.h"
#include "GlobEgg.h"
#include "Egg.h"
#include "Actor.h"
#include "World.h"
#include "Rect.h"
#include "Container.h"
#include "UCList.h"
#include "ShapeInfo.h"
#include "TeleportEgg.h"
#include "EggHatcherProcess.h"
#include "Kernel.h"
#include "GameData.h"
#include "MainShapeFlex.h"
#include "GUIApp.h"
#include "GameMapGump.h"
#include "IDataSource.h"
#include "ODataSource.h"

using std::list; // too messy otherwise
using Pentagram::Rect;
typedef list<Item*> item_list;

CurrentMap::CurrentMap()
	: current_map(0), egghatcher(0),
		fast_x_min(-1), fast_y_min(-1), fast_x_max(-1), fast_y_max(-1)
{
	items = new list<Item*>*[MAP_NUM_CHUNKS];
	fast = new uint32*[MAP_NUM_CHUNKS];
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		items[i] = new list<Item*>[MAP_NUM_CHUNKS];
		fast[i] = new uint32[MAP_NUM_CHUNKS/32];
		std::memset(fast[i],false,sizeof(uint32)*MAP_NUM_CHUNKS/32);
	}
}


CurrentMap::~CurrentMap()
{
//	clear();

	//! get rid of constants
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		delete[] items[i];
	}
	delete[] items;
}

void CurrentMap::clear()
{
	//! get rid of constants
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
				delete *iter;
			items[i][j].clear();
		}
	}

	current_map = 0;

	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();
	egghatcher = 0;
}

uint32 CurrentMap::getNum() const
{
	if (current_map == 0)
		return 0;

	return current_map->mapnum;
}

void CurrentMap::createEggHatcher()
{
	// get rid of old one, if any
	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();

    ehp = new EggHatcherProcess();
	egghatcher = Kernel::get_instance()->addProcess(ehp);
}

void CurrentMap::writeback()
{
	if (!current_map)
		return;

	//! constants

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin(); iter != items[i][j].end(); ++iter)
			{
				Item* item = *iter;

				// item is being removed from the CurrentMap item lists
				item->clearExtFlag(Item::EXT_INCURMAP);

				// delete all fast only and disposable items
				if ((item->getFlags() & Item::FLG_FAST_ONLY) ||
					(item->getFlags() & Item::FLG_DISPOSABLE))
				{
					delete item;
					continue;
				}

				// Reset the egg
				Egg* egg = p_dynamic_cast<Egg*>(item);
				if (egg) {
					egg->reset();
				}

				// this item isn't from the Map. (like NPCs)
				if (item->getFlags() & Item::FLG_IN_NPC_LIST)
					continue;

				item->clearObjId();
				if (item->getExtFlags() & Item::EXT_FIXED) {
					// item came from fixed
					current_map->fixeditems.push_back(item);
				} else {
					current_map->dynamicitems.push_back(item);
				}
			}
			items[i][j].clear();
		}
	}

	// delete egghatcher
	Process* ehp = Kernel::get_instance()->getProcess(egghatcher);
	if (ehp)
		ehp->terminate();
	egghatcher = 0;
}

void CurrentMap::loadItems(list<Item*> itemlist)
{
	item_list::iterator iter;
	for (iter = itemlist.begin(); iter != itemlist.end(); ++iter)
	{
		Item* item = *iter;

		item->assignObjId();

		// No fast area for you!
		item->clearFlag(Item::FLG_FASTAREA);

		// add item to internal object list
		addItemToEnd(item);
		
		item->callUsecodeEvent_cachein();
	}
}

void CurrentMap::loadMap(Map* map)
{
	current_map = map;

	createEggHatcher();

	// Clear fast area
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		std::memset(fast[i],false,sizeof(uint32)*MAP_NUM_CHUNKS/32);
	}
	fast_x_min = -1;
	fast_y_min = -1;
	fast_x_max = -1;
	fast_y_max = -1;

	loadItems(map->fixeditems);
	loadItems(map->dynamicitems);

	// we take control of the items in map, so clear the pointers
	map->fixeditems.clear();
	map->dynamicitems.clear();

	// load relevant NPCs to the item lists
	// !constant
	for (uint16 i = 0; i < 256; ++i) {
		Actor* actor = World::get_instance()->getNPC(i);

		// Schedule
		if (actor) actor->callUsecodeEvent_schedule();

		if (actor && actor->getMapNum() == getNum()) {
			addItemToEnd(actor);

			// Cachein
			actor->callUsecodeEvent_cachein();
		}
	}
}

void CurrentMap::addItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	//! constants
	if (ix < 0 || ix >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		iy < 0 || iy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / MAP_CHUNK_SIZE;
	sint32 cy = iy / MAP_CHUNK_SIZE;

	items[cx][cy].push_front(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg* egg = p_dynamic_cast<Egg*>(item);
	if (egg) {
		EggHatcherProcess* ehp = p_dynamic_cast<EggHatcherProcess*>(Kernel::get_instance()->getProcess(egghatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::addItemToEnd(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	//! constants
	if (ix < 0 || ix >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		iy < 0 || iy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << ix << "," << iy << ")" << std::endl;
		return;
	}

	sint32 cx = ix / MAP_CHUNK_SIZE;
	sint32 cy = iy / MAP_CHUNK_SIZE;

	items[cx][cy].push_back(item);
	item->setExtFlag(Item::EXT_INCURMAP);

	Egg* egg = p_dynamic_cast<Egg*>(item);
	if (egg) {
		EggHatcherProcess* ehp = p_dynamic_cast<EggHatcherProcess*>(Kernel::get_instance()->getProcess(egghatcher));
		assert(ehp);
		ehp->addEgg(egg);
	}
}

void CurrentMap::removeItem(Item* item)
{
	sint32 ix, iy, iz;

	item->getLocation(ix, iy, iz);

	removeItemFromList(item, ix, iy);
}


void CurrentMap::removeItemFromList(Item* item, sint32 oldx, sint32 oldy)
{
	//! This might a bit too inefficient
	// if it's really a problem we could change the item lists into sets
	// or something, but let's see how it turns out

	//! constants
	if (oldx < 0 || oldx >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS || 
		oldy < 0 || oldy >= MAP_CHUNK_SIZE*MAP_NUM_CHUNKS) {
		perr << "Skipping item: out of range (" 
			 << oldx << "," << oldy << ")" << std::endl;
		return;
	}

	sint32 cx = oldx / MAP_CHUNK_SIZE;
	sint32 cy = oldy / MAP_CHUNK_SIZE;

	items[cx][cy].remove(item);
	item->clearExtFlag(Item::EXT_INCURMAP);
}

void CurrentMap::updateFastArea(sint32 from_x, sint32 from_y, sint32 to_x, sint32 to_y)
{
	from_x /= MAP_CHUNK_SIZE;
	from_y /= MAP_CHUNK_SIZE;
	to_x /= MAP_CHUNK_SIZE;
	to_y /= MAP_CHUNK_SIZE;

	int x_min = from_x;
	int x_max = to_x;

	if (x_max < x_min)  {
		x_min = to_x;
		x_max = from_x;
	}

	int y_min = from_y;
	int y_max = to_y;

	if (y_max < y_min)  {
		y_min = to_y;
		y_max = from_y;
	}

	// Don't do anything IF the regions are the same
	if (fast_x_min == x_min && fast_y_min == y_min &&
		fast_x_max == x_max && fast_y_max == y_max)
		return;

	// Update the saved region
	fast_x_min = x_min;
	fast_y_min = y_min;
	fast_x_max = x_max;
	fast_y_max = y_max;

	// Get Limits
	sint32 sx_limit;
	sint32 sy_limit;
	sint32 xy_limit;

	GUIApp::get_instance()->getGameMapMapGump()->
			calcFastAreaLimits(sx_limit, sy_limit, xy_limit);

	x_min -= xy_limit;
	x_max += xy_limit;
	y_min -= xy_limit;
	y_max += xy_limit;

	for (sint32 cy = 0; cy < MAP_NUM_CHUNKS; cy++) {
		for (sint32 cx = 0; cx < MAP_NUM_CHUNKS; cx++) {

			bool want_fast = cx>=x_min && cx<=x_max && cy>=y_min && cy<=y_max;
			bool currently_fast = isChunkFast(cx,cy);

			// Don't do anything, they are the same
			if (want_fast == currently_fast) continue;

			// leave fast area
			if (!want_fast) unsetChunkFast(cx,cy);
			// Enter fast area
			else setChunkFast(cx,cy);
		}
	}
}

void CurrentMap::setChunkFast(sint32 cx, sint32 cy)
{
	fast[cy][cx/32] |= 1<<(cx&31);

	item_list::iterator iter;
	for (iter = items[cx][cy].begin();
			iter != items[cx][cy].end(); ++iter) {
				(*iter)->enterFastArea();
			}
}

void CurrentMap::unsetChunkFast(sint32 cx, sint32 cy)
{
	fast[cy][cx/32] &= ~(1<<(cx&31));

	item_list::iterator iter = items[cx][cy].begin();
	while (iter != items[cx][cy].end())
	{
		Item* item = *iter;
		++iter;
		item->leaveFastArea();	// Can destroy the item
	}
}

void CurrentMap::areaSearch(UCList* itemlist, const uint8* loopscript,
							uint32 scriptsize, Item* check, uint16 range,
							bool recurse, sint32 x, sint32 y)
{
	sint32 z;
	// if item != 0, search an area around item. Otherwise, search an area
	// around (x,y)
	if (check)
		check->getLocation(x,y,z);

	//!! do the dimensions of item have to be included too?
	Rect searchrange(x-range,y-range,2*range,2*range);

	int minx, miny, maxx, maxy;

	//! constants
	minx = ((x-range)/MAP_CHUNK_SIZE) - 1;
	maxx = ((x+range)/MAP_CHUNK_SIZE) + 1;
	miny = ((y-range)/MAP_CHUNK_SIZE) - 1;
	maxy = ((y+range)/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter) {

				Item* item = *iter;

				if (item == check) continue;

				// check if item is in range?
				sint32 ix, iy, iz;
				item->getLocation(ix, iy, iz);

				ShapeInfo* info = item->getShapeInfo();
				sint32 ixd, iyd;

				//!! constants
				if (item->getFlags() & Item::FLG_FLIPPED) {
					ixd = 32 * info->y;
					iyd = 32 * info->x;
				} else {
					ixd = 32 * info->x;
					iyd = 32 * info->y;
				}

				Rect itemrect(ix - ixd, iy - iyd, ixd, iyd);

				if (!itemrect.Overlaps(searchrange)) continue;

				if (recurse) {
					// recurse into child-containers
					Container *container = p_dynamic_cast<Container*>(*iter);
					if (container)
						container->containerSearch(itemlist, loopscript,
												   scriptsize, recurse);
				}
				
				// check item against loopscript
				if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
					uint16 objid = (*iter)->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);				
				}
			}
		}
	}
}

void CurrentMap::surfaceSearch(UCList* itemlist, const uint8* loopscript,
					uint32 scriptsize, Item* check, bool above, bool below,
					bool recurse)
{
	sint32 x,y,z;
	sint32 xd,yd,zd;
	// if item != 0, search an area around item. Otherwise, search an area
	// around (x,y)
	check->getLocation(x,y,z);
	check->getFootpad(xd,yd,zd);
	xd *= 32;
	yd *= 32;
	zd *= 8;

	Rect searchrange(x-xd,y-yd,xd,yd);

	int minx, miny, maxx, maxy;

	//! constants
	minx = ((x-xd)/MAP_CHUNK_SIZE) - 1;
	maxx = ((x)/MAP_CHUNK_SIZE) + 1;
	miny = ((y-yd)/MAP_CHUNK_SIZE) - 1;
	maxy = ((y)/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter) {

				Item* item = *iter;

				if (item == check) continue;

				// check if item is in range?
				sint32 ix, iy, iz;
				item->getLocation(ix, iy, iz);
				sint32 ixd, iyd, izd;
				item->getFootpad(ixd, iyd, izd);
				//!! constants
				ixd *= 32;
				iyd *= 32;
				izd *= 8;

				Rect itemrect(ix - ixd, iy - iyd, ixd, iyd);

				if (!itemrect.Overlaps(searchrange)) continue;

				bool ok = false;
				
				if (above && iz == (z + zd)) 
				{
					ok = true;
					// Only recursive if tops aren't same (i.e. NOT flat)
					if (recurse && (izd+iz != zd+z) )
						surfaceSearch(itemlist, loopscript, scriptsize, item, true, false, true);
				}
				
				if (below && z == (iz + izd)) 
				{
					ok = true;
					// Only recursive if bottoms aren't same (i.e. NOT flat)
					if (recurse && (izd != zd) )
						surfaceSearch(itemlist, loopscript, scriptsize, item, false, true, true);
				}

				if (!ok) continue;

				// check item against loopscript
				if ((*iter)->checkLoopScript(loopscript, scriptsize)) {
					uint16 objid = (*iter)->getObjId();
					uint8 buf[2];
					buf[0] = static_cast<uint8>(objid);
					buf[1] = static_cast<uint8>(objid >> 8);
					itemlist->append(buf);				
				}
			}
		}
	}
}

TeleportEgg* CurrentMap::findDestination(uint16 id)
{
	//! constants
	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; i++) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS; j++) {
			item_list::iterator iter;
			for (iter = items[i][j].begin();
				 iter != items[i][j].end(); ++iter)
			{
				TeleportEgg* egg = p_dynamic_cast<TeleportEgg*>(*iter);
				if (egg) {
					if (!egg->isTeleporter() && egg->getTeleportId() == id)
						return egg;
				}
			}
		}
	}
	return 0;
}

bool CurrentMap::isValidPosition(sint32 x, sint32 y, sint32 z,
								 int xd, int yd, int zd,
								 uint16 item_, uint16* support_, uint16* roof_)
{
	bool valid = true;
	uint16 support = 0;
	uint16 roof = 0;
	sint32 roofz = 1 << 24; //!! semi-constant

	int minx, miny, maxx, maxy;

	//! constants
	minx = ((x-xd)/MAP_CHUNK_SIZE) - 1;
	maxx = (x/MAP_CHUNK_SIZE) + 1;
	miny = ((y-yd)/MAP_CHUNK_SIZE) - 1;
	maxy = (y/MAP_CHUNK_SIZE) + 1;
	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* item = *iter;
				if (item->getObjId() == item_) continue;

				ShapeInfo* si = item->getShapeInfo();
				//!! need to check is_sea() and is_land() maybe?
				if (!si->is_solid() && !si->is_roof())
					continue; // not an interesting item

				sint32 ix, iy, iz, ixd, iyd, izd;
				item->getLocation(ix, iy, iz);
				item->getFootpad(ixd, iyd, izd);
				ixd *= 32; iyd *= 32; izd *= 8; //!! constants

#if 0
				if (item->getShape() == 145) {
					perr << "Shape 145: (" << ix-ixd << "," << iy-iyd << ","
						 << iz << ")-(" << ix << "," << iy << "," << iz+izd
						 << ")" << std::endl;
					if (!si->is_solid()) perr << "not solid" << std::endl;
				}
#endif

				// check overlap
				if (si->is_solid() &&
					!(x <= ix - ixd || x - xd >= ix ||
					  y <= iy - iyd || y - yd >= iy ||
					  z + zd <= iz || z >= iz + izd))
				{
					// overlapping a solid item. Invalid position
#if 0
					if (item->getShape() == 145) {
						perr << "Hit!" << std::endl;
					}
#endif
					valid = false;
				}

				// check xy overlap
				if (!(x <= ix - ixd || x - xd >= ix ||
					  y <= iy - iyd || y - yd >= iy))
				{
					// check support
					if (support == 0 && si->is_solid() &&
						iz + izd == z)
					{
						support = item->getObjId();
					}

					// check roof
					if (si->is_roof() && iz < roofz && iz >= z + zd) {
						roof = item->getObjId();
						roofz = iz;
					}
				}
			}
		}
	}

	if (support_)
		*support_ = support;
	if (roof_)
		*roof_ = roof;

	return valid;
}

// Do a sweepTest of an item from start to end point.
// dims is the bounding box size.
// item is the item that we are checking to move
// solid_only forces us to check against solid items only.
// skip will skip all items until item num skip is reached
// Returns item hit or 0 if no hit.
// end is set to the colision point
bool CurrentMap::sweepTest(const sint32 start[3], const sint32 end[3],
						   const sint32 dims[3],
						   uint16 item, bool solid_only,
						   std::list<SweepItem> *hit)
{

	int i;

	//! constants
	int minx, miny, maxx, maxy;
	minx = ((start[0]-dims[0])/MAP_CHUNK_SIZE) - 1;
	maxx = (start[0]/MAP_CHUNK_SIZE) + 1;
	miny = ((start[1]-dims[1])/MAP_CHUNK_SIZE) - 1;
	maxy = (start[1]/MAP_CHUNK_SIZE) + 1;

	{
		//! constants
		int dminx, dminy, dmaxx, dmaxy;
		dminx = ((end[0]-dims[0])/MAP_CHUNK_SIZE) - 1;
		dmaxx = (end[0]/MAP_CHUNK_SIZE) + 1;
		dminy = ((end[1]-dims[1])/MAP_CHUNK_SIZE) - 1;
		dmaxy = (end[1]/MAP_CHUNK_SIZE) + 1;
		if (dminx < minx) minx = dminx;
		if (dmaxx > maxx) maxx = dmaxx;
		if (dminy < miny) miny = dminy;
		if (dmaxy > maxy) maxy = dmaxy;
	}

	if (minx < 0) minx = 0;
	if (maxx >= MAP_NUM_CHUNKS) maxx = MAP_NUM_CHUNKS-1;
	if (miny < 0) miny = 0;
	if (maxy >= MAP_NUM_CHUNKS) maxy = MAP_NUM_CHUNKS-1;

	// Get velocity of item
	sint32 vel[3];
	sint32 ext[3];
	for (i = 0; i < 3; i++) 
	{
		vel[i] = end[i] - start[i];
		ext[i] = dims[i]/2;
	}

	// Centre of object
	sint32 centre[3];
	centre[0] = start[0] - ext[0];
	centre[1] = start[1] - ext[1];
	centre[2] = start[2] + ext[2];

//	pout << "Sweeping from (" << -ext[0] << ", " << -ext[1] << ", " << -ext[2] << ")" << std::endl;
//	pout << "              (" << ext[0] << ", " << ext[1] << ", " << ext[2] << ")" << std::endl;
//	pout << "Sweeping to   (" << vel[0]-ext[0] << ", " << vel[1]-ext[1] << ", " << vel[2]-ext[2] << ")" << std::endl;
//	pout << "              (" << vel[0]+ext[0] << ", " << vel[1]+ext[1] << ", " << vel[2]+ext[2] << ")" << std::endl;

	std::list<SweepItem>::iterator sw_it;
	if (hit) sw_it = hit->end();

	for (int cx = minx; cx <= maxx; cx++) {
		for (int cy = miny; cy <= maxy; cy++) {
			item_list::iterator iter;
			for (iter = items[cx][cy].begin();
				 iter != items[cx][cy].end(); ++iter)
			{
				Item* other_item = *iter;
				if (other_item->getObjId()==item) continue;

				// This WILL hit everything and return them unless solid_only 
				// is set
				if (solid_only && !other_item->getShapeInfo()->is_solid()) 
					continue; 

				sint32 other[3], oext[3];
				other_item->getLocation(other[0], other[1], other[2]);
				other_item->getFootpad(oext[0], oext[1], oext[2]);
				oext[0] *= 16; oext[1] *= 16; oext[2] *= 4; //!! constants

				// Put other into our coord frame
				other[0] -= oext[0]+centre[0];
				other[1] -= oext[1]+centre[1];
				other[2] += oext[2]-centre[2];

				//first times of overlap along each axis
				sint32 u_1[3] = {0,0,0};

				//last times of overlap along each axis 
				sint32 u_0[3] = {0x4000,0x4000,0x4000}; // CONSTANTS

				bool touch = false;

				//find the possible first and last times
				//of overlap along each axis
				for( long i=0 ; i<3 ; i++ )
				{
					sint32 A_max = ext[i];	
					sint32 A_min = -ext[i];	
					sint32 B_max = other[i]+oext[i];	
					sint32 B_min = other[i]-oext[i];	

					if ( vel[i] < 0 && A_max>=B_min )		// A_max>=B_min not required
					{
						if (A_max==B_min) touch = true;

						// - want to know when rear of A passes front of B
						u_0[i] = ((B_max - A_min)*0x4000) / vel[i];
						// - want to know when front of A passes rear of B
						u_1[i] = ((B_min - A_max)*0x4000) / vel[i]; 
					}
					else if( vel[i] > 0 && A_min<=B_max)	// A_min<=B_max not required
					{
						if (A_min==B_max) touch = true;

						// + want to know when front of A passes rear of B
						u_0[i] = ((B_min - A_max)*0x4000) / vel[i];
						// + want to know when rear of A passes front of B
						u_1[i] = ((B_max - A_min)*0x4000) / vel[i]; 
					}
					else if( vel[i] == 0 && A_max >= B_min && A_min <= B_max)
					{
						if (A_min==B_max || A_max==B_min) touch = true;
						u_0[i] = 0;
						u_1[i] = 0x4000;
					}
					else
					{
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}

					if (u_1[i] >= u_0[i] && (u_0[i] > 0x4000 || u_1[i] < 0))
					{
						u_0[i] = 0x4001;
						u_1[i] = -1;
					}
				}

				//possible first time of overlap
				sint32 first = u_0[0];
				if (u_0[1] > first) first = u_0[1];
				if (u_0[2] > first) first = u_0[2];

				//possible last time of overlap
				sint32 last = u_1[0];
				if (u_1[1] < last) last = u_1[1];
				if (u_1[2] < last) last = u_1[2];

				//they could have only collided if
				//the first time of overlap occurred
				//before the last time of overlap
				if (first <= last)
				{
					//pout << "Hit item " << other_item->getObjId() << " at first: " << first << "  last: " << last << std::endl;

					if (!hit) return true;

					// Clamp
					if (first < 0) first = 0;
					if (last > 0x4000) last = 0x4000;

					// Ok, what we want to do here is add to the list.
					// Sorted by hit_time. 

					// Small speed up.
					if (sw_it != hit->end())
					{
						SweepItem &si = *sw_it;
						if (si.hit_time > first) sw_it = hit->begin();
					}
					else
						sw_it = hit->begin();

					for (;sw_it != hit->end(); ++sw_it) 
						if ((*sw_it).hit_time > first) break;

					// Now add it
					sw_it = hit->insert(sw_it, SweepItem(other_item->getObjId(),first,last,touch));
//					pout << "Hit item " << other_item->getObjId() << " at (" << first << "," << last << ")" << std::endl;
//					pout << "hit item      (" << other[0] << ", " << other[1] << ", " << other[2] << ")" << std::endl;
//					pout << "hit item time (" << u_0[0] << "-" << u_1[0] << ") (" << u_0[1] << "-" << u_1[1] << ") ("
//						 << u_0[2] << "-" << u_1[2] << ")" << std::endl;
				}
			}
		}
	}

	return hit && hit->size();
}

void CurrentMap::save(ODataSource* ods)
{
	ods->write2(1); // version

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS/32; ++j) {
			ods->write4(fast[i][j]);
		}
	}
}

bool CurrentMap::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	for (unsigned int i = 0; i < MAP_NUM_CHUNKS; ++i) {
		for (unsigned int j = 0; j < MAP_NUM_CHUNKS/32; ++j) {
			fast[i][j] = ids->read4();
		}
	}

	return true;
}

uint32 CurrentMap::I_canExistAt(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(shape);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	//!! TODO: figure these out
	ARG_UINT16(unk1); // is either 1 or 4
	ARG_UINT16(unk2); // looks like it could be an objid
	ARG_UINT16(unk3); // always zero

	int xd, yd, zd;
	ShapeInfo* si = GameData::get_instance()->
		getMainShapes()->getShapeInfo(shape);
	//!! constants
	xd = si->x * 32;
	yd = si->y * 32;
	zd = si->z * 32;

	CurrentMap* cm = World::get_instance()->getCurrentMap();
	bool valid = cm->isValidPosition(x, y, z, xd, yd, zd, 0, 0, 0);
	if (valid)
		return 1;
	else
		return 0;
}
