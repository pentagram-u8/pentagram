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
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "GUIApp.h"
#include "ShapeFrame.h"
#include "Shape.h"
#include "Mouse.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(ButtonWidget,Gump);

ButtonWidget::ButtonWidget()
	: Gump()
{
}

ButtonWidget::ButtonWidget(int X, int Y, std::string txt, int font,
						   int w, int h) :
	Gump(X,Y,w,h), shape_up(0), shape_down(0)
{
	TextWidget* widget = new TextWidget(0,0,txt,font,w,h);
	textwidget = widget->getObjId();
}

ButtonWidget::ButtonWidget(int X, int Y, Shape* shape_up_, uint32 framenum_up_,
						   Shape* shape_down_, uint32 framenum_down_) :
	Gump(X,Y,5,5), shape_up(shape_up_), framenum_up(framenum_up_),
	shape_down(shape_down_), framenum_down(framenum_down_), textwidget(0)
{
	
}


ButtonWidget::~ButtonWidget(void)
{
}

void ButtonWidget::InitGump()
{
	if (textwidget != 0) {
		Gump* widget = GUIApp::get_instance()->getGump(textwidget);
		assert(widget);
		widget->InitGump();

		widget->GetDims(dims); // transfer child dimension to self
		AddChild(widget);
		widget->Move(0,dims.y); // move it to the correct height
	} else {
		assert(shape_up != 0);
		assert(shape_down != 0);

		shape = shape_up;
		framenum = framenum_up;

		ShapeFrame* sf = shape->getFrame(framenum);
		assert(sf);
		dims.w = sf->width;
		dims.h = sf->height;
	}
}

Gump *ButtonWidget::OnMouseDown(int button, int mx, int my)
{
	Gump *ret = Gump::OnMouseDown(button,mx,my);
	if (ret) return ret;
	if (button == BUTTON_LEFT)
	{
		// CHECKME: change dimensions or not?
		shape = shape_down;
		framenum = framenum_down;
		return this;
    }
	return 0;
}

uint16 ButtonWidget::TraceObjId(int mx, int my)
{
	if (PointOnGump(mx, my))
		return getObjId();
	else
		return 0;
}


void ButtonWidget::OnMouseUp(int button, int mx, int my)
{
	if (button == BUTTON_LEFT) {
		shape = shape_up;
		framenum = framenum_up;
		parent->ChildNotify(this,BUTTON_UP);
	}
}

void ButtonWidget::OnMouseClick(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_CLICK);
}

void ButtonWidget::OnMouseDouble(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_DOUBLE);
}

void ButtonWidget::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);

	ods->write4(0); //TODO: shape_up
	ods->write4(framenum_up);
	ods->write4(0); //TODO: shape_down
	ods->write4(framenum_down);
	ods->write2(textwidget);
}

bool ButtonWidget::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	shape_up = 0; ids->read4(); // TODO
	framenum_up = ids->read4();
	shape_down = 0; ids->read4(); // TODO
	framenum_down = ids->read4();
	textwidget = ids->read2();

	return true;
}
