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
#include "ShapeFlex.h"
#include "Palette.h"

#include <SDL.h>

int classid, offset; // only temporary, don't worry :-)
ShapeFlex* shapes;

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
	IDataSource *pf = filesystem->ReadFile("u8pal.pal");
	if (!pf) {
		perr << "Unable to load u8pal.pal. Exiting" << std::endl;
		std::exit(-1);
	}
	pf->seek(4); // seek past header
	palettemanager->load(PaletteManager::Pal_Game, *pf);

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("u8shapes.flx");
	if (!sf) {
		perr << "Unable to load u8shapes.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	shapes = new ShapeFlex(sf);

	// Load confont
	pout << "Load Confont" << std::endl;
	IDataSource *cf = filesystem->ReadFile("fixedfont.tga");
	if (cf) confont = Texture::Create(*cf, "fixedfont.tga");
	else confont = 0;
	if (!confont)
	{
		perr << "Unable to load fixedfont.tga. Exiting" << std::endl;
		std::exit(-1);
	}
	delete cf;
	con.SetConFont(confont);

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
	IDataSource* ds = filesystem->ReadFile("eusecode.flx");
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
	screen->EndPainting();
}


// load configuration files
void Application::loadConfig()
{
	pout << "Loading configuration files:" << std::endl;

	// TODO:
	// load system-wide config...
	// load user-specific config...

	// for now:
	pout << "pentagram.cfg... ";
	if (config->readConfigFile("pentagram.cfg", "config"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;


	pout << "test.cfg... ";
	if (config->readConfigFile("test.cfg", "config"))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;
}
