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

#include <vector>

#include "SDL_events.h"
#include "intrinsics.h"

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
class ItemSorter;	// TODO MOVE THIS TO GameMapGump
class CameraProcess;

// extremely simplified stub-ish Application class
class Application {
public:
	Application(const int argc, const char * const * const argv);
	~Application();

	static Application* get_instance() { return application; }

	void run();
	void paint();
	void handleEvent(const SDL_Event& event);

	void loadConfig();
	void setupVirtualPaths();

	void U8Playground();

	void ForceQuit() { isRunning=false; };

	uint32 getFrameNum() const { return framenum; }

	INTRINSIC(I_getCurrentTimerTick);
	INTRINSIC(I_setAvatarInStasis);
	INTRINSIC(I_getAvatarInStasis);

	// To be moved 'somewhere' else
	void GetCamera(sint32 &x, sint32 &y, sint32 &z);
	uint16 SetCameraProcess(CameraProcess *);	// Set the current camera process. Adds process. Return PID
	void setAvatarInStasis(bool stat) { avatarInStasis = stat; }

private:
	static Application* application;

	// minimal system
	Kernel* kernel;
	UCMachine* ucmachine;
	FileSystem* filesystem;
	Configuration* config;
	// full system
	ResizableGump* desktop;
	ConsoleGump* console;
	RenderSurface *screen;
	PaletteManager *palettemanager;
	GameData *gamedata;
	World *world;
	ItemSorter *display_list;	// TODO MOVE THIS TO GameMapGump

	void ParseArgs(const int argc, const char * const * const argv);

	// internal initilisation functions,
	// called depending upon command line arguments
	void MinimalSysInit(); // console only system
	void GraphicSysInit(); // starts the graphics subsystem
	void LoadConsoleFont(); // loads the console font
	void SDLInit(); // start sdl

	void SetupDisplayList();	// TODO MOVE THIS TO GameMapGump

	// Various dependancy flags
	bool runMinimalSysInit;
	bool runGraphicSysInit;
	bool runSDLInit;

	std::string game;
	// various temporary state flags, to be moved somewhere more appropriate in time.
	bool weAreDisasming;
	bool weAreCompiling;
	bool weAreWeAreWeAreTheMany; // false for the time being. *grin*

	bool isRunning;

	// Timing stuff
	uint32 framenum;
	sint32 lerpFactor;			// Interpolation factor for this frame (0-256)
	bool inBetweenFrame;		// Set true if we are doing an inbetween frame

	bool frameSkip;				// Set to true to enable frame skipping (default false)
	bool frameLimit;			// Set to true to enable frame limiting (default true)
	bool interpolate;			// Set to true to enable interpolation (default true)
	sint32 animationRate;		// The animation rate in ms. Affects all processes! (default 100)

	// fastArea stuff. Move somewhere else
	std::vector<uint16>			fastAreas[2];
	int							fastArea;	// 0 or 1

	// Sort of Camera Related Stuff, move somewhere else

	bool avatarInStasis;	// If this is set to true, Avatar can't move, 
							// nor can Avatar start more usecode

	CameraProcess	 *camera;
};

#endif
