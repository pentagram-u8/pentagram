/*
 *  Copyright (C) 2004  The Pentagram Team
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
#include "ReadableGump.h"

#include "TextWidget.h"
#include "GUIApp.h"
#include "GameData.h"
#include "Shape.h"
#include "GumpShapeFlex.h"
#include "ShapeFrame.h"
#include "UCMachine.h"
#include "GumpNotifyProcess.h"
#include "Item.h"
#include "ObjectManager.h"


#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ReadableGump,ModalGump);

ReadableGump::ReadableGump()
	: ModalGump()
{

}

ReadableGump::ReadableGump(ObjId owner, uint16 shape, int font, std::string msg) :
	ModalGump(0, 0, 100, 100, owner), shapenum(shape), fontnum(font), text(msg)
{
}

ReadableGump::~ReadableGump(void)
{
}

void ReadableGump::InitGump()
{
	ModalGump::InitGump();

	Shape* shape = GameData::get_instance()->getGumps()->getShape(shapenum);

	SetShape(shape, 0);

	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Gump *widget = new TextWidget(0, 0, text, fontnum, dims.w - 16, 0, Pentagram::Font::TEXT_CENTER);
	widget->InitGump();

	textwidget = widget->getObjId();

	// Add it to us
	AddChild(widget);
	widget->setRelativePosition(CENTER);
}

Gump *ReadableGump::OnMouseDown(int button, int mx, int my)
{
	// Scroll to next text, if possible
	Close();
	return this;
}

uint32 ReadableGump::I_readGrave(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	ARG_STRING(str);
	assert(item);

	GUIApp *app = p_dynamic_cast<GUIApp*>(GUIApp::get_instance());
	assert(app);

	Gump *desktop = app->getDesktopGump();
	Gump *gump = new ReadableGump(item->getObjId(), shape, 11, str);
	gump->InitGump();
	desktop->AddChild(gump);
	gump->setRelativePosition(CENTER);
	
	return gump->GetNotifyProcess()->getPid();
}

uint32 ReadableGump::I_readPlaque(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	ARG_STRING(str);
	assert(item);

	GUIApp *app = p_dynamic_cast<GUIApp*>(GUIApp::get_instance());
	assert(app);

	Gump *desktop = app->getDesktopGump();
	Gump *gump = new ReadableGump(item->getObjId(), shape, 10, str);
	gump->InitGump();
	desktop->AddChild(gump);
	gump->setRelativePosition(CENTER);
	
	return gump->GetNotifyProcess()->getPid();
}

void ReadableGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	ModalGump::saveData(ods);

	ods->write2(textwidget);
}

bool ReadableGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ModalGump::loadData(ids)) return false;

	textwidget = ids->read2();

	return true;
}

