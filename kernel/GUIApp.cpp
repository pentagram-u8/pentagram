/*
Copyright (C) 2002-2003 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "GUIApp.h"

#include "Kernel.h"
#include "FileSystem.h"
#include "Configuration.h"

#include "RenderSurface.h"
#include "Texture.h"
#include "PaletteManager.h"
#include "GameData.h"
#include "World.h"

#include "U8Save.h"

#include "Gump.h"
#include "DesktopGump.h"
#include "ConsoleGump.h"
#include "GameMapGump.h"
#include "BarkGump.h"

// TODO MOVE THIS STUFF TO GameMapGump or somewhere else
#include "Item.h"
#include "Actor.h"
#include "MainActor.h"
#include "ItemSorter.h"
#include "CurrentMap.h"
#include "Rect.h"
#include "CameraProcess.h"
// END TODO

// just for testing
#include "Actor.h"
#include "ActorAnimProcess.h"
#include "Font.h"
#include "FontShapeFlex.h"
#include "u8intrinsics.h"
#include "Egg.h"

#include <SDL.h>

#include "DisasmProcess.h"
#include "CompileProcess.h"

#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
#include <windows.h>
#include "resource.h"
#endif

using std::string;

DEFINE_RUNTIME_CLASSTYPE_CODE(GUIApp,CoreApp);

GUIApp::GUIApp(const int argc, const char * const * const argv)
	: CoreApp(argc, argv, "u8", true), ucmachine(0), screen(0),
	  palettemanager(0), gamedata(0), world(0), desktopGump(0),
	  consoleGump(0), gameMapGump(0),
	  runGraphicSysInit(false), runSDLInit(false),
	  frameSkip(false), frameLimit(true), interpolate(true),
	  animationRate(33), avatarInStasis(false), painting(false)
{
	// Set the console to auto paint, till we have finished initing
	con.SetAutoPaint(conAutoPaint);

	application = this;

	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine(U8Intrinsics);

	postInit(argc, argv);

	GraphicSysInit();

	U8Playground();

	// Unset the console auto paint, since we have finished initing
	con.SetAutoPaint(0);

	for (int i = 0; i < NUM_MOUSEBUTTONS+1; ++i) {
		mouseDownGump[i] = 0;
		lastMouseDown[i] = 0;
		mouseState[i] = MBS_HANDLED;
	}
}

GUIApp::~GUIApp()
{
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(palettemanager);
}

void GUIApp::run()
{
	isRunning = true;

	sint32 next_ticks = SDL_GetTicks();	// Next time is right now!
	
	// Ok, the theory is that if this is set to true then we must do a repaint
	// At the moment only it's ignored
	bool repaint;

	SDL_Event event;
	while (isRunning) {
		inBetweenFrame = true;	// Will get set false if it's not an inBetweenFrame

		if (!frameLimit) {
			repaint = false;
			
			if (kernel->runProcesses(framenum)) repaint = true;
			desktopGump->Run(framenum);
			framenum++;
			inBetweenFrame = false;
			next_ticks = animationRate + SDL_GetTicks();
			lerpFactor = 256;
		}
		else 
		{
			sint32 ticks = SDL_GetTicks();
			sint32 diff = next_ticks - ticks;
			repaint = false;

			while (diff < 0) {
				next_ticks += animationRate;
				if (kernel->runProcesses(framenum)) repaint = true;
				desktopGump->Run(framenum);
				framenum++;
	
				inBetweenFrame = false;

				ticks = SDL_GetTicks();

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time. 
				if (!frameSkip && diff <= -animationRate*2) next_ticks = animationRate + ticks;

				diff = next_ticks - ticks;
				if (!frameSkip) break;
			}

			// Calculate the lerp_factor
			lerpFactor = ((animationRate-diff)*256)/animationRate;
			//pout << "lerpFactor: " << lerpFactor << " framenum: " << framenum << std::endl;
			if (!interpolate || lerpFactor > 256) lerpFactor = 256;
		}


		repaint = true;

		// get & handle all events in queue
		while (isRunning && SDL_PollEvent(&event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Paint Screen
		paint();
	}
}

void GUIApp::U8Playground()
{
	inBetweenFrame = 0;
	lerpFactor = 256;

	// Load palette
	pout << "Load Palette" << std::endl;
	palettemanager = new PaletteManager(screen);
	IDataSource *pf = filesystem->ReadFile("@u8/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load static/u8pal.pal. Exiting" << std::endl;
		std::exit(-1);
	}
	pf->seek(4); // seek past header
	palettemanager->load(PaletteManager::Pal_Game, *pf, U8XFormFuncs);

	pout << "Load GameData" << std::endl;
	gamedata = new GameData();
	gamedata->loadU8Data();

	// Initialize world
	pout << "Initialize World" << std::endl;
	world = new World();

	IDataSource *saveds = filesystem->ReadFile("@u8/savegame/u8save.000");
	U8Save *u8save = new U8Save(saveds);

	IDataSource *nfd = u8save->get_datasource("NONFIXED.DAT");
	if (!nfd) {
		perr << "Unable to load savegame/u8save.000/NONFIXED.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	world->initMaps();
	world->loadNonFixed(nfd);
	delete nfd;
	IDataSource *icd = u8save->get_datasource("ITEMCACH.DAT");
	if (!icd) {
		perr << "Unable to load savegame/u8save.000/ITEMCACH.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	IDataSource *npcd = u8save->get_datasource("NPCDATA.DAT");
	if (!npcd) {
		perr << "Unable to load savegame/u8save.000/NPCDATA.DAT. Exiting" << std::endl;
		std::exit(-1);
	}

	world->loadItemCachNPCData(icd, npcd);
	delete icd;
	delete npcd;
	delete u8save;
	delete saveds;

	Actor* av = world->getNPC(1);
//	av->teleport(40, 16240, 15240, 64); // central Tenebrae
//	av->teleport(3, 11391, 1727, 64); // docks, near gate
//	av->teleport(39, 16240, 15240, 64); // West Tenebrae
//	av->teleport(41, 12000, 15000, 64); // East Tenebrae

	if (av)
		world->switchMap(av->getMapNum());
	else
		world->switchMap(3);

	// Create GameMapGump
	Rect dims;
	screen->GetSurfaceDims(dims);
	
	pout << "Create GameMapGump" << std::endl;
	gameMapGump = new GameMapGump(0, 0, dims.w, dims.h);
	gameMapGump->InitGump();
	desktopGump->AddChild(gameMapGump);

	pout << "Create Camera" << std::endl;
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	pout << "Paint Inital display" << std::endl;
	paint();
}

// conAutoPaint hackery
void GUIApp::conAutoPaint(void)
{
	GUIApp *app = GUIApp::get_instance();
	if (app && !app->isPainting()) app->paint();
}

// Paint the screen
void GUIApp::paint()
{
	if(!runGraphicSysInit) // need to worry if the graphics system has been started. Need nicer way.
		return;

	painting = true;

	// Begin painting
	screen->BeginPainting();

	// We need to get the dims
	Rect dims;
	screen->GetSurfaceDims(dims);

	long before_gumps = SDL_GetTicks();
	desktopGump->Paint(screen, lerpFactor);
	long after_gumps = SDL_GetTicks();

	static long prev = 0;
	long now = SDL_GetTicks();
	long diff = now - prev;
	prev = now;

	char buf[256];
	snprintf(buf, 255, "Rendering time %li ms %li FPS - Paint Gumps %li ms", diff, 1000/diff, after_gumps-before_gumps);
	screen->PrintTextFixed(con.GetConFont(), buf, 8, dims.h-16);

	// End painting
	screen->EndPainting();

	painting = false;
}

void GUIApp::GraphicSysInit()
{
	// if we've already done this...
	if(runGraphicSysInit) return;
	//else...

	// Set Screen Resolution
	pout << "Set Video Mode" << std::endl;

	std::string fullscreen;
	config->value("config/video/fullscreen", fullscreen, "no");
	if (fullscreen!="yes") fullscreen="no";
	int width = 640, height = 480;
	screen = RenderSurface::SetVideoMode(width, height, 32, fullscreen=="yes", false);

	if (!screen)
	{
		perr << "Unable to set video mode. Exiting" << std::endl;
		std::exit(-1);
	}

	pout << "Create Desktop" << std::endl;
	desktopGump = new DesktopGump(0,0, width, height);
	desktopGump->InitGump();
	desktopGump->MakeFocus();

	pout << "Create Graphics Console" << std::endl;
	consoleGump = new ConsoleGump(0, 0, width, height);
	consoleGump->InitGump();
	desktopGump->AddChild(consoleGump);

	LoadConsoleFont();

	// Create desktop
	Rect dims;
	screen->GetSurfaceDims(dims);

	runGraphicSysInit=true;

	// Do initial console paint
	paint();
}

void GUIApp::LoadConsoleFont()
{
#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
	HRSRC res = FindResource(NULL,  MAKEINTRESOURCE(IDR_FIXED_FONT_TGA), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.tga", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));

	res = FindResource(NULL, MAKEINTRESOURCE(IDR_FIXED_FONT_CFG), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.cfg", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));
#endif

	std::string data;
	std::string confontfile;
	std::string confontcfg("@data/fixedfont.cfg");

	pout << "Searching for alternate console font... ";
	config->value("config/general/console-font", data, "");
	if (data != "")
	{
		confontcfg = data;
		pout << "Found." << std::endl;
	}
	else
		pout << "Not Found." << std::endl;

	// try to load the file
	pout << "Loading console font config: " << confontcfg << "... ";
	Configuration *fontconfig = new Configuration();
	if(fontconfig->readConfigFile(confontcfg, "font"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;

	// search for the path to the font...
	fontconfig->value("font/path", confontfile, "");
	if(confontfile=="")
	{
		pout << "Error: Console font path not found! Unable to continue. Exiting." << std::endl;
		std::exit(-1);
	}

	// clean up
	delete fontconfig;

	// Load confont
	pout << "Loading Confont: " << confontfile << std::endl;
	IDataSource *cf = filesystem->ReadFile(confontfile.c_str());
	Texture *confont;
	if (cf) confont = Texture::Create(*cf, confontfile.c_str());
	else confont = 0;
	if (!confont)
	{
		perr << "Unable to load " << confontfile << ". Exiting" << std::endl;
		std::exit(-1);
	}
	delete cf;

	con.SetConFont(confont);
}

void GUIApp::handleEvent(const SDL_Event& event)
{
	extern int userchoice; // major hack... see Item::I_ask
	uint32 now = SDL_GetTicks();

	switch (event.type) {
	case SDL_QUIT:
	{
		isRunning = false;
	}
	break;

	case SDL_ACTIVEEVENT:
	{
		// pause when lost focus?
	}
	break;


	// most of these events will probably be passed to a gump manager,
	// since almost all (all?) user input will be handled by a gump

	case SDL_MOUSEBUTTONDOWN:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		mouseDownGump[button] =
			desktopGump->OnMouseDown(button, mx, my)->getObjId();
		mouseDownX[button] = mx;
		mouseDownY[button] = my;
		mouseState[button] |= MBS_DOWN;
		mouseState[button] &= ~MBS_HANDLED;

		if (now - lastMouseDown[button] < 200) { //!! constant
			Gump* gump = getGump(mouseDownGump[button]);
			if (gump)
				gump->OnMouseDouble(button, mx, my);
			mouseState[button] |= MBS_HANDLED;
		}
		lastMouseDown[button] = now;
	}
	break;

	case SDL_MOUSEBUTTONUP:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		mouseState[button] &= ~MBS_DOWN;

		Gump* gump = getGump(mouseDownGump[button]);
		if (gump)
			gump->OnMouseUp(button, mx, my);
	}
	break;

	case SDL_MOUSEMOTION:
	{

	}
	break;

	case SDL_KEYDOWN:
	{

	}
	break;

	case SDL_KEYUP:
	{
		switch (event.key.keysym.sym) {
		case SDLK_0: userchoice = 0; break;
		case SDLK_1: userchoice = 1; break;
		case SDLK_2: userchoice = 2; break;
		case SDLK_3: userchoice = 3; break;
		case SDLK_4: userchoice = 4; break;
		case SDLK_5: userchoice = 5; break;
		case SDLK_6: userchoice = 6; break;
		case SDLK_7: userchoice = 7; break;
		case SDLK_8: userchoice = 8; break;
		case SDLK_9: userchoice = 9; break;
		case SDLK_a: userchoice = 10; break;
		case SDLK_b: userchoice = 11; break;
		case SDLK_c: userchoice = 12; break;
		case SDLK_d: userchoice = 13; break;
		case SDLK_e: userchoice = 14; break;
		case SDLK_UP: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x-64,y-64,z);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			}
		} break;
		case SDLK_DOWN: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x+64,y+64,z);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			}
		} break;
		case SDLK_LEFT: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x-64,y+64,z);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			}
		} break;
		case SDLK_RIGHT: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x+64,y-64,z);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			}
		} break;
		case SDLK_BACKQUOTE: {
			if (consoleGump->IsHidden())
				consoleGump->UnhideGump();
			else
				consoleGump->HideGump();
			break;
		}
		case SDLK_ESCAPE: case SDLK_q: isRunning = false; break;
		case SDLK_PAGEUP: {
			if (!consoleGump->IsHidden()) con.ScrollConsole(-3);
			break;
		}
		case SDLK_PAGEDOWN: {
			if (!consoleGump->IsHidden()) con.ScrollConsole(3); 
			break;
		}
		//case SDLK_LEFTBRACKET: display_list->DecSortLimit(); break;
		//case SDLK_RIGHTBRACKET: display_list->IncSortLimit(); break;
		case SDLK_t: { // quick animation test

			if (!avatarInStasis) { 
                Actor* devon = World::get_instance()->getNPC(2);
				Process* p = new ActorAnimProcess(devon, 0, 2);
				Kernel::get_instance()->addProcess(p);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			} 
		} break;
		case SDLK_f: { // trigger 'first' egg
//			lx = 14527 + 3*512;
//			ly = 5887 + 3*512;
//			lz = 8;

			if (!avatarInStasis) {
				Egg* egg = p_dynamic_cast<Egg*>(Kernel::get_instance()->getObject(21185)); // *cough*
				if (!egg || egg->getQuality() != 36)
					egg = p_dynamic_cast<Egg*>(Kernel::get_instance()->getObject(21186)); // *cough*
				sint32 ix, iy, iz;
				egg->getLocation(ix,iy,iz);
				CameraProcess::SetCameraProcess(new CameraProcess(ix,iy,iz)); // Center on egg
				egg->hatch();
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			} 
		} break;
		case SDLK_g: { // trigger 'execution' egg
			if (!avatarInStasis) {
				Egg* egg = p_dynamic_cast<Egg*>(Kernel::get_instance()->getObject(21164)); // *cough*
				if (!egg || egg->getQuality() != 4)
					egg = p_dynamic_cast<Egg*>(Kernel::get_instance()->getObject(21165)); // *cough*
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				egg->getLocation(x,y,z);
				avatar->move(x,y,z);
				egg->hatch();
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			} 
		} break;
		default: break;
		}
	}
	break;

	// any more useful events?

	default:
		break;
	}
}

void GUIApp::handleDelayedEvents()
{
	uint32 now = SDL_GetTicks();
	for (int button = 0; button <= NUM_MOUSEBUTTONS; ++button) {
		if (!(mouseState[button] & (MBS_HANDLED | MBS_DOWN)) &&
			now - lastMouseDown[button] > 200) // !constant
		{
			Gump* gump = getGump(mouseDownGump[button]);
			if (gump)
				gump->OnMouseClick(button, mouseDownX[button],
								   mouseDownY[button]);

			mouseDownGump[button] = 0;
			mouseState[button] &= ~MBS_HANDLED;
		}
	}

}

Gump* GUIApp::getGump(uint16 gumpid)
{
	return p_dynamic_cast<Gump*>(Kernel::get_instance()->getObject(gumpid));
}

uint32 GUIApp::I_getCurrentTimerTick(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// number of ticks of a 60Hz timer, with the default animrate of 30Hz
	return get_instance()->getFrameNum()*2;
}

uint32 GUIApp::I_setAvatarInStasis(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(statis);
	get_instance()->setAvatarInStasis(statis!=0);
	return 0;
}

uint32 GUIApp::I_getAvatarInStasis(const uint8* /*args*/, unsigned int /*argsize*/)
{
	if (get_instance()->avatarInStasis)
		return 1;
	else
		return 0;
}


