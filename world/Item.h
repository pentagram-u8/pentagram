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
class Gump;

class Item : public Object
{
	friend class ItemFactory;

public:
	Item();
	virtual ~Item();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	//! Get the Container this Item is in, if any. (NULL if not in a Container)
	Container* getParent() const { return parent; }

	//! Set the parent container of this item.
	void setParent(Container* p) { parent = p; }

	//! Set item location. This strictly sets the location, and does not
	//! even update CurrentMap
	void setLocation(sint32 x, sint32 y, sint32 z); // this only sets the loc.

	//! Move an item. This moves an item to the new location, and updates
	//! CurrentMap if necessary.
	//! \param forcemapupdate if set, will force updating the item lists in
	//!                       CurrentMap
	void move(sint32 x, sint32 y, sint32 z, bool forcemapupdate=false);

	//! Get the location of the top-most container this Item is in, or
	//! this Item's location if not in a container.
	void getLocationAbsolute(sint32& x, sint32& y, sint32& z) const;

	//! Get this Item's location. Note that this does not return
	//! 'usable' coordinates if the Item is contained or equipped.
	void getLocation(sint32& x, sint32& y, sint32& z) const;

	//! Get this Item's Z coordinate.
	sint32 getZ() const;

	//! Get this Item's location in a ContainerGump. Undefined if the Item 
	//! is not in a Container.
	void getGumpLocation(sint32& x, sint32& y) const;

	//! Set the Item's location in a ContainerGump. NOP if the Item
	//! is not in a Container.
	void setGumpLocation(sint32 x, sint32 y);

	//! Get the world coordinates of the Item's centre. Undefined if the Item
	//! is contained or equipped.
	void getCentre(sint32& x, sint32& y, sint32& z) const;

	//! Get the size of this item's 3D bounding box. (Note that the coordinates
	//! are not in the same unit as world coordinates)
	void getFootpad(sint32& x, sint32& y, sint32& z) const;

	//! Get flags
	uint16 getFlags() const { return flags; }

	//! Set the flags set in the given mask.
	void setFlag(uint32 mask) { flags |= mask; }

	//! Clear the flags set in the given mask.
	void clearFlag(uint32 mask) { flags &= ~mask; }

	//! Set extendedflags
	void setExtFlags(uint32 f) { extendedflags = f; }

	//! Get extendedflags
	uint32 getExtFlags() const { return extendedflags; }

	//! Set the extendedflags set in the given mask.
	void setExtFlag(uint32 mask) { extendedflags |= mask; }

	//! Clear the extendedflags set in the given mask.
	void clearExtFlag(uint32 mask) { extendedflags &= ~mask; }

	//! Get this Item's shape number
	uint32 getShape() const { return shape; }

	//! Set this Item's shape number
	// Hack Alert!
	void setShape(uint32 shape_) { *const_cast<uint32*>(&shape) = shape_; cachedShapeInfo = 0; cachedShape = 0; }

	//! Get this Item's frame number
	uint32 getFrame() const { return frame; }

	//! Set this Item's frame number
	void setFrame(uint32 frame_) { frame = frame_; }

	//! Get this Item's quality (a.k.a. 'Q')
	uint16 getQuality() const { return quality; }

	//! Set this Item's quality (a.k.a 'Q');
	void setQuality(uint16 quality_) { quality = quality_; }

	//! Get the 'NpcNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	uint16 getNpcNum() const { return npcnum; }

	//! Set the 'NpcNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	void setNpcNum(uint16 npcnum_) { npcnum = npcnum_; }

	//! Get the 'MapNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	uint16 getMapNum() const { return mapnum; }

	//! Set the 'NpcNum' of this Item. Note that this can represent various
	//! things depending on the family of this Item.
	void setMapNum(uint16 mapnum_) { mapnum = mapnum_; }

	//! Get the ShapeInfo object for this Item. (The pointer will be cached.)
	ShapeInfo* getShapeInfo();

