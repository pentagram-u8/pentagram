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

#ifndef GUIAPP_H
#define GUIAPP_H

#include <vector>

#include "SDL_events.h"
#include "intrinsics.h"
#include "Args.h"

#include "CoreApp.h"

class Gump;
class ConsoleGump;
class GameMapGump;
class RenderSurface;
class PaletteManager;
class GameData;
class World;

// extremely simplified stub-ish Application class
class GUIApp : public CoreApp
{
	public:
		ENABLE_RUNTIME_CLASSTYPE();

		GUIApp(const int argc, const char * const * const argv);
		virtual ~GUIApp();

		virtual void run();
		virtual void handleEvent(const SDL_Event& event);

		//virtual void loadConfig();
		//virtual void setupVirtualPaths();

		void U8Playground();
		void paint();
		bool isPainting() { return painting; }


		INTRINSIC(I_getCurrentTimerTick);
		INTRINSIC(I_setAvatarInStasis);
		INTRINSIC(I_getAvatarInStasis);

		void setAvatarInStasis(bool stat) { avatarInStasis = stat; }

		GameMapGump *getGameMapMapGump() { return gameMapGump; }
		Gump *getDestkopGump() { return desktopGump; }

	private:

		// full system
		RenderSurface *screen;
		PaletteManager *palettemanager;
		GameData *gamedata;
		World *world;

		Gump* desktopGump;
		ConsoleGump *consoleGump;
		GameMapGump *gameMapGump;

		// called depending upon command line arguments
		void GraphicSysInit(); // starts the graphics subsystem
		void LoadConsoleFont(); // loads the console font

		// Various dependancy flags
		bool runGraphicSysInit;
		bool runSDLInit;

		// Timing stuff
		sint32 lerpFactor;			// Interpolation factor for this frame (0-256)
		bool inBetweenFrame;		// Set true if we are doing an inbetween frame

		bool frameSkip;				// Set to true to enable frame skipping (default false)
		bool frameLimit;			// Set to true to enable frame limiting (default true)
		bool interpolate;			// Set to true to enable interpolation (default true)
		sint32 animationRate;		// The animation rate in ms. Affects all processes! (default 100)

		// Sort of Camera Related Stuff, move somewhere else

		bool avatarInStasis;	// If this is set to true, Avatar can't move, 
								// nor can Avatar start more usecode

		bool painting;			// Set true when painting
};

inline GUIApp *getGUIInstance() { return static_cast<GUIApp *>(CoreApp::get_instance()); };


#endif
