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

#include "Application.h"
#include "Kernel.h"
#include "FileSystem.h"
#include "Configuration.h"

#include "UCMachine.h"
#include "UCProcess.h"
#include "UsecodeFlex.h"
#include "IDataSource.h"
#include "RenderSurface.h"
#include "Texture.h"
//#include "ConsoleGump.h"
#include "PaletteManager.h"
#include "MainShapeFlex.h"
#include "Palette.h"
#include "XFormBlend.h"
#include "GameData.h"
#include "World.h"
#include "Args.h"

#include "Map.h" // temp
#include "U8Save.h"

// TODO MOVE THIS STUFF TO GameMapGump or somewhere else
#include "Item.h"
#include "Actor.h"
#include "ItemSorter.h"
#include "CurrentMap.h"
#include "Rect.h"
#include "CameraProcess.h"
// END TODO

// just for testing
#include "Actor.h"
#include "ActorAnimProcess.h"

#include <SDL.h>
#include <cstdlib>

#include "DisasmProcess.h"
#include "CompileProcess.h"

using std::string;

bool showconsole=true; // yes, yes, more of them :-)

Application* Application::application = 0;

Application::Application(const int argc, const char * const * const argv)
	: kernel(0), ucmachine(0), filesystem(0), config(0), desktop(0),
	  console(0), screen(0), palettemanager(0), gamedata(0), world(0),
	  display_list(0),
	  runMinimalSysInit(false), runGraphicSysInit(false), runSDLInit(false),
	  weAreDisasming(false), weAreCompiling(false),
	  isRunning(false), framenum(0),
	  frameSkip(false), frameLimit(true), interpolate(true), animationRate(100),
  	  fastArea(0), avatarInStasis(false), camera(0)
{
	assert(application == 0);
	application = this;

	ParseArgs(argc, argv);

	if(weAreDisasming==true)
	{
		pout << "We Are Disassembling..." << std::endl;
		MinimalSysInit();
		kernel->addProcess(new DisasmProcess());
	}
	else if(weAreCompiling==true)
	{
		pout << "We Are Compiling..." << std::endl;
		MinimalSysInit();
#ifndef WIN32
		kernel->addProcess(new CompileProcess(filesystem));
#else
		pout << "Then again, maybe not" << std::endl;
#endif
	}
	else
	{
		MinimalSysInit();

		GraphicSysInit();

		U8Playground();
	}
}

Application::~Application()
{
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(config);
	FORGET_OBJECT(palettemanager);

	application = 0;
}

void Application::run()
{
	isRunning = true;

	sint32 next_ticks = SDL_GetTicks();	// Next time is right now!
	
	// Ok, the theory is that if this is set to true then we must do a repaint
	// At the moment only it's ignored
	bool repaint;

	SDL_Event event;
	while (isRunning) {
		// this needs some major changes, including possibly:
		// - handling events in-between processes?
		//   (so have a kernel->runProcess() that runs a single process)
		// - smarter painting
		//    - dirty rectangles?
		//    - more/less often depending on speed
		//    ...
		// ...

		inBetweenFrame = true;	// Will get set false if it's not an inBetweenFrame

		if (!frameLimit) {
			repaint = false;
			
			if (kernel->runProcesses(framenum++)) repaint = true;
			inBetweenFrame = false;
			next_ticks = animationRate + SDL_GetTicks();
			lerpFactor = 0;
		}
		else 
		{
			sint32 ticks = SDL_GetTicks();
			sint32 diff = next_ticks - ticks;
			repaint = false;

			while (diff < 0) {
				next_ticks += animationRate;
				if (kernel->runProcesses(framenum++)) repaint = true;
	
				inBetweenFrame = false;

				ticks = SDL_GetTicks();

				// If frame skipping is off, we will only recalc next ticks IF the frames are
				// taking up 'way' too much time. 
				if (!frameSkip && diff <= -animationRate*2) next_ticks = animationRate + ticks;

				diff = next_ticks - ticks;
				if (!frameSkip) break;
			}

			// Calcualte the lerp_factor
			lerpFactor = ((animationRate-diff)*256)/animationRate;
			//pout << "lerpFactor: " << lerpFactor << " framenum: " << framenum << std::endl;
			if (lerpFactor > 256) lerpFactor = 256;
		}

		repaint = true;

		// get & handle all events in queue
		while (isRunning && SDL_PollEvent(&event)) {
			handleEvent(event);
		}

		// Paint Screen
		if(runGraphicSysInit) // need to worry if the graphics system has been started. Need nicer way.
			paint();
	}
}

