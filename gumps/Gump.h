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

#ifndef GUMP_H_INCLUDED
#define GUMP_H_INCLUDED

#include "Object.h"
#include "Rect.h"
#include <list>

class RenderSurface;
class Shape;
class GumpNotifyProcess;

//
// Class Gump
//
// Desc: Base Gump Class that all other Gumps inherit from
//

class Gump : public Object
{
protected:

	friend class GumpList;

	uint16				owner;			// Owner item
	Gump *				parent;			// Parent gump
	sint32				x, y;			// Gumps position. This is always the upper left corner!

	Rect				dims;			// The dimentions/coord space of the gump
	uint32				flags;			// Gump flags
	sint32				layer;			// gump ordering layer

	sint32				index;			// 'Index'

	Shape				*shape;			// The gumps shape (always painted at 0,0)
	uint32				framenum;

	// The Gump list for this gump. This will contain all child gumps,
	// as well as all gump widgets. 
	std::list<Gump*>	children;		// List of all gumps
	Gump *				focus_child;	// The child that has focus

	GumpNotifyProcess	*notifier;		// Process to notify when we're closing
	uint32				process_result;	// Result for the notifier process

public:

	ENABLE_RUNTIME_CLASSTYPE();

	Gump(int x, int y, int width, int height, uint16 owner = 0,
		 uint32 _Flags = 0, sint32 layer = LAYER_NORMAL);
	virtual ~Gump();

	virtual void				CreateNotifier();
	inline GumpNotifyProcess*	GetNotifyProcess() { return notifier; }

	inline void					SetShape(Shape *_shape, uint32 _framenum)
		{ shape = _shape; framenum = _framenum; }
	inline void					SetFramenum(uint32 _framenum)
		{ framenum = _framenum; }

	// Init the gump, call after construction
	virtual void				InitGump();

	// Find a gump of the specified type (this or child). 2 ways of doing it:
	// gump->FindChild(ChildGump::ClassType, recursive, no_inheritance);
	// gump->FindChild<ChildGump>(recursive, no_inheritance);
	virtual Gump *				FindGump(const RunTimeClassType& t,
										 bool recursive=true,
										 bool no_inheritance=false);
	template<class T> Gump *	FindGump(bool recursive=true,
										 bool no_inheritance=false)
		{ return FindGump(T::ClassType, recursive, no_inheritance); }

	// Get the mouse cursor for position mx, my relative to parents
	// position. Returns true if this gump wants to set the cursor.
	// If false, the gump list will attempt to get the cursor shape from
	// the next lower gump.
	virtual bool		GetMouseCursor(int mx, int my, Shape &shape,
									   sint32 &frame);

	// Update the RenderSurface of this gump and all children (probably
	//  only needed for scaled gumps).
	//virtual bool		DeviceChanged();


	// Run the gump (returns true if repaint required)
	virtual bool		Run(const uint32 framenum);

	// Called when there is a map change (so the gumps can self terminate
	// among other things)
	virtual void		MapChanged(void);

	// Paint the Gump (RenderSurface is relative to parent).
	// Calls PaintThis and PaintChildren
	virtual void		Paint(RenderSurface*, sint32 lerp_factor);

protected:

	// Overloadable method to Paint just this Gumps
	// (RenderSurface is relative to this)
	virtual void		PaintThis(RenderSurface*, sint32 lerp_factor);

	// Paint the Gumps Children (RenderSurface is relative to this)
	virtual void		PaintChildren(RenderSurface*, sint32 lerp_factor);

public:

	// Close the gump
	virtual void		Close(bool no_del = false);

	// Move this gump
	virtual void		Move(int x_, int y_) { x = x_; y = y_; }

	//
	// Points and Coords
	//

	// Get the dims
	virtual void		GetDims(Rect &d) { d = dims; }

	// Used to detect if a point is on the gump
	virtual bool		PointOnGump(int mx, int my);

	// Convert a screen space point to a gump point
	virtual void		ScreenSpaceToGump(int &sx, int &sy);

	// Convert a gump point to a screen space point
	virtual void		GumpToScreenSpace(int &gx, int &gy);

	// Convert a parent relative point to a gump point
	virtual void		ParentToGump(int &px, int &py);

	// Convert a gump point to parent relative point
	virtual void		GumpToParent(int &gx, int &gy);


