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
#include "Shape.h"
#include "PaletteManager.h"
#include "MainShapeFlex.h"
#include "Palette.h"
#include "XFormBlend.h"
#include "World.h"

#include "Map.h" // temp

#include <SDL.h>
#include <cstdlib>

int classid, offset; // only temporary, don't worry :-)
Shape* shape;

Application::Application(int argc, char *argv[])
{
	// Init sdl
	pout << "Init SDL" << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	// Create the kernel
	pout << "Create Kernel" << std::endl;
	kernel = new Kernel;
	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine;
	pout << "Create FileSystem" << std::endl;
	filesystem = new FileSystem;
	setupVirtualPaths(); // setup @home, @data

	pout << "Create Configuration" << std::endl;
	config = new Configuration;
	loadConfig();

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

	// Load palette
	pout << "Load Palette" << std::endl;
	palettemanager = new PaletteManager(screen);
	IDataSource *pf = filesystem->ReadFile("@u8/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load u8pal.pal. Exiting" << std::endl;
		std::exit(-1);
	}
	pf->seek(4); // seek past header
	palettemanager->load(PaletteManager::Pal_Game, *pf, U8XFormFuncs);

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("@u8/static/u8shapes.flx");
	if (!sf) {
		perr << "Unable to load u8shapes.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes = new MainShapeFlex(sf);

	// Load typeflags
	IDataSource *tfs = filesystem->ReadFile("@u8/static/typeflag.dat");
	if (!tfs) {
		perr << "Unable to load typeflag.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadTypeFlags(tfs);
	delete tfs;

	shape = mainshapes->getShape(1);
	shape->setPalette(palettemanager->getPalette(PaletteManager::Pal_Game));

	// Initialize world
	pout << "Initialize World" << std::endl;
	world = new World();

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

	IDataSource *fd = filesystem->ReadFile("@u8/static/fixed.dat");
	Flex *ffd = new Flex(fd);
	uint32 mapsize = ffd->get_size(3);
	uint8* mapdata = ffd->get_object(3);
	IBufferDataSource *mds = new IBufferDataSource(mapdata, mapsize);
	Map* m = new Map();
	std::list<Item*> itemlist;
	pout << "Loading fixed from map 3" << std::endl;
	pout << "----------------------------------------------" << std::endl;
	m->loadFixedFormatObjects(itemlist, mds);
	IDataSource *nfd = filesystem->ReadFile("@u8/gamedat/nonfixed.dat");
	Flex *fnfd = new Flex(nfd);
	mapsize = fnfd->get_size(3);
	mapdata = fnfd->get_object(3);
	pout << "Loading nonfixed from map 3" << std::endl;
	pout << "----------------------------------------------" << std::endl;
	IBufferDataSource *nmds = new IBufferDataSource(mapdata, mapsize);
	m->loadFixedFormatObjects(itemlist, nmds);

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

Application::~Application()
{
	delete kernel;
	delete ucmachine;
	delete filesystem;
	delete config;
	delete palettemanager;
}

void Application::run()
{
	IDataSource* ds = filesystem->ReadFile("@u8/usecode/eusecode.flx");
	if (!ds) {
		perr << "Unable to load eusecode.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	Usecode* u = new UsecodeFlex(ds);
	UCProcess* p;
	if (classid != -1) {
		p = new UCProcess(u, classid, offset);
	} else {
//		p = new UCProcess(u, 0xD0, 0x80);
		p = new UCProcess(u, 0x581, 0x28F9);
	}

    ucmachine->addProcess(p);
	uint32 framenum = 0;
	SDL_Event event;
	while (1) {
		kernel->runProcesses(framenum++);

		pout << "Pausing execution. Press a key (in graphics window) to continue" << std::endl;
		
		// Paint Screen
		paint();

		while (1)
		{
			if (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN) break;
			}
		}
	}
}

// Paint the screen
void Application::paint()
{
	screen->BeginPainting();
	screen->Fill32(0x3F3F3F, 0, 0, 640, 480);
 	con.DrawConsole(screen, 0, 0, 640, 480);
	screen->Paint(shape,2,320,240);
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
	config->value("config/paths/data", data, "");
	if (data != "") {
		filesystem->AddVirtualPath("@data", data);
	}

	std::string u8;
	config->value("config/paths/u8", u8, ".");
	filesystem->AddVirtualPath("@u8", u8);
}
