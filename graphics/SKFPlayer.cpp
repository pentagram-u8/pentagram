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
#include "SKFPlayer.h"

#include "u8/ConvertShapeU8.h"
#include "Flex.h"
#include "Shape.h"
#include "Texture.h"
#include "SoftRenderSurface.h"
#include "PaletteManager.h"
#include "MusicProcess.h"
#include "IDataSource.h"

enum SKFAction {
	SKF_PlayMusic = 3,
	SKF_SlowStopMusic = 4,
	SKF_PlaySFX = 5,
	SKF_StopSFX = 6,
	SKF_SetSpeed = 7,
	SKF_FadeOut = 8,
	SKF_FadeIn = 9,
	SKF_Wait = 12,
	SKF_PlaySound = 14,
	SKF_FadeWhite = 15,
	SKF_ClearSubs = 18
};

struct SKFEvent {
	unsigned int frame;
	SKFAction action;
	unsigned int data;
};

// number of steps in a fade
static const int FADESTEPS = 16; // HACK: half speed


SKFPlayer::SKFPlayer(Flex* movie, int width_, int height_)
	: width(width_), height(height_), skf(movie),
	  curframe(0), curobject(0), curaction(0), curevent(0), playing(false),
	  timer(0), fadecolour(0), fadelevel(0), buffer(0)
{
	IDataSource* eventlist = skf->get_datasource(0);
	if (!eventlist)
	{
		perr << "No eventlist found in SKF" << std::endl;
		return;
	}

	parseEventList(eventlist);
	delete eventlist;

	buffer = RenderSurface::CreateSecondaryRenderSurface(width, height);
}

SKFPlayer::~SKFPlayer()
{
	for (unsigned int i = 0; i < events.size(); ++i)
		delete events[i];

	delete skf;
	delete buffer;
}

void SKFPlayer::parseEventList(IDataSource* eventlist)
{
	uint16 frame = eventlist->read2();
	while (frame != 0xFFFF) {
		SKFEvent* ev = new SKFEvent;
		ev->frame = frame;
		ev->action = static_cast<SKFAction>(eventlist->read2());
		ev->data = eventlist->read2();
		events.push_back(ev);

		frame = eventlist->read2();
	}
}

void SKFPlayer::start()
{
	buffer->BeginPainting();
	buffer->Fill32(0, 0, 0, width, height);
	buffer->EndPainting();
	MusicProcess* musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
	playing = true;
}

void SKFPlayer::stop()
{
	MusicProcess* musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
	playing = false;
}

void SKFPlayer::paint(RenderSurface* surf, int /*lerp*/)
{
	if (!buffer) return;

	Texture* tex = buffer->GetSurfaceAsTexture();

	if (!fadelevel)
		surf->Blit(tex, 0, 0, width, height, 0, 0);
	else {
		uint32 fade = TEX32_PACK_RGBA(fadecolour,fadecolour,fadecolour,
									  (fadelevel*255)/FADESTEPS);
		surf->FadedBlit(tex, 0, 0, width, height, 0, 0, fade);
	}
}

void SKFPlayer::run()
{
	if (!playing || !buffer) return;

	// if doing something, continue
	if (curaction) {
		if (curaction == SKF_FadeOut || curaction == SKF_FadeWhite) {
			fadelevel++;
			if (fadelevel == FADESTEPS) curaction = 0; // done
		} else if (curaction == SKF_FadeIn) {
			fadelevel--;
			if (fadelevel == 0) curaction = 0; // done
		} else {
			pout << "Unknown fade action: " << curaction << std::endl;
		}
	}

	// if waiting, continue to wait
	if (timer) {
		timer--;
		return;
	}

	MusicProcess* musicproc = MusicProcess::get_instance();

	// handle events for the current frame
	while (curevent < events.size() && events[curevent]->frame <= curframe) {
		pout << "event " << curevent << std::endl;
		switch (events[curevent]->action)
		{
		case SKF_FadeOut:
			curaction = SKF_FadeOut;
			fadecolour = 0;
			fadelevel = 0;
			pout << "FadeOut" << std::endl;
			break;
		case SKF_FadeIn:
			curaction = SKF_FadeIn;
			fadelevel = FADESTEPS;
			pout << "FadeIn" << std::endl;
			break;
		case SKF_FadeWhite:
			curaction = SKF_FadeWhite;
			fadecolour = 0xFF;
			fadelevel = 0;
			pout << "FadeWhite" << std::endl;
			break;
		case SKF_Wait:
			pout << "Wait " << events[curevent]->data << std::endl;
			timer = events[curevent]->data * 2; //HACK: half speed...
			curevent++;
			return;
		case SKF_PlayMusic:
			pout << "PlayMusic " << events[curevent]->data << std::endl;
			if (musicproc) musicproc->playMusic(events[curevent]->data);
			break;
		case SKF_SlowStopMusic:
			pout << "SlowStopMusic" << std::endl;
			if (musicproc) musicproc->playMusic(0);
			break;
		case SKF_PlaySFX:
			pout << "PlaySFX " << events[curevent]->data << std::endl;
			break;
		case SKF_StopSFX:
			pout << "StopSFX" << std::endl;
			break;
		case SKF_SetSpeed:
			pout << "SetSpeed " << events[curevent]->data << std::endl;
			break;
		case SKF_PlaySound:
			pout << "PlaySound " << events[curevent]->data << std::endl;
			break;
		case SKF_ClearSubs:
			pout << "ClearSubs" << std::endl;
			break;
		default:
			pout << "Unknown action" << std::endl;
			break;
		}

		curevent++;
	}

	curframe++;

	PaletteManager* palman = PaletteManager::get_instance();
	IDataSource* object;

	uint16 objecttype = 0;
	do {
		curobject++;
		if (curobject >= skf->get_count()) {
			stop(); // done
			return;
		}

		// read object
		object = skf->get_datasource(curobject);
		if (!object || object->getSize() < 2)
			continue;

		objecttype = object->read2();

		pout << "Object " << curobject << "/" << skf->get_count() << ", type = " << objecttype << std::endl;


		if (objecttype == 1) {
			palman->load(PaletteManager::Pal_Movie, *object);
		}

		if (objecttype != 2)
			delete object;

	} while (objecttype == 1);

	if (objecttype == 2) {
		object->seek(0);
		Shape* shape = new Shape(object, &U8SKFShapeFormat);
		Pentagram::Palette* pal= palman->getPalette(PaletteManager::Pal_Movie);
		shape->setPalette(pal);
		buffer->BeginPainting();
		buffer->Paint(shape, 0, 0, 0);
		buffer->EndPainting();
		delete shape;
	
		delete object;
	}

	timer = 1; //HACK: half speed by waiting one frame after each image
}