	// Trace a click, and return ObjID
	virtual uint16		TraceObjID(int mx, int my);

	// Get the location of an item in the gump (coords relative to this).
	// Returns false on failure
	virtual bool		GetLocationOfItem(uint16 itemid, int &gx, int &gy,
										  sint32 lerp_factor = 256);


	//
	// Some event handlers. In theory they 'should' be able to be mapped to
	// Usecode classes.
	//
	// mx and my are relative to parents position
	//
	// OnMouseDown returns the Gump that handled the Input, if it was handled.
	// The MouseUp,MouseDouble events will be sent to the same gump.
	//
	// Unhandled input will be passed down to the next lower gump.
	//
	// A mouse click on a gump will make it focus, IF it wants it.
	//
	
	// Return Gump that handled event
	virtual Gump *		OnMouseDown(int button, int mx, int my);
	virtual void		OnMouseUp(int  button, int mx, int my) { }
	virtual void		OnMouseClick(int button, int mx, int my) { }
	virtual void		OnMouseDouble(int button, int mx, int my) { }

	// Keyboard input gets sent to the FocusGump. Or if there isn't one, it
	// will instead get sent to the default key handler. TextInput requires
	// that text mode be enabled. Return true if handled, false if not.
	// Default, returns false, unless handled by focus child
	virtual bool		OnKeyDown(int key);
	virtual bool		OnKeyUp(int key);
	virtual bool		OnTextInput(int unicode);

	// This is for detecting focus changes for keyboard input. Gets called true
	// when the this gump is being set as the focus focus gump. It is called
	// false when focus is being taken away.
	virtual void		OnFocus(bool /*gain*/) { }
	
	// Makes this gump the focus
	virtual void		MakeFocus();

	// Is this gump the focus?
	inline bool			IsFocus()
		{ return parent?parent->focus_child==this:false; }

	// Get the child in focus
	inline Gump *		GetFocusChild() { return focus_child; }

	// Find a new Child to be the focus
	void				FindNewFocusChild();	


	//
	// Child gump related
	//

	// Add a gump to the list. 
	virtual void		AddChild(Gump *, bool take_focus = true);

	// Remove a gump from the list
	virtual void		RemoveChild(Gump *);	

	// Get the parent
	inline Gump *		GetParent() { return parent; }

	// Get the root gump (or self)
	Gump *				GetRootGump();

	// This function is used by our children to notifty us of 'something'
	// Think of it as a generic call back function
	virtual void		ChildNotify(Gump *child, uint32 message) { }
	void				SetIndex(sint32 i) { index = i; }
	sint32				GetIndex() { return index; }

	// Dragging
	virtual void		StartDraggingChild(Gump* gump, int mx, int my);
	virtual void		DraggingChild(Gump* gump, int mx, int my);
	virtual void		StopDraggingChild(Gump* gump);


	// the MoveOffset is the point relative to which Move() will move the gump
	void				SetMoveOffset(int mx, int my)
		{ moveOffsetX = mx; moveOffsetY = my; }
protected:
	int moveOffsetX, moveOffsetY;

public:

	//
	// Gump Flags
	//
	enum GumpFlags {
		FLAG_UNMOVABLE		= 0x01,		// When set, the gump can not be dragged
		FLAG_HIDDEN			= 0x02,		// When set, the gump will not be drawn
		FLAG_CLOSING		= 0x04,		// When set, the gump is closing
		FLAG_CLOSE_AND_DEL	= 0x08,		// When set, the gump is closing and will be deleted
		FLAG_ITEM_DEPENDANT	= 0x10		// When set, the gump will be deleted on MapChange
	};

	inline bool			IsHidden() { return (flags&FLAG_HIDDEN) != 0; }
	virtual void		HideGump() { flags |= FLAG_HIDDEN; }
	virtual void		UnhideGump() { flags &= ~FLAG_HIDDEN; }

	//
	// Gump Layers
	//
	enum GumpLayers {
		LAYER_DESKTOP		= -16,		// Layer for Desktop 'bottom most'
		LAYER_GAMEMAP		= -8,		// Layer for the World Gump
		LAYER_NORMAL		= 0,		// Layer for Normal gumps
		LAYER_ABOVE_NORMAL	= 8,		// Layer for Always on top Gumps
		LAYER_CONSOLE		= 16		// Layer for the console
	};
};

#endif //GUMP_H_INCLUDED
