/*
 *  Copyright (C) 2003-2005  The Pentagram Team
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
#include "SliderGump.h"

#include "GameData.h"
#include "ShapeFrame.h"
#include "GumpShapeArchive.h"
#include "Shape.h"
#include "SlidingWidget.h"
#include "Font.h"
#include "RenderedText.h"
#include "FontManager.h"
#include "ButtonWidget.h"
#include "UCProcess.h"
#include "Kernel.h"
#include "GUIApp.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(SliderGump,ModalGump);

SliderGump::SliderGump() : ModalGump()
{
	renderedtext = 0;
}


SliderGump::SliderGump(int x, int y, sint16 min_, sint16 max_,
					   sint16 value_, sint16 delta_)
	: ModalGump(x, y, 5, 5), min(min_), max(max_), delta(delta_), value(value_)
{
	usecodeNotifyPID = 0;
	renderedtext = 0;
}

SliderGump::~SliderGump()
{

}

/*
  41:0 = slider gump
  42:0,1 = ok button
  43:0,1 = left button
  44:0,1 = right button
  45:0 = slider
*/

static const int gumpshape = 41;
static const int gumpframe = 0;
static const int okshape = 42;
static const int leftshape = 43;
static const int rightshape = 44;
static const int slidershape = 45;
static const int sliderframe = 0;
static const int slidery = 17;
static const int sliderminx = 55;
static const int slidermaxx = 130;
static const int labelx = 161;
static const int labely = 26;
static const int labelfont = 0;

static const int OK_INDEX = 1;
static const int LEFT_INDEX = 2;
static const int RIGHT_INDEX = 3;
static const int SLIDER_INDEX = 4;

int SliderGump::getSliderPos()
{
	return sliderminx + (value-min)*(slidermaxx-sliderminx)/(max-min);
}

void SliderGump::setValueFromSlider(int sliderx)
{
	int val = (sliderx-sliderminx)*(max-min)/(slidermaxx-sliderminx) + min;
	if (val < min) val = min;
	if (val > max) val = max;
	value = min + delta*(static_cast<sint16>(val/delta));
}

void SliderGump::setSliderPos()
{
	Gump* slider = Gump::FindGump(SlidingWidget::ClassType);
	assert(slider);
	slider->Move(getSliderPos(), slidery);
}

void SliderGump::drawText(RenderSurface* surf)
{
	if (!renderedtext || value != renderedvalue) {
		Pentagram::Font *font;
		font = FontManager::get_instance()->getGameFont(labelfont);
		char buf[10]; // more than enough for a sint16
		sprintf(buf, "%d", value);

		unsigned int remaining;
		delete renderedtext;
		renderedtext = font->renderText(buf, remaining);
		renderedvalue = value;
	}

	renderedtext->draw(surf, labelx, labely);
}

void SliderGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	Gump::PaintThis(surf, lerp_factor, scaled);

	drawText(surf);
}


void SliderGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);

	shape = GameData::get_instance()->getGumps()->getShape(gumpshape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	Shape* childshape = GameData::get_instance()->
		getGumps()->getShape(slidershape);

	// Create the SlidingWidget
	Gump *widget = new SlidingWidget(getSliderPos(),slidery,
									 childshape,sliderframe);
	widget->SetIndex(SLIDER_INDEX);
	widget->InitGump(this);

	FrameID button_up(GameData::GUMPS, okshape, 0);
	FrameID button_down(GameData::GUMPS, okshape, 1);

	widget = new ButtonWidget(14, 17, button_up, button_down);
	widget->SetIndex(OK_INDEX);
	widget->InitGump(this);

	FrameID buttonleft_up(GameData::GUMPS, leftshape, 0);
	FrameID buttonleft_down(GameData::GUMPS, leftshape, 1);

	widget = new ButtonWidget(36, 17, buttonleft_up, buttonleft_down);
	widget->SetIndex(LEFT_INDEX);
	widget->InitGump(this);


	FrameID buttonright_up(GameData::GUMPS, rightshape, 0);
	FrameID buttonright_down(GameData::GUMPS, rightshape, 1);

	widget = new ButtonWidget(141, 17, buttonright_up, buttonright_down);
	widget->SetIndex(RIGHT_INDEX);
	widget->InitGump(this);
}

void SliderGump::ChildNotify(Gump *child, uint32 message)
{
	if (message == ButtonWidget::BUTTON_CLICK) {
		switch (child->GetIndex()) {
		case OK_INDEX:
			Close();
			break;
		case LEFT_INDEX:
			value -= delta;
			if (value < min) value = min;
			setSliderPos();
			break;
		case RIGHT_INDEX:
			value += delta;
			if (value > max) value = max;
			setSliderPos();
			break;
		}
	}
}


void SliderGump::Close(bool no_del)
{
	process_result = value;

	if (usecodeNotifyPID) {
		UCProcess* ucp = p_dynamic_cast<UCProcess*>(Kernel::get_instance()->getProcess(usecodeNotifyPID));
		assert(ucp);
		ucp->setReturnValue(value);
		ucp->wakeUp(value);
	}

	ModalGump::Close(no_del);
}

bool SliderGump::StartDraggingChild(Gump* gump, int mx, int my)
{
	if (gump->GetIndex() == SLIDER_INDEX) {
		gump->ParentToGump(mx, my);
		GUIApp::get_instance()->setDraggingOffset(mx, 0);
		return true;
	}

	return false;
}

void SliderGump::DraggingChild(Gump* gump, int mx, int my)
{
	if (gump->GetIndex() == SLIDER_INDEX) {
		int dox, doy;
		GUIApp::get_instance()->getDraggingOffset(dox, doy);
		setValueFromSlider(mx-dox);
		gump->Move(getSliderPos(), slidery);
	}
}

void SliderGump::StopDraggingChild(Gump* gump)
{
}

bool SliderGump::OnKeyDown(int key, int mod)
{
	switch(key)
	{
	case SDLK_LEFT:
		value -= delta;
		if (value < min) value = min;
		setSliderPos();
		break;
	case SDLK_RIGHT:
		value += delta;
		if (value > max) value = max;
		setSliderPos();
		break;
	case SDLK_RETURN:
		Close();
		break;
	default:
		break;
	}

	return true;
}

void SliderGump::setUsecodeNotify(UCProcess* ucp)
{
	assert(ucp);
	usecodeNotifyPID = ucp->getPid();
}

void SliderGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool SliderGump::loadData(IDataSource* ids, uint32 version)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}
