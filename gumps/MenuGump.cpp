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
#include "MenuGump.h"

#include "GameData.h"
#include "GumpShapeFlex.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GUIApp.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "QuitGump.h"
#include "ControlsGump.h"
#include "PagedGump.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MenuGump,ModalGump);

MenuGump::MenuGump(): ModalGump(0, 0, 5, 5)
{
	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	app->setMouseCursor(GUIApp::MOUSE_HAND);
}

MenuGump::~MenuGump()
{
	GUIApp::get_instance()->popMouseCursor();
}

static const int gumpShape = 35;
static const int pagenShape = 32;
static const int menuEntryShape = 37;

void MenuGump::InitGump()
{
	ModalGump::InitGump();
	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Shape* logoShape = GameData::get_instance()->getGumps()->getShape(pagenShape);
	sf = logoShape->getFrame(0);
	assert(sf);

	Gump * logo = new Gump(42, 10, sf->width, sf->height);
	logo->SetShape(logoShape, 0);
	logo->InitGump();
	AddChild(logo);

	Shape* entryShape = GameData::get_instance()->getGumps()->getShape(menuEntryShape);
	sf = shape->getFrame(0);
	assert(sf);

	// Language offset = currently 0, english only
	int lang = 0;

	int x = dims.w / 2 + 14;
	int y = 18;
	Gump * widget;
	Pentagram::Rect rect;
	for (int i = 0; i < 8; ++i)
	{
		widget = new ButtonWidget(x, y, entryShape, i * 2 + lang, entryShape, i * 2 + 1 + lang);
		widget->InitGump();
		AddChild(widget);
		entryGumps[i] = widget->getObjId();
		widget->GetDims(rect);
		y+= rect.h + 1;
	}
	
	// Should be Avatar's name.
	widget = new TextWidget(0, 0, "Pentagram", 6);
	widget->InitGump();
	widget->GetDims(rect);
	widget->Move(90 - rect.w / 2, dims.h - 40);
	AddChild(widget);
}


void MenuGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);
}

bool MenuGump::OnKeyDown(int key, int mod)
{
	switch (key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	case SDLK_1:
	{	// Intro
	} break;
	case SDLK_2:
	{	// Read Diary
		// I'm lazy - MJ
		GUIApp::get_instance()->loadGame("@save/quicksave");
	} break;
	case SDLK_3:
	{	// Write Diary
		// I'm lazy - MJ
		GUIApp::get_instance()->saveGame("@save/quicksave");
	} break;	
	case SDLK_4:
	{	// Options
		PagedGump * gump = new PagedGump(36, -40, 4, gumpShape);
		gump->InitGump();
		gump->addGump("Engine", ControlsGump::showEngineMenu());
		gump->addGump("Game", ControlsGump::showU8Menu());
		AddChild(gump);
		gump->setRelativePosition(CENTER);
	} break;
	case SDLK_5:
	{	// Credits
	} break;
	case SDLK_6:
	{	// Quit
		QuitGump::verifyQuit();
	} break;
	case SDLK_7:
	{	// Quotes
	} break;	
	case SDLK_8:
	{	// End Game
	} break;	
	default:
		break;
	}

	return true;
}

void MenuGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (cid == entryGumps[i])
			{
				switch (i + 1)
				{
				case 1:
				{	// Intro
				} break;
				case 2:
				{	// Read Diary
					// I'm lazy - MJ
					GUIApp::get_instance()->loadGame("@save/quicksave");
				} break;
				case 3:
				{	// Write Diary
					// I'm lazy - MJ
					GUIApp::get_instance()->saveGame("@save/quicksave");
				} break;	
				case 4:
				{	// Options
					PagedGump * gump = new PagedGump(36, -40, 4, gumpShape);
					gump->InitGump();
					gump->addGump("Engine", ControlsGump::showEngineMenu());
					gump->addGump("Game", ControlsGump::showU8Menu());
					AddChild(gump);
					gump->setRelativePosition(CENTER);
				} break;
				case 5:
				{	// Credits
				} break;
				case 6:
				{	// Quit
					QuitGump::verifyQuit();
				} break;
				case 7:
				{	// Quotes
				} break;	
				case 8:
				{	// End Game
				} break;	
				default:
					break;
				}
			}
		}
	}
}

bool MenuGump::OnTextInput(int unicode)
{
	switch (unicode)
	{
	default:
		break;
	}

	return true;
}

//static
void MenuGump::showMenu()
{
	Gump* desktopGump = GUIApp::get_instance()->getDesktopGump();
	ModalGump* gump = new MenuGump();
	gump->InitGump();
	desktopGump->AddChild(gump);
	gump->setRelativePosition(CENTER);
}

bool MenuGump::loadData(IDataSource* ids)
{
	return true;
}

void MenuGump::saveData(ODataSource* ods)
{
}

