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

#ifndef ITEM_H
#define ITEM_H

#include "Object.h"

#include "intrinsics.h"
class Container;
class ShapeInfo;
class Shape;

class Item : public Object
{
	friend class ItemFactory;

public:
	Item();
	virtual ~Item();

	// p_dynamic_cast stuff
	ENABLE_DYNAMIC_CAST(Item);

	Container* getParent() const { return parent; }
	void setLocation(sint32 x, sint32 y, sint32 z); // this only sets the loc.
	void move(sint32 x, sint32 y, sint32 z); // move also handles item lists
	void getLocation(sint32& x, sint32& y, sint32& z) const;
	void getCentre(sint32& x, sint32& y, sint32& z) const;
	void getFootpad(sint32& x, sint32& y, sint32& z) const;
	uint16 getFlags() const { return flags; }
	void setFlag(uint32 mask) { flags |= mask; }
	void clearFlag(uint32 mask) { flags &= ~mask; }
	void setExtFlags(uint32 f) { extendedflags = f; }
	uint32 getExtFlags() const { return extendedflags; }
	uint32 getShape() const { return shape; }
	// Hack Alert!
	void setShape(uint32 shape_) { *const_cast<uint32*>(&shape) = shape_; cachedShapeInfo = 0; cachedShape = 0; }
	uint32 getFrame() const { return frame; }
	void setFrame(uint32 frame_) { frame = frame_; }
	uint16 getQuality() const { return quality; }
	void setQuality(uint16 quality_) { quality = quality_; }
	uint16 getNpcNum() const { return npcnum; }
	uint16 getMapNum() const { return mapnum; }
	void setMapNum(uint16 mapnum_) { mapnum = mapnum_; }
	ShapeInfo* getShapeInfo();
	ShapeInfo* getShapeInfoNoCache() const;
	Shape* getShapeObject();
	Shape* getShapeObjectNoCache() const;
	uint16 getFamily();

	Item* getGlobNext() const { return glob_next; }
	void setGlobNext(Item* i) { glob_next = i; }

	virtual void destroy();

	bool overlaps(Item& item2) const;
	bool overlapsxy(Item& item2) const;
	bool isOn(Item& item2) const;

	virtual uint32 getTotalWeight(); // weight including contents (if any)
	bool checkLoopScript(const uint8* script, uint32 scriptsize);
	uint32 callUsecodeEvent(uint32 event);

	inline void getLerped(sint32& x, sint32& y, sint32& z) const // Get lerped location
		{ x = ix; y = iy; z = iz; }

	inline void doLerp(sint32 camx, sint32 camy, sint32 camz, sint32 factor) 		// Does lerping for an in between frame (0-256)
	{
		// Should be noted that this does indeed limit us to 'only' 24bit coords
		// not that it matters because on disk they are unsigned 16 bit
		if (factor == 256)
		{
			ix = l_next.x - camx;
			iy = l_next.y - camy;
			iz = l_next.z - camz;
		}
		else if (factor == 0)
		{
			ix = l_prev.x - camx;
			iy = l_prev.y - camy;
			iz = l_prev.z - camz;
		}
		else
		{
#if 1
			// This way while possibly slower is more accurate
			ix = ((l_prev.x*(256-factor) + l_next.x*factor)>>8) - camx;
			iy = ((l_prev.y*(256-factor) + l_next.y*factor)>>8) - camy;
			iz = ((l_prev.z*(256-factor) + l_next.z*factor)>>8) - camz;
#else
			ix = l_prev.x + (((l_next.x-l_prev.x)*factor)>>8) - cx;
			iy = l_prev.y + (((l_next.y-l_prev.y)*factor)>>8) - cy;
			iz = l_prev.z + (((l_next.z-l_prev.z)*factor)>>8) - cz;
#endif
		}
	}

	// fastArea Handling. Note that these are intended to call the usecode funcs,
	// and do other stuff
	void inFastArea(int even_odd);		// Called when an item has entered or is in the fast area
	void leavingFastArea();				// Called when an item is leaving the fast area


