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
#include "Gump.h"
#include "RenderSurface.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GumpNotifyProcess.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(Gump,Object);

Gump::Gump(int _X, int _Y, int Width, int Height, uint32 _Flags, sint32 _Layer) : 
	Object(), parent(0), x(_X), y(_Y), dims(0,0,Width,Height), layer(_Layer), 
	flags(_Flags), shape(0), framenum(0), children(), focus_child(0),
	notifier(0), process_result(0)
{
	
}

Gump::~Gump()
{
	// Get rid of focus
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;

	// Delete all children
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	while (it != end)
	{
		Gump *g = *it;
		it = children.erase(it);
		delete g;
	}

}

void Gump::InitGump()
{
	// Not yet...
	// assignObjId();
}

void Gump::Close(bool no_del)
{
	if (notifier) notifier->notifyClosing(process_result);
	notifier = 0;

	if (!parent) 
	{
		if (!no_del) delete this;
	}
	else
	{
		flags |= FLAG_CLOSING;
		if (!no_del) flags |= FLAG_CLOSE_AND_DEL;
	}
}

void Gump::SetupLerp()
{
	// Iterate all children
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	while (it != end)
	{
		Gump *g = *it;

		// Setup the lerp of the child
		g->SetupLerp();

		if (g->flags & FLAG_CLOSING)
		{
			it = children.erase(it);
			FindNewFocusChild();
			// Uh, why does this cause crashes...
			 if (g->flags & FLAG_CLOSE_AND_DEL) delete g;
		}
		else
		{
			++it;
		}
	}	
}

void Gump::Paint(RenderSurface *surf, sint32 lerp_factor)
{
	// Don't paint if hidder
	if (IsHidden()) return;

	// Get old Origin
	int ox=0, oy=0;
	surf->GetOrigin(ox, oy);

	// Set the new Origin
	int nx=0, ny=0;
	GumpToParent(nx,ny);
	surf->SetOrigin(ox+nx, oy+ny);

	// Get Old Clipping Rect
	Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	Rect new_rect = old_rect;
	new_rect.Intersect(dims);
	surf->SetClippingRect(new_rect);

	// Paint This
	PaintThis(surf, lerp_factor);

	// Paint children
	PaintChildren(surf, lerp_factor);

	// Reset The Clipping Rect
	surf->SetClippingRect(old_rect);

	// Reset The Origin
	surf->SetOrigin(ox, oy);
}

void Gump::PaintThis(RenderSurface* surf, sint32 /*lerp_factor*/)
{
	if (shape)
	{
		Rect sr;
		ShapeFrame *f = shape->getFrame(framenum);
		sr.h = f->height;
		sr.w = f->height;
		sr.x = -f->xoff;
		sr.y = -f->yoff;

		if (surf->CheckClipped(sr))
			surf->Paint(shape, framenum, 0, 0);
		else
			surf->PaintNoClip(shape, framenum, 0, 0);
	}
}

void Gump::PaintChildren(RenderSurface *surf, sint32 lerp_factor)
{
	// Iterate all children
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	for (;it != end; ++it)
	{
		// Paint if not hidden
		(*it)->Paint(surf, lerp_factor);
	}	
}

bool Gump::PointOnGump(int mx, int my)
{
	ParentToGump(mx,my);

	// First check again rectangle
	if (!dims.InRect(mx,my)) 
		return false;

	// TODO - Check again shape/texture if this gump has one
	// Then again, we might want to have stuff like that somewhere
	// else like in a TexturedGump or ShapeGump or something
	//
	// Also might want to check children, cause they may not be over
	// our shape

	return true;
}

// Convert a screen space point to a gump point
void Gump::ScreenSpaceToGump(int &sx, int &sy)
{
	// This is a recursive operation. We get each
	// parent to convert the point into their local
	// coords. 
	if (parent) parent->ScreenSpaceToGump(sx,sy);

	ParentToGump(sx,sy);
}

// Convert a gump point to a screen space point
void Gump::GumpToScreenSpace(int &gx, int &gy)
{
	// This is a recursive operation. We get each
	// gump to convert the point to their parent

	GumpToParent(gx,gy);

	if (parent) parent->GumpToScreenSpace(gx,gy);
}

// Convert a parent relative point to a gump point
void Gump::ParentToGump(int &px, int &py)
{
	px -= x;
	px += dims.x;
	py -= y;
	py += dims.y;
}

