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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "SDL_events.h"

class Kernel;
class UCMachine;
class FileSystem;
class Configuration;
class ResizableGump;
class ConsoleGump;
class RenderSurface;
class PaletteManager;
class GameData;
class World;

// extremely simplified stub-ish Application class
class Application {
public:
	Application(int argc, char *argv[]);
	~Application();

	static Application* get_instance() { return application; }
	
	void run();
	void paint();
	void handleEvent(const SDL_Event& event);

	void loadConfig();
	void setupVirtualPaths();

	void U8Playground(int argc, char *argv[]);

private:
	Kernel* kernel;
	UCMachine* ucmachine;
	FileSystem* filesystem;
	Configuration* config;
	ResizableGump* desktop;
	ConsoleGump* console;
	RenderSurface *screen;
	PaletteManager *palettemanager;
	GameData *gamedata;
	World *world;

	static Application* application;

	// internal initilisation functions,
	// called depending upon command line arguments
	void MinimalSysInit(); // console only system
	void GraphicSysInit(); // starts the graphics subsystem
	void SDLInit(); // start sdl
	void UCMachineInit(); // start usecode machine

	// Various dependancy flags
	bool runMinimalSysInit;
	bool runGraphicSysInit;
	bool runSDLInit;

	bool isRunning;
};

#endif
