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

class RenderSurface;


//class GumpList;

//
// Gump Flags
//
#define GUMP_FLAG_UNMOVABLE		1

//
// Gump Layers
// Higher numbered layers are always on top of lower numbered layers
//
#define GUMP_LAYER_DESKTOP		-16		// Layer for Desktop 'bottom most'
#define GUMP_LAYER_WORLD		-8		// Layer for the World Gump
#define GUMP_LAYER_NORMAL		0		// Layer for Normal gumps
#define GUMP_LAYER_ON_TOP		8		// Layer for Always on top Gumps

//
// Gump::UpdateRenderSurface reasons
//
enum URS_Reason
{
	URS_DeviceChanged			= 0		// Device settings have changed
};

//
// Class Gump
//
// Desc: Base Gump Class that all other Gumps inherit from
//

class Gump : public Object
{
protected:

	Gump *			parent;					// Parent gump
	uint32			gump_flags;				// Gump flags

	sint32			x, y;					// x, y position of gump reletive to parent
	sint32			layer;					// gump ordering layer

	Rect			dims;					// The dimentions of the gumps. Should be noted
											// that gumps don't 'need' to start at 0,0

	RenderSurface	*surface;				// Gump's RenderSurface, if it has one

	// The Gump list for this gump. This will contain all child gumps, as well as
	// all gump widgets. It has it's own 'Active' gump selection, handles layering,
	// painting, and can also issues input commands
	//GumpList		*children;

public:

	Gump(int x, int y, int width, int height);
	virtual ~Gump();

	// Get the mouse cursor for position mx, my relative to parents position
	// Returns true if this gump wants to set the cursor. If false, thet gump list
	// will attempt to get the cursor shape from the next lower shape.
	//virtual bool		GetMouseCursor(int mx, int my, Shape &shape, sint32 &frame);

	// Update the RenderSurface of this gump and all children.
	//virtual bool		UpdateRenderSurface(uint32 reason);

	// TODO: Change this
	// RenderSurface is the parent RenderSurface. This is required for gumps
	// that don't have their own RenderSurfaces
	virtual void		Paint(RenderSurface*);

	// Move this gump
	// virtual void		Move(int x, int y);


	//
	// Some event handlers. In theory they 'should' be able to be mapped to
	// Usecode classes.
	//
	// mx and my are relative to parents position
	//
	// return true if handled, false if not handled
	//
	// Unhandle input will be passed down to the next lower gump
	//
	
	virtual bool		OnMouseDown(int button, int mx, int my) { return false; }
	virtual bool		OnMouseUp(int button, int mx, int my) { return false; }
	virtual bool		OnMouseDouble(int button, int mx, int my) { return false; }
	virtual bool		OnMouseDrag(int button, int mx, int my) { return false; }
	virtual bool		OnKeyDown(int key) { return false; }
	virtual bool		OnKeyUp(int key) { return false; }

	virtual bool		OnTextInput(int unicode) { return false; }

	// This is for detecting focus changes for keyboard input. Gets called true
	// when the GumpList wants to make this the focus gump. It is called false 
	// when focus is being taken away. If returning false on focus gain, the 
	// gump states it doesn't want to gain input focus. For OnFocus(false) 
	// calls return value is ignored
	virtual bool		OnFocus(bool gain) { return false; }
};

#endif //GUMP_H_INCLUDED