// Convert a gump point to parent relative point
void Gump::GumpToParent(int &gx, int &gy)
{
	gx -= dims.x;
	gx += x;
	gy -= dims.y;
	gy += y;
}

uint16 Gump::TraceObjID(int mx, int my)
{
	// Convert to local coords
	ParentToGump(mx,my);

	std::list<Gump*>::iterator	it = children.end();
	std::list<Gump*>::iterator	begin = children.begin();

	uint16 objid = 0;

	// Iterate children
	while (it != begin)
	{
		Gump *g = *--it;

		// It's got the point
		if (g->PointOnGump(mx,my)) objid = g->TraceObjID(mx, my);

		if (objid && objid != 65535) break;
	}

	if (!objid || objid == 65535)
		objid = getObjId();	// Ha, yeah right, we don't assign objId's yet :-)

	return objid;
}

bool Gump::GetLocationOfItem(uint16 itemid, int &gx, int &gy, sint32 lerp_factor)
{
	gx = 0;
	gy = 0;
	return false;
}

// Find a child gump of the specified type
Gump* Gump::FindGump(const RunTimeClassType &t, bool recursive, bool no_inhertance)
{
	// If that is our type, then return us!
	if (GetClassType() == t) return this;
	else if (!no_inhertance && IsOfType(t)) return this;

	// Iterate all children
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	for (;it != end; ++it)
	{
		Gump *g = *it;

		if (g->GetClassType() == t) return g;
		else if (!no_inhertance && g->IsOfType(t)) return g;
	}

	if (!recursive) return 0;

	// Recursive Iterate all children
	it = children.begin();
	end = children.end();

	for (;it != end; ++it)
	{
		Gump *g = (*it)->FindGump(t,recursive,no_inhertance);
			
		if (g) return g;
	}

	return 0;
}

// Makes this gump the focus
void Gump::MakeFocus()
{
	// By default we WONT do anything
	if (parent)
	{
		if (parent->focus_child) parent->focus_child->OnFocus(false);
		parent->focus_child = this;
	}
	OnFocus(true);
}

void Gump::FindNewFocusChild()
{
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;

	// Now add the gump to use as the new focus
	std::list<Gump*>::reverse_iterator	it = children.rbegin();

	if (it != children.rend())
	{
		(*it)->MakeFocus();
	}
}


// Adds a child to the list
void Gump::AddChild(Gump *gump, bool take_focus)
{
	if (!gump) return;

	// Remove it if requrietd
	Gump *old_parent = gump->GetParent();
	if (old_parent) old_parent->RemoveChild(gump);

	// Now add the gump in the correct spot
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	for (;it != end; ++it)
	{
		Gump *other = *it;
		// If we are same layer as focus and we wont take it, we will not be placed in front of it
		if (!take_focus && other == focus_child && other->layer == gump->layer) break;

		// Lower layers get placed before higher layers
		if (other->layer > gump->layer) break;
	}

	// Now add it
	children.insert(it, gump);
	gump->parent = this;

	// Make the gump the focus if needed
	if (take_focus || !focus_child)
	{
		if (focus_child) focus_child->OnFocus(false);
		gump->OnFocus(true);
		focus_child = gump;
	}
}

// Remove a gump from the list
void Gump::RemoveChild(Gump *gump)
{
	if (!gump) return;

	// Remove it
	children.remove(gump);
	gump->parent = 0;

	// Remove focus, the give upper most gump the focus
	if (gump == focus_child) 
	{
		FindNewFocusChild();
	}
}

Gump * Gump::GetRootGump()
{
	if (!parent) return this;
	return parent->GetRootGump();
}

//
// Input handling
//

Gump *Gump::OnMouseDown(int button, int mx, int my)
{
	// Convert to local coords
	ParentToGump(mx,my);

	std::list<Gump*>::iterator	it = children.end();
	std::list<Gump*>::iterator	begin = children.begin();

	Gump *handled = 0;

	// Iterate children
	while (it != begin)
	{
		Gump *g = *--it;

		// It's got the point
		if (g->PointOnGump(mx,my)) handled = g->OnMouseDown(button, mx, my);

		if (handled) break;
	}

	return handled;
}

//
// KeyInput
//
bool Gump::OnKeyDown(int key)
{
	bool handled = false;
	return handled;
}

bool Gump::OnKeyUp(int key)
{
	bool handled = false;
	return handled;
}

bool Gump::OnTextInput(int unicode)
{
	bool handled = false;
	return handled;
}

// Colourless Protection