	// Intrinsics
	INTRINSIC(I_getX);
	INTRINSIC(I_getY);
	INTRINSIC(I_getZ);
	INTRINSIC(I_getCX);
	INTRINSIC(I_getCY);
	INTRINSIC(I_getCZ);
	INTRINSIC(I_getPoint);
	INTRINSIC(I_getShape);
	INTRINSIC(I_setShape);
	INTRINSIC(I_getFrame);
	INTRINSIC(I_setFrame);
	INTRINSIC(I_getQuality);
	INTRINSIC(I_getUnkEggType);
	INTRINSIC(I_getQuantity);
	INTRINSIC(I_getContainer);
	INTRINSIC(I_getRootContainer);
	INTRINSIC(I_getQ);
	INTRINSIC(I_setQ);
	INTRINSIC(I_setQuality);
	INTRINSIC(I_setQuantity);
	INTRINSIC(I_getFamily);
	INTRINSIC(I_getTypeFlag);
	INTRINSIC(I_getStatus);
	INTRINSIC(I_orStatus);
	INTRINSIC(I_andStatus);
	INTRINSIC(I_getFootpad);
	INTRINSIC(I_overlaps);
	INTRINSIC(I_overlapsXY);
	INTRINSIC(I_isOn);
	INTRINSIC(I_getWeight);
	INTRINSIC(I_getWeightIncludingContents);
	INTRINSIC(I_getVolume);
	INTRINSIC(I_bark);
	INTRINSIC(I_getDirToCoords);
	INTRINSIC(I_getDirFromCoords);
	INTRINSIC(I_getDirToItem);
	INTRINSIC(I_getDirFromItem);
	INTRINSIC(I_look);
	INTRINSIC(I_use);
	INTRINSIC(I_enterFastArea);
	INTRINSIC(I_ask);
	INTRINSIC(I_create);
	INTRINSIC(I_legalCreateAtPoint);
	INTRINSIC(I_legalCreateAtCoords);
	INTRINSIC(I_legalCreateInCont);
	INTRINSIC(I_push);
	INTRINSIC(I_pop);
	INTRINSIC(I_popToCoords);
	INTRINSIC(I_popToContainer);
	INTRINSIC(I_popToEnd);
	INTRINSIC(I_destroy);
	INTRINSIC(I_move);
	INTRINSIC(I_shoot);
	INTRINSIC(I_getFamilyOfType);
	INTRINSIC(I_getEtherealTop);

protected:
	const uint32 shape;	// DO NOT modify this directly! Always use setShape()!
	uint32 frame;

	sint32 x,y,z; // world coordinates
	uint16 flags;
	uint16 quality;
	uint16 npcnum;
	uint16 mapnum;

	uint32 extendedflags; // pentagram's own flags

	Container* parent; // container this item is in (or 0 for top-level items)

	Shape *cachedShape;
	ShapeInfo *cachedShapeInfo;

	// This is stuff that is used for displaying and interpolation
	struct Lerped
	{
		sint32 x,y,z;
		sint32 shape,frame;
	};
	
	Lerped	l_prev;			// Previous state (relative to camera)
	Lerped	l_next;			// Next (current) state (relative to camera)
	sint32	ix,iy,iz;		// Interpolated position in camera space

private:
	Item* glob_next; // next item in glob

	// Animate the item (called by inFastArea)
	void animateItem(int even_odd);

	// Setup the lerped info for this frame (called by inFastArea)
	void setupLerp();		

public:
	enum {
		FLG_DISPOSABLE	 = 0x0002,
		FLG_OWNED		 = 0x0004,
		FLG_CONTAINED	 = 0x0008,
		FLG_INVISIBLE	 = 0x0010,
		FLG_FLIPPED		 = 0x0020,
		FLG_IN_NPC_LIST	 = 0x0040,
		FLG_MONSTER_NPC	 = 0x0080,	// Maybe... i'm not entirely sure that this is what this is
		FLG_GUMP_OPEN	 = 0x0100,
		FLG_EQUIPPED	 = 0x0200,
		FLG_BOUNCING	 = 0x0400,
		FLG_ETHEREAL	 = 0x0800,
		FLG_HANGING		 = 0x1000,
		FLG_FASTAREA     = 0x2000,
		FLG_LOW_FRICTION = 0x4000
	} statusflags;

	enum {
		EXT_FIXED     = 0x0001, // item came from FIXED
		EXT_INGLOB    = 0x0002, // item is part of a glob
		EXT_NOTINMAP  = 0x0004, // item isn't part of the map itself (e.g. NPCs)

		EXT_FAST0	  = 0x0008,	// Special stuff for detecting an item leaving the fast area
		EXT_FAST1	  = 0x0010	// They are mutually exclusive
	} extflags;
};

#endif