	//! Get the ShapeInfo object for this Item, bypassing the cached pointer.
	ShapeInfo* getShapeInfoNoCache() const;

	//! Get the Shape object for this Item. (The pointer will be cached.)
	Shape* getShapeObject();

	//! Get the Shape object for this Item, bypassing the cached pointer.
	Shape* getShapeObjectNoCache() const;

	//! Get the family of the shape number of this Item. (This is a
	//! member of the ShapeInfo object.)
	uint16 getFamily();

	//! Get the open ContainerGump for this Item, if any. (NULL if not open.)
	uint16 getGump() { return gump; }
	//! Call this to notify the Item's open Gump has closed.
	void clearGump(); // set gump to 0 and clear the GUMP_OPEN flag

	//! Get the next Item in the Glob this Item belongs to
	uint16 getGlobNext() const { return glob_next; }
	//! Set the next Item in the Glob this Item belongs to.
	void setGlobNext(Item* i) { glob_next = i->getObjId(); }

	//! Destroy self.
	virtual void destroy();

	//! Check if this item overlaps another item in 3D world-space
	bool overlaps(Item& item2) const;

	//! Check if this item overlaps another item in the xy dims in 3D space
	bool overlapsxy(Item& item2) const;

	//! Check if this item is on top another item
	bool isOn(Item& item2) const;

	//! Check if this item can exist at the given coordinates
	bool canExistAt(sint32 x, sint32 y, sint32 z) const;

	//! Move the object to (x,y,z) colliding with objects in the way.
	//! \param teleport move without colliding with objects between source and
	//!        destination
	//! \param force force the object to get to the destination without being
	//!        blocked by solid objects
	//! \returns 0-0x4000 representing how far it got.
	//!          0 = didn't move
	//!          0x4000 = reached destination
	sint32 collideMove(sint32 x,sint32 y,sint32 z, bool teleport, bool force);

	//! Make the item fall down.
	//! This creates a GravityProcess to do the actual work if the Item
	//! doesn't already have one.
	void fall();

	//! Make any items on top of this Item fall down.
	//! Note that this Item has to be moved away right after calling grab(),
	//! since otherwise the items will immediately hit this Item again.
	void grab();

	//! Assign a GravityProcess to this Item
	void setGravityProcess(uint16 pid) { gravitypid = pid; }

	//! Get the weight of this Item and its contents, if any
	virtual uint32 getTotalWeight();

	//! Check this Item against the given loopscript
	//! \param script The loopscript to run
	//! \param scriptsize The size (in bytes) of the loopscript
	//! \return true if the item matches, false otherwise
	bool checkLoopScript(const uint8* script, uint32 scriptsize);
	
	uint32 callUsecodeEvent_look();								// event 0
	uint32 callUsecodeEvent_use();								// event 1
	uint32 callUsecodeEvent_anim();								// event 2
	uint32 callUsecodeEvent_cachein();							// event 4
	uint32 callUsecodeEvent_hit(uint16 hitted, sint16 unk);		// event 5
	uint32 callUsecodeEvent_gotHit(uint16 hitter, sint16 unk);	// event 6
	uint32 callUsecodeEvent_hatch();							// event 7
	uint32 callUsecodeEvent_schedule();							// event 8
	uint32 callUsecodeEvent_release();							// event 9
	uint32 callUsecodeEvent_combine();							// event C
	uint32 callUsecodeEvent_enterFastArea();					// event F
	uint32 callUsecodeEvent_leaveFastArea();					// event 10
	uint32 callUsecodeEvent_cast(uint16 unk);					// event 11
	uint32 callUsecodeEvent_justMoved();						// event 12
	uint32 callUsecodeEvent_AvatarStoleSomething(uint16 unk);	// event 14
	uint32 callUsecodeEvent_guardianBark(sint16 unk);			// event 15

	//! Get lerped location.
	inline void getLerped(sint32& x, sint32& y, sint32& z) const
		{ x = ix; y = iy; z = iz; }

