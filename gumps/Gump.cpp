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
 *  GNU General Public License for more details.
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
#include "Kernel.h"
#include "World.h"
#include "IDataSource.h"
#include "ODataSource.h"
#include "ObjectManager.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(Gump,Object);

Gump::Gump()
	: Object(), parent(0), children(), moveOffsetX(0), moveOffsetY(0)
{
}

Gump::Gump(int inX, int inY, int Width, int Height, uint16 inOwner,
		   uint32 inFlags, sint32 inLayer) : 
	Object(), owner(inOwner), parent(0), x(inX), y(inY),
	dims(0,0,Width,Height), flags(inFlags), layer(inLayer), index(-1),
	shape(0), framenum(0), children(), focus_child(0), notifier(0),
	process_result(0), moveOffsetX(0), moveOffsetY(0)
{
	assignObjId(); // gumps always get an objid
}

Gump::~Gump()
{
	// Get rid of focus
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;

	// Delete all children
	std::list<Gump*>::iterator it = children.begin();
	std::list<Gump*>::iterator end = children.end();

	while (it != end)
	{
		Gump *g = *it;
		it = children.erase(it);
		delete g;
	}
}

void Gump::InitGump()
{
	if (owner) CreateNotifier();
}

void Gump::CreateNotifier()
{
	// Create us a GumpNotifyProcess
	GumpNotifyProcess *p = new GumpNotifyProcess(owner);
	p->setGump(this);
	notifier = Kernel::get_instance()->addProcess(p);
}

GumpNotifyProcess* Gump::GetNotifyProcess()
{
	return p_dynamic_cast<GumpNotifyProcess*>(Kernel::get_instance()->
											  getProcess(notifier));
}


void Gump::Close(bool no_del)
{
	GumpNotifyProcess* p = GetNotifyProcess();
	if (p) {
		p->notifyClosing(process_result);
	}
	notifier = 0;

	if (!parent) 
	{
		flags |= FLAG_CLOSING;
		if (!no_del) delete this;
	}
	else
	{
		flags |= FLAG_CLOSING;
		if (!no_del) flags |= FLAG_CLOSE_AND_DEL;
	}
}

bool Gump::Run(const uint32 framenum)
{
	// Iterate all children
	std::list<Gump*>::iterator it = children.begin();
	std::list<Gump*>::iterator end = children.end();
	bool repaint = false;

	while (it != end)
	{
		Gump *g = *it;

		// Run the child if it's not closing
		if (!(g->flags & FLAG_CLOSING)) if (g->Run(framenum)) repaint = true;

		// If closing, we can kill it
		if (g->flags & FLAG_CLOSING)
		{
			it = children.erase(it);
			FindNewFocusChild();
			 if (g->flags & FLAG_CLOSE_AND_DEL) delete g;
		}
		else
		{
			++it;
		}
	}	
	return repaint;
}

void Gump::MapChanged()
{
	// Close it, and return
	if (flags & FLAG_ITEM_DEPENDANT) 
	{
		Close();
		return;
	}

	// Pass the MapChanged message to all the children
	std::list<Gump*>::iterator it = children.begin();
	std::list<Gump*>::iterator end = children.end();

	while (it != end)
	{
		Gump *g = *it;

		// Pass to child if it's not closing
		if (!(g->flags & FLAG_CLOSING)) g->MapChanged();

		// If closing, we can kill it
		if (g->flags & FLAG_CLOSING)
		{
			it = children.erase(it);
			FindNewFocusChild();
			if (g->flags & FLAG_CLOSE_AND_DEL) delete g;
		}
		else
		{
			++it;
		}
	}	
}

bool Gump::GetMouseCursor(int mx, int my, Shape &shape, sint32 &frame)
{
	ParentToGump(mx,my);


	bool ret = false;

	// This reverse iterates the children
	std::list<Gump*>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it)
	{
		Gump *g = *--it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		// It's got the point
		if (g->PointOnGump(mx,my))
			ret = g->GetMouseCursor(mx, my, shape, frame);

		if (ret) break;
	}

	return ret;
}

void Gump::Paint(RenderSurface *surf, sint32 lerp_factor)
{
	// Don't paint if hidden
	if (IsHidden()) return;

	// Get old Origin
	int ox=0, oy=0;
	surf->GetOrigin(ox, oy);

	// Set the new Origin
	int nx=0, ny=0;
	GumpToParent(nx,ny);
	surf->SetOrigin(ox+nx, oy+ny);

	// Get Old Clipping Rect
	Pentagram::Rect old_rect;
	surf->GetClippingRect(old_rect);

	// Set new clipping rect
	Pentagram::Rect new_rect = old_rect;
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
#if 0
		Pentagram::Rect sr;
		ShapeFrame *f = shape->getFrame(framenum);
		sr.h = f->height;
		sr.w = f->height;
		sr.x = -f->xoff;
		sr.y = -f->yoff;

		if (surf->CheckClipped(sr))
#endif
			surf->Paint(shape, framenum, 0, 0);
#if 0
		else
			surf->PaintNoClip(shape, framenum, 0, 0);
#endif
	}
}

void Gump::PaintChildren(RenderSurface *surf, sint32 lerp_factor)
{
	// Iterate all children
	std::list<Gump*>::iterator it = children.begin();
	std::list<Gump*>::iterator end = children.end();

	while (it != end)
	{
		Gump *g = *it;
		// Paint if not closing
		if (!(g->flags & FLAG_CLOSING)) 
			g->Paint(surf, lerp_factor);

		++it;
	}	
}

