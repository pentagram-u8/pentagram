/*
 *	CoreApp.h - Base application class that contains the minimal functality to
 *	            support an instance of the pentagram engine
 *
 *  Copyright (C) 2002-2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef COREAPP_H
#define COREAPP_H

#include "SDL_events.h"
#include "intrinsics.h"
#include "Args.h"

class Kernel;
class UCMachine;
class FileSystem;
class Configuration;

class CoreApp
{
	public:
		ENABLE_RUNTIME_CLASSTYPE();
		CoreApp(const int argc, const char * const * const argv,
			const std::string _defaultGame, bool delayPostInit=false);
		virtual ~CoreApp();
		
		// if delayPostInit is true, it deferrs to the child class to do the post
		// construction postInit stuff.
		// it's basically so the child classes can setup their own config handling stuff,
		// and register arguments, etc, before we go to all the effort of parsing things.
		void postInit(const int argc, const char * const * const argv);
		
		static CoreApp* get_instance() { return application; };

		virtual void run()=0; // FIXME: Need stub
		virtual void paint()=0; // probably shouldn't exist
		virtual bool isPainting() { return false; }
		virtual void handleEvent(const SDL_Event&event)=0; // FIXME: need stub

		virtual void ForceQuit() { isRunning=false; };
	
		uint32 getFrameNum() const { return framenum; };

		virtual void helpMe();

		bool help()   const { return oHelp;   };
		bool quiet()  const { return oQuiet;  };
		bool vquiet() const { return oVQuiet; };
		
	//protected: // should be, but I think the nesting is too deep for this to work right...

		bool isRunning;

		uint32 framenum;
	
		std::string game;

		// minimal system
		Kernel* kernel;
		FileSystem* filesystem;
		Configuration* config;

		Args parameters;
	
		static CoreApp* application; // derived classes need to override this
		
	private:
		std::string defaultGame;
		
		void SDLInit(); // start sdl
		virtual void sysInit();
		
		virtual void ParseArgs(const int argc, const char * const * const argv);
		virtual void setupVirtualPaths();
		virtual void loadConfig();

		bool oHelp;
		bool oQuiet;
		bool oVQuiet;
};

inline CoreApp *getCoreInstance() { return CoreApp::get_instance(); };

#endif