void Application::U8Playground()
{
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

	gamedata = new GameData();
	gamedata->loadU8Data();

	// Create ItemSorter *** TODO MOVE THIS TO GameMapGump
	pout << "Create display_list ItemSorter object" << std::endl;
	display_list = new ItemSorter();

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
	world->initNPCs();
	world->loadItemCachNPCData(icd, npcd);
	delete icd;
	delete npcd;
	delete u8save;
	delete saveds;

	Actor* av = world->getNPC(1);
	//av->teleport(40, 16240, 15240, 64);
	if (av)
		world->switchMap(av->getMapNum());
	else
		world->switchMap(3);

	// some testing...
	//world->switchMap(3);

	//world->switchMap(43);
  	//world->switchMap(3);

	// Create console gump
	//pout << "Create Graphics Console" << std::endl;
	//desktop = console = new ConsoleGump(0,0, width, height);

	// Clear Screen
	pout << "Paint Inital display" << std::endl;
	inBetweenFrame = 0;
	lerpFactor = 256;
	SetCameraProcess(new CameraProcess(1)); // Follow Avatar
	paint();
}

uint16 Application::SetCameraProcess(CameraProcess *cam)
{
	if (!cam) cam = new CameraProcess();
	if (camera) camera->terminate();
	camera = cam;
	return kernel->addProcess(camera);
}

void Application::GetCamera(sint32 &x, sint32 &y, sint32 &z)
{
	if (!camera) {

		CurrentMap *map = world->getCurrentMap();
		int map_num = map->getNum();
		Actor* av = world->getNPC(1);
		
		if (!av || av->getMapNum() != map_num)
		{
			x = 8192;
			y = 8192;
			z = 64;
		}
		else
			av->getLocation(x,y,z);
	}
	else
	{
		camera->GetLerped(x, y, z, 256);
	}
}

