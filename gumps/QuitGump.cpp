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
#include "QuitGump.h"

#include "GameData.h"
#include "GumpShapeFlex.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GUIApp.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(QuitGump,ModalGump);

QuitGump::QuitGump(): ModalGump(0, 0, 5, 5)
{
	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	app->setMouseCursor(GUIApp::MOUSE_HAND);
}

QuitGump::~QuitGump()
{
	GUIApp::get_instance()->popMouseCursor();
}

static const int gumpShape = 17;
static const int askShapeId = 18;
static const int yesShapeId = 47;
static const int noShapeId = 50;

void QuitGump::InitGump()
{
	ModalGump::InitGump();
	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Shape* askShape = GameData::get_instance()->getGumps()->getShape(askShapeId);
	sf = askShape->getFrame(0);
	assert(sf);

	// Language offset = currently 0, english only
	int lang = 0;

	Gump * ask = new Gump(6, 6, sf->width, sf->height);
	ask->SetShape(askShape, lang);
	ask->InitGump();
	AddChild(ask);

 	askShape = GameData::get_instance()->getGumps()->getShape(yesShapeId + lang);
	Gump * widget;
	widget = new ButtonWidget(16, 38, askShape, 0, askShape, 1);
	widget->InitGump();
	AddChild(widget);
	yesWidget = widget->getObjId();

 	askShape = GameData::get_instance()->getGumps()->getShape(noShapeId + lang);
	widget = new ButtonWidget(dims.w / 2 + 8, 38, askShape, 0, askShape, 1);
	widget->InitGump();
	AddChild(widget);
	noWidget = widget->getObjId();
}


void QuitGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);
}

bool QuitGump::OnKeyDown(int key, int mod)
{
	switch (key)
	{
	case SDLK_BACKSPACE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

void QuitGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		if (cid == yesWidget)
		{
			GUIApp::get_instance()->ForceQuit();
		}
		else if (cid == noWidget)
		{
			Close();
		}
	}
}

bool QuitGump::OnTextInput(int unicode)
{
	switch (unicode)
	{
	case 'y': case 'Y':
	{
		GUIApp::get_instance()->ForceQuit();
	} break;
	case 'n': case 'N':
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

//static
void QuitGump::verifyQuit()
{
	Gump* desktopGump = GUIApp::get_instance()->getDesktopGump();
	ModalGump* gump = new QuitGump();
	gump->InitGump();
	desktopGump->AddChild(gump);
	gump->setRelativePosition(CENTER);
}

bool QuitGump::loadData(IDataSource* ids)
{
	return true;
}

void QuitGump::saveData(ODataSource* ods)
{
}

