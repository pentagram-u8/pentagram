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

#include "Map.h" // temp
#include "U8Save.h"

#include "Item.h"
#include "ItemSorter.h" // TODO MOVE THIS TO GameMapGump
#include "CurrentMap.h"

#include <SDL.h>
#include <cstdlib>


Application* Application::application = 0;

int classid, offset; // only temporary, don't worry :-)

Application::Application(int argc, char *argv[])
	: kernel(0), ucmachine(0), filesystem(0), config(0), desktop(0),
	  console(0), screen(0), palettemanager(0), gamedata(0), world(0),
	  display_list(0),
	  runMinimalSysInit(false), runGraphicSysInit(false), runSDLInit(false),
	  isRunning(false)
{
	assert(application == 0);
	application = this;

	MinimalSysInit();

	UCMachineInit();

	GraphicSysInit();

	U8Playground(argc, argv);
}

Application::~Application()
{
	delete kernel;
	delete ucmachine;
	delete filesystem;
	delete config;
	delete palettemanager;

	application = 0;
}

void Application::run()
{
	UCProcess* p;
	Usecode* u = GameData::get_instance()->getMainUsecode();
	if (classid != -1) {
		p = new UCProcess(u, classid, offset);
	} else {
//		p = new UCProcess(u, 0xD0, 0x80);
		p = new UCProcess(u, 0x581, 0x28F9);
	}

    ucmachine->addProcess(p);


	isRunning = true;

	uint32 framenum = 0;
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

		kernel->runProcesses(framenum++);

		// get & handle all events in queue
		while (isRunning && SDL_PollEvent(&event)) {
			handleEvent(event);
		}

		// Paint Screen
		paint();
	}
}

void Application::U8Playground(int argc, char *argv[])
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

	IDataSource *fd = filesystem->ReadFile("@u8/static/fixed.dat");
	if (!fd) {
		perr << "Unable to load static/fixed.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	world->loadFixed(fd);
	// some testing...
	world->switchMap(3);
	//world->switchMap(40);
	//world->switchMap(43);
  	//world->switchMap(3);

	// Create console gump
	//pout << "Create Graphics Console" << std::endl;
	//desktop = console = new ConsoleGump(0,0, width, height);

	// Clear Screen
	pout << "Paint Inital display" << std::endl;
	paint();

	classid = offset = -1;
	if (argc == 3) {
		classid = std::strtol(argv[1], 0, 16);
		offset = std::strtol(argv[2], 0, 16);
	}
}

// Paint the screen
void Application::paint()
{
	screen->BeginPainting();
	screen->Fill32(0x3F3F3F, 0, 0, 640, 480);

	display_list->BeginDisplayList(screen, palettemanager->getPalette(PaletteManager::Pal_Game));
	long before_sort = SDL_GetTicks();
	world->SetupDisplayList(display_list);
	long after_sort = SDL_GetTicks();
	display_list->PaintDisplayList();
	long after_paint = SDL_GetTicks();

 	con.DrawConsole(screen, 0, 0, 640, 480);
	screen->EndPainting();

	static long prev = 0;
	long now = SDL_GetTicks();
	long diff = now - prev;
	prev = now;

	con.Printf("Rendering time %i ms %i FPS - Sort %i ms  Paint %i ms       \r", diff, 1000/diff, after_sort-before_sort, after_paint-after_sort);
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
	filesystem->AddVirtualPath("@data", data);
}

// load configuration files
void Application::loadConfig()
{
	pout << "Loading configuration files:" << std::endl;

	// TODO:
	// load system-wide config...
	// load user-specific config...

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


	// Question: config files can specify an alternate data path
	// Do we reload the config files if that path differs from the
	// hardcoded data path? (since the system-wide config file is in @data)

	std::string data;
	pout << "Reading \"config/paths/data\" config key." << std::endl;
	config->value("config/paths/data", data, "");
	if (data != "") {
		pout << "Data Path: " << data << std::endl;
		filesystem->AddVirtualPath("@data", data);
	}
	else {
		pout << "Key not found. Data path set to default." << std::endl;
	}

	std::string u8;
	pout << "Reading \"config/paths/u8\" config key." << std::endl;
	config->value("config/paths/u8", u8, ".");
	filesystem->AddVirtualPath("@u8", u8);
	pout << "U8 Path: " << u8 << std::endl;
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
	screen = RenderSurface::SetVideoMode(640, 480, 32, false, false);
	if (!screen)
	{
		perr << "Unable to set video mode. Exiting" << std::endl;
		std::exit(-1);
	}
	pout << "Resize Console" << std::endl;
	con.CheckResize(640);

	// Load confont
	pout << "Load Confont" << std::endl;
	IDataSource *cf = filesystem->ReadFile("@data/fixedfont.tga");
	Texture *confont;
	if (cf) confont = Texture::Create(*cf, "@data/fixedfont.tga");
	else confont = 0;
	if (!confont)
	{
		perr << "Unable to load fixedfont.tga. Exiting" << std::endl;
		std::exit(-1);
	}
	delete cf;
	con.SetConFont(confont);

	runGraphicSysInit=true;
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


void Application::UCMachineInit()
{
	// if we've already done this...
	if(ucmachine!=0) return;
	//else...

	// check we've run the prereqs...
	if(!runMinimalSysInit)
		MinimalSysInit();
	// go!

	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine;
}


void Application::handleEvent(const SDL_Event& event)
{
	uint32 eventtime = SDL_GetTicks();

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
		isRunning = false;		
	}
	break;

	// any more useful events?
	
	default:
		break;
	}
}