Gump* Gump::FindGump(int mx, int my)
{
	int gx = mx, gy = my;
	ParentToGump(gx, gy);
	Gump* gump = 0;

	// Iterate all children
	std::list<Gump*>::reverse_iterator it = children.rbegin();
	std::list<Gump*>::reverse_iterator end = children.rend();

	while (it != end && !gump)
	{
		Gump *g = *it;
		gump = g->FindGump(gx, gy);
		++it;
	}

	// it's over a child
	if (gump) return gump;

	// it's over this gump
	if (PointOnGump(mx, my)) return this;

	return 0;		
}


bool Gump::PointOnGump(int mx, int my)
{
	int gx = mx, gy = my;
	ParentToGump(gx,gy);

	// First check again rectangle
	if (!dims.InRect(gx,gy)) {
		return false;
	}

	if (shape) {
		ShapeFrame* sf = shape->getFrame(framenum);
		assert(sf);
		if (!sf->hasPoint(gx, gy)) {
			return false;
		}
	}
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

uint16 Gump::TraceObjId(int mx, int my)
{
	// Convert to local coords
	int gx = mx,gy = my;
	ParentToGump(gx,gy);

	uint16 objid = 0;

	// reverse-iterate children
	std::list<Gump*>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it)
	{
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		// It's got the point
		if (g->PointOnGump(gx,gy)) objid = g->TraceObjId(gx, gy);

		if (objid && objid != 65535) break;
	}

//	if (!objid || objid == 65535)
//		if (PointOnGump(mx,my))
//			objid = getObjId();

	return objid;
}

bool Gump::GetLocationOfItem(uint16 itemid, int &gx, int &gy,
							 sint32 lerp_factor)
{
	gx = 0;
	gy = 0;
	return false;
}

// Find a child gump of the specified type
Gump* Gump::FindGump(const RunTimeClassType &t, bool recursive,
					 bool no_inheritance)
{
	// If that is our type, then return us!
	if (GetClassType() == t) return this;
	else if (!no_inheritance && IsOfType(t)) return this;

	// Iterate all children
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	for (;it != end; ++it)
	{
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		if (g->GetClassType() == t) return g;
		else if (!no_inheritance && g->IsOfType(t)) return g;
	}

	if (!recursive) return 0;

	// Recursive Iterate all children
	it = children.begin();
	end = children.end();

	for (;it != end; ++it)
	{
		Gump *g = (*it);
			
		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

		g = g->FindGump(t,recursive,no_inheritance);

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

	// Remove it if required
	Gump *old_parent = gump->GetParent();
	if (old_parent) old_parent->RemoveChild(gump);

	// Now add the gump in the correct spot
	std::list<Gump*>::iterator	it = children.begin();
	std::list<Gump*>::iterator	end = children.end();

	for (;it != end; ++it)
	{
		Gump *other = *it;

		// Why don't we check for FLAG_CLOSING here?
		// Because we want to make sure that the sort order is always valid

		// If we are same layer as focus and we wont take it, we will not be
		// placed in front of it
		if (!take_focus && other == focus_child && other->layer == gump->layer)
			break;

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


bool Gump::StartDraggingChild(Gump* gump, int mx, int my)
{
	gump->ParentToGump(mx, my);
	gump->SetMoveOffset(mx, my);
	return true;
}

void Gump::DraggingChild(Gump* gump, int mx, int my)
{
	gump->Move(mx, my);
}

void Gump::StopDraggingChild(Gump* gump)
{
	gump->SetMoveOffset(0, 0);
}

//
// Input handling
//

Gump *Gump::OnMouseDown(int button, int mx, int my)
{
	// Convert to local coords
	ParentToGump(mx,my);

	Gump *handled = 0;

	// Iterate children backwards
	std::list<Gump*>::reverse_iterator it;
	for (it = children.rbegin(); it != children.rend(); ++it) {
		Gump *g = *it;

		// Not if closing
		if (g->flags & FLAG_CLOSING) continue;

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

void Gump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Object::saveData(ods);

	ods->write2(owner);
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(dims.x));
	ods->write4(static_cast<uint32>(dims.y));
	ods->write4(static_cast<uint32>(dims.w));
	ods->write4(static_cast<uint32>(dims.h));
	ods->write4(flags);
	ods->write4(static_cast<uint32>(layer));
	ods->write4(static_cast<uint32>(index));
	ods->write4(0); //!!TODO: shape
	ods->write4(framenum);
	if (focus_child)
		ods->write2(focus_child->getObjId());
	else
		ods->write2(0);
	ods->write2(notifier);
	ods->write4(process_result);
	// write children:
	ods->write4(children.size());
	std::list<Gump*>::iterator it;
	for (it = children.begin(); it != children.end(); ++it) {
		(*it)->save(ods);
	}
}

bool Gump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Object::loadData(ids)) return false;

	owner = ids->read2();
	x = static_cast<sint32>(ids->read4());
	y = static_cast<sint32>(ids->read4());

	int dx = static_cast<sint32>(ids->read4());
	int dy = static_cast<sint32>(ids->read4());
	int dw = static_cast<sint32>(ids->read4());
	int dh = static_cast<sint32>(ids->read4());
	dims.Set(dx, dy, dw, dh);

	flags = ids->read4();
	layer = static_cast<sint32>(ids->read4());
	index = static_cast<sint32>(ids->read4());
	ids->read4(); //!!TODO: shape
	shape = 0;
	framenum = ids->read4();
	uint16 focusid = ids->read2();
	focus_child = 0;
	notifier = ids->read2();
	process_result = ids->read4();

	// read children
	uint32 childcount = ids->read4();
	for (unsigned int i = 0; i < childcount; ++i) {
		Object* obj = ObjectManager::get_instance()->loadObject(ids);
		Gump* child = p_dynamic_cast<Gump*>(obj);
		if (!child) return false;

		AddChild(child, false);

		if (child->getObjId() == focusid)
			focus_child = child;

	}

	return true;
}

// Colourless Protection