void Application::SetupDisplayList()
{
	Rect dims;
	screen->GetSurfaceDims(dims);
	sint32 resx = dims.w, resy = dims.h;
	/*
	at 640x480, with u8 sizes
	sint32 sx_limit = 4;
	sint32 sy_limit = 8;
	sint32 xy_limit = 6;

	at 320x240, with u8 sizes
	sint32 sx_limit = 3;
	sint32 sy_limit = 7;
	sint32 xy_limit = 4;

	for tgwds use half the resolution
	*/


	//if (tgwds)
	//{
	//	resx/=2;
	//	resy/=2;
	//}

	// By default the fastArea is the screensize plus a border of no more
	// than 256 pixels wide and 384 pixels high

	sint32 sx_limit = resx/256 + 2;
	sint32 sy_limit = resy/128 + 6;
	sint32 xy_limit = (sy_limit+sx_limit)/2;
	CurrentMap *map = world->getCurrentMap();

	std::vector<uint16> *prev_fast = &fastAreas[1-fastArea];
	std::vector<uint16> *fast = &fastAreas[fastArea];
	if (!inBetweenFrame) 
	{
		fast->erase(fast->begin(), fast->end());
	}

	// Get the initial camera location
	int lx, ly, lz;
	if (!camera) {

		int map_num = map->getNum();
		Actor* av = world->getNPC(1);
		
		if (!av || av->getMapNum() != map_num)
		{
			lx = 8192;
			ly = 8192;
			lz = 64;
		}
		else
			av->getLocation(lx,ly,lz);
	}
	else
	{
		camera->GetLerped(lx, ly, lz, lerpFactor);
	}

	sint32 gx = lx/512;
	sint32 gy = ly/512;

	// Get all the required items and sort
	for (int y = -xy_limit; y <= xy_limit; y++)
	{
		for (int x = -xy_limit; x <= xy_limit; x++)
		{
			sint32 sx = x - y;
			sint32 sy = x + y;

			if (sx < -sx_limit || sx > sx_limit || sy < -sy_limit || sy > sy_limit)
				continue;

			const std::list<Item*>* items = map->getItemList(gx+x,gy+y);

			if (!items) continue;

			std::list<Item*>::const_iterator it = items->begin();
			std::list<Item*>::const_iterator end = items->end();
			for (; it != end; ++it)
			{
				Item *item = *it;
				if (!item) continue;

				if (!inBetweenFrame) 
				{
					item->inFastArea(fastArea);
					fast->push_back(item->getObjId());
				}
				item->doLerp(lx,ly,lz,lerpFactor);
				display_list->AddItem(item);
			}
		}
	}

	// Now handle leaving the fast area
	if (!inBetweenFrame) 
	{
		std::vector<uint16>::iterator it  = prev_fast->begin();
		std::vector<uint16>::iterator end  = prev_fast->end();

		for (;it != end; ++it)
		{
			Object *obj = world->getObject(*it);

			// No object, continue
			if (!obj) continue;

			Item *item = p_dynamic_cast<Item*>(obj);

			// Not an item, continue
			if (!item) continue;

			// If the fast area for the item is the current one, continue
			if (item->getExtFlags() & (Item::EXT_FAST0<<fastArea)) continue;

			// Ok, we must leave te Fast area
			item->leavingFastArea();
		}

		// Flip the fast area
		fastArea=1-fastArea;
	}
}

// Paint the screen
void Application::paint()
{
	// Begin painting
	screen->BeginPainting();

	// We need to get the dims
	Rect dims;
	screen->GetSurfaceDims(dims);

	long before_fill = SDL_GetTicks();
	screen->Fill32(0x3F3F3F, dims.x, dims.y, dims.w, dims.h);
	long after_fill = SDL_GetTicks();

	// Set the origin to the center of the screen
	screen->SetOrigin(dims.w/2, dims.h/2);

	// Do display list
	display_list->BeginDisplayList(screen, palettemanager->getPalette(PaletteManager::Pal_Game));
	long before_sort = SDL_GetTicks();
	SetupDisplayList();
	long after_sort = SDL_GetTicks();
	display_list->PaintDisplayList();
	long after_paint = SDL_GetTicks();

	// Put the origin to 0,0 for drawing console
	screen->SetOrigin(0,0);

	if (showconsole)
		con.DrawConsole(screen, 0, 0, dims.w, dims.h);

	static long prev = 0;
	long now = SDL_GetTicks();
	long diff = now - prev;
	prev = now;

	char buf[256];
	snprintf(buf, 255, "Rendering time %li ms %li FPS - Sort %li ms  Paint %li ms  Fill %li ms", diff, 1000/diff, after_sort-before_sort, after_paint-after_sort, after_fill-before_fill);
	screen->PrintTextFixed(con.GetConFont(), buf, 8, dims.h-16);

	// End painting
	screen->EndPainting();
}

void Application::setupVirtualPaths()
{
	// setup the 'base' virtual paths:
	// @home - $HOME/.pentagram/ - for config files, saves,... (OS dependant)
	// @data - /usr/share/pentagram/ - for config files, data,.. (OS dependant)
	//       NB: @data can be overwritten by config files
	//       this should be a default set by configure (or other build systems)

	std::string home;
#ifdef HAVE_HOME
	home = getenv("HOME");
	home += "/.pentagram";
#else
	// TODO: what to do on systems without $HOME?
	home = ".";
#endif
	filesystem->AddVirtualPath("@home", home);

	std::string data;
#ifdef DATA_PATH
	data = DATA_PATH;
#else
	data = "data";
#endif
	pout << "Trying built-in data path" << std::endl;
	filesystem->AddVirtualPath("@data", data);
}