	//! Do lerping for an in between frame (0-256)
	//! The result can be retrieved with getLerped(x,y,z)
	//! \param factor The lerp factor: 0 is start of move, 256 is end of move
	inline void doLerp(sint32 factor)
	{
	// Should be noted that this does indeed limit us to 'only' 24bit coords
	// not that it matters because on disk they are unsigned 16 bit

		if (factor == 256)
		{
			ix = l_next.x;
			iy = l_next.y;
			iz = l_next.z;
		}
		else if (factor == 0)
		{
			ix = l_prev.x;
			iy = l_prev.y;
			iz = l_prev.z;
		}
		else
		{
#if 1
			// This way while possibly slower is more accurate
			ix = ((l_prev.x*(256-factor) + l_next.x*factor)>>8);
			iy = ((l_prev.y*(256-factor) + l_next.y*factor)>>8);
			iz = ((l_prev.z*(256-factor) + l_next.z*factor)>>8);
#else
			ix = l_prev.x + (((l_next.x-l_prev.x)*factor)>>8);
			iy = l_prev.y + (((l_next.y-l_prev.y)*factor)>>8);
			iz = l_prev.z + (((l_next.z-l_prev.z)*factor)>>8);
#endif
		}
	}


	// fastArea Handling. Note that these are intended to call the usecode
	// funcs, and do other stuff

	//! This is called when an item has entered or is in the fast area
	//! \param even_odd Used internally for tracking when items enter/leave
	//!        the fast area.
	//! \param framenum The current framenum.
	void inFastArea(int even_odd, int framenum); 

	//! This is called when an item is leaving the fast area
	void leavingFastArea();

	// Intrinsics
	INTRINSIC(I_touch);
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
	INTRINSIC(I_getMapArray);
	INTRINSIC(I_setMapArray);
	INTRINSIC(I_getDirToCoords);
	INTRINSIC(I_getDirFromCoords);
	INTRINSIC(I_getDirToItem);
	INTRINSIC(I_getDirFromItem);
	INTRINSIC(I_look);
	INTRINSIC(I_use);
	INTRINSIC(I_enterFastArea);
	INTRINSIC(I_ask);
	INTRINSIC(I_openGump);
	INTRINSIC(I_closeGump);
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
	INTRINSIC(I_legalMoveToPoint);
	INTRINSIC(I_hurl);
	INTRINSIC(I_shoot);
	INTRINSIC(I_fall);
	INTRINSIC(I_grab);
	INTRINSIC(I_getFamilyOfType);
	INTRINSIC(I_getEtherealTop);
	INTRINSIC(I_guardianBark);
	INTRINSIC(I_getSurfaceWeight);
	INTRINSIC(I_isExplosive);

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

	uint16 gump;			// Item's gump
	uint16 gravitypid;		// Item's GravityTracker (or 0)

private:

	//! Call a Usecode Event. Use the separate functions instead!
	uint32 callUsecodeEvent(uint32 event, const uint8* args=0, int argsize=0);

	uint16 glob_next; // next item in glob

	// The frame setupLerp was last called on
	uint32	last_setup;	

	//! Animate the item (called by inFastArea)
	void animateItem();

	//! Setup the lerped info for this frame (called by inFastArea)
	void setupLerp(bool post);		

public:
	enum statusflags {
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
	};

	enum extflags {
		EXT_FIXED     = 0x0001, // item came from FIXED
		EXT_INGLOB    = 0x0002, // item is part of a glob
		EXT_NOTINMAP  = 0x0004, // item isn't part of the map itself (e.g. NPCs)

		EXT_FAST0	  = 0x0008,	// Special stuff for detecting an item leaving the fast area
		EXT_FAST1	  = 0x0010,	// They are mutually exclusive

		EXT_HIGHLIGHT = 0x0020	// Paint the item highlighted

	};
};

#endif