// load configuration files
void Application::loadConfig()
{
	pout << "Loading configuration files:" << std::endl;

	// system-wide config
	pout << "@data/pentagram.cfg... ";
	if (config->readConfigFile("@data/pentagram.cfg", "config"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;

	// user config
	pout << "@home/pentagram.cfg... ";
	if (config->readConfigFile("@home/pentagram.cfg", "config"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;

	pout << "Game: " << game << std::endl;

	// Question: config files can specify an alternate data path
	// Do we reload the config files if that path differs from the
	// hardcoded data path? (since the system-wide config file is in @data)

	/**********
	  load pentagram specific data path
	 **********/
	std::string data;
	pout << "Reading \"config/general/data\" config key." << std::endl;
	config->value("config/general/data", data, "");
	if (data != "") {
		pout << "Data Path: " << data << std::endl;
		filesystem->AddVirtualPath("@data", data);
	}
	else {
		pout << "Key not found. Data path set to default." << std::endl;
	}

	/**********
	  load main game data path
	 **********/
	std::string u8;
	pout << "Reading \"config/" << game << "/path\" config key." << std::endl;
	config->value(string("config/")+game+"/path", u8, ".");
	filesystem->AddVirtualPath("@u8", u8);
	pout << "U8 Path: " << u8 << std::endl;

	/**********
	  load work path. Default is $(HOME)/.pentagram/game-work
	  where 'game' in the above is the specified 'game' loaded (default 'u8')
	 **********/
	std::string home;
#ifdef HAVE_HOME
	home = getenv("HOME");
	home += "/.pentagram";
#else
	// TODO: what to do on systems without $HOME?
	home = ".";
#endif
	std::string work(home+"/"+game+"-work");
	pout << "Reading \"config/" << game << "/work\" config key." << std::endl;
	config->value(string("config/")+game+"/work", work, string(home+"/"+game+"-work").c_str());
	filesystem->AddVirtualPath("@work", work, true); // force creation if it doesn't exist
	pout << "U8 Workdir: " << work << std::endl;
}

void Application::ParseArgs(const int argc, const char * const * const argv)
{
	pout << "Parsing Args" << std::endl;

	Args parameters;

	parameters.declare("--disasm",  &weAreDisasming, true);
	parameters.declare("--compile", &weAreCompiling, true);
	parameters.declare("--game",    &game,           "u8");
	//parameters.declare("--singlefile",	&singlefile, true);

	parameters.process(argc, argv);
}

void Application::MinimalSysInit()
{
	// if we've already done this...
	if(runMinimalSysInit) return;
	//else...

	SDLInit();

	// Create the kernel
	pout << "Create Kernel" << std::endl;
	kernel = new Kernel;

	pout << "Create FileSystem" << std::endl;
	filesystem = new FileSystem;
	setupVirtualPaths(); // setup @home, @data

	pout << "Create Configuration" << std::endl;
	config = new Configuration;
	loadConfig();

	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine;

	runMinimalSysInit=true;
}

void Application::GraphicSysInit()
{
	// if we've already done this...
	if(runGraphicSysInit) return;
	//else...

	// check we've run the prereqs...
	if(!runMinimalSysInit)
		MinimalSysInit();
	// go!

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
	pout << "Resize Console" << std::endl;
	con.CheckResize(width);

	LoadConsoleFont();

	runGraphicSysInit=true;
}

void Application::LoadConsoleFont()
{
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

// Init sdl
void Application::SDLInit()
{
	// if we're already done this...
	if(runSDLInit) return;
	//else...

	pout << "Init SDL" << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	runSDLInit=true;
}


void Application::handleEvent(const SDL_Event& event)
{
	uint32 eventtime = SDL_GetTicks();

	extern int userchoice; // major hack... see Item::I_ask

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

	}
	break;

	case SDL_MOUSEBUTTONUP:
	{
		// Ok, a bit of a hack for now
		if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)
		{
			if (avatarInStasis) {
				pout << "Can't: avatarInStatus" << std::endl; 
				break;
			}

			pout << std::endl << "Tracing mouse click: ";

			Rect dims;
			screen->GetSurfaceDims(dims);

			// We will assume the display_list has all the items in it
			uint16 objID = display_list->Trace(event.button.x - dims.w/2,
							event.button.y - dims.h/2);

			if (!objID)
				pout << "Didn't find an item" << std::endl;
			else
			{
				extern uint16 targetObject; // major hack number 2
				targetObject = objID;

				World *world = World::get_instance();
				Item *item = p_dynamic_cast<Item*>(world->getObject(objID));

				pout << "Found item " << objID << " (shape " << item->getShape() << ", " << item->getFrame() << ", q:" << item->getQuality() << ")" << std::endl;
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (item) item->callUsecodeEvent(0);	// CONSTANT
				} else {
					if (item) item->callUsecodeEvent(1);	// CONSTANT
				}
			}
		}

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
				avatar->move(x-512,y-512,z);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			}
		} break;
		case SDLK_DOWN: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x+512,y+512,z);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			}
		} break;
		case SDLK_LEFT: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x-512,y+512,z);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			}
		} break;
		case SDLK_RIGHT: {
			if (!avatarInStasis) { 
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				avatar->getLocation(x,y,z);
				avatar->move(x+512,y-512,z);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			}
		} break;
		case SDLK_BACKQUOTE: showconsole = !showconsole; break;
		case SDLK_ESCAPE: case SDLK_q: isRunning = false; break;
		case SDLK_PAGEUP: if (showconsole) con.ScrollConsole(-3); break;
		case SDLK_PAGEDOWN: if (showconsole) con.ScrollConsole(3); break;
		case SDLK_LEFTBRACKET: display_list->DecSortLimit(); break;
		case SDLK_RIGHTBRACKET: display_list->IncSortLimit(); break;
		case SDLK_t: { // quick animation test

			if (!avatarInStasis) { 
                Actor* devon = World::get_instance()->getNPC(2);
				Process* p = new ActorAnimProcess(devon, 0, 2);
				Kernel::get_instance()->addProcess(p);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			} 
		} break;
		case SDLK_f: { // trigger 'first' egg
//			lx = 14527 + 3*512;
//			ly = 5887 + 3*512;
//			lz = 8;

			if (!avatarInStasis) {
				Item* item = p_dynamic_cast<Item*>(World::get_instance()->getObject(21183)); // *cough*
				if (item->getQuality() != 36)
					item = p_dynamic_cast<Item*>(World::get_instance()->getObject(21184)); // *cough*
				sint32 ix, iy, iz;
				item->getLocation(ix,iy,iz);
				SetCameraProcess(new CameraProcess(ix,iy,iz)); // Center on egg
				item->callUsecodeEvent(7);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
			} 
		} break;
		case SDLK_g: { // trigger 'execution' egg
			if (!avatarInStasis) {
				Item* item = p_dynamic_cast<Item*>(World::get_instance()->getObject(21162)); // *cough*
				if (item->getQuality() != 4)
					item = p_dynamic_cast<Item*>(World::get_instance()->getObject(21163)); // *cough*
				Actor* avatar = World::get_instance()->getNPC(1);
				sint32 x,y,z;
				item->getLocation(x,y,z);
				avatar->move(x,y,z);
				item->callUsecodeEvent(7);
			} else { 
				pout << "Can't: avatarInStatus" << std::endl; 
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



uint32 Application::I_getCurrentTimerTick(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	return get_instance()->getFrameNum()*8;
}

uint32 Application::I_setAvatarInStasis(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(statis);
	get_instance()->setAvatarInStasis(statis!=0);
	return 0;
}

