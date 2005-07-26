/*
 *	CoreApp.h - Base application class that contains the minimal functionality
 *              to support pentagram tools
 *
 *  Copyright (C) 2002-2005  The Pentagram Team
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

#ifndef COREAPP_H
#define COREAPP_H

#include "SDL_events.h"
#include "intrinsics.h"
#include "Args.h"

class FileSystem;
class ConfigFileManager;
class SettingManager;
struct GameInfo;

class CoreApp
{
public:
	ENABLE_RUNTIME_CLASSTYPE();
	CoreApp(int argc, const char* const* argv);
	virtual ~CoreApp();
	
	static CoreApp* get_instance() { return application; };

	virtual void run()=0; // FIXME: Need stub
	virtual void paint()=0; // probably shouldn't exist
	virtual bool isPainting() { return false; }
	virtual void handleEvent(const SDL_Event&event)=0; // FIXME: need stub
	
	virtual void ForceQuit() { isRunning=false; };

	//! Startup the application. This will prepare the application for run().
	//! Should call parent class' startup().
	void startup();

	//! Get current GameInfo struct
	GameInfo* getGameInfo() const { return gameinfo; }
	
	virtual void helpMe();
	
	bool help()   const { return oHelp;   };
	bool quiet()  const { return oQuiet;  };
	bool vquiet() const { return oVQuiet; };
	
protected:

	//! Declare commandline arguments.
	//! Should call parent class' DeclareArgs().
	virtual void DeclareArgs();
	
	//! Fill a GameInfo struct for the give game name
	//! \param game The id of the game to check (from pentagram.cfg)
	//! \param gameinfo The GameInfo struct to fill
	//! \return true if detected all the fields, false if detection failed
	bool getGameInfo(Pentagram::istring& game, GameInfo* gameinfo);

	//! Setup the virtual game paths for the given game/GameInfo
	//! Specifically, @u8 and @work (//!!FIXME)
	//! \param game The id of the game
	//! \param gameinfo The GameInfo for the game
	void setupGamePaths(Pentagram::istring& game, GameInfo* gameinfo);

	bool isRunning;
	
	Pentagram::istring gamename;
	
	// minimal system
	GameInfo* gameinfo;
	FileSystem* filesystem;
	ConfigFileManager* configfileman;
	SettingManager* settingman;

	Args parameters;
	
	static CoreApp* application;
	
private:
	int argc;
	const char* const* argv;

	//! start SDL
	void SDLInit();

	//! start filesystem, kernel, config
	virtual void sysInit();

	//! parse commandline arguments
	void ParseArgs(int argc, const char* const*  argv);

	//! setup default virtual paths (@home, @data)
	void setupVirtualPaths();

	//! load configuration files
	void loadConfig();

protected:
	//! Do initial Game init
	//! \return false if no default game (implies go to Pentagram Menu)
	bool getDefaultGame();

	//! Setup up a game
	//! \return false if failed (implies go to Pentagram Menu)
	bool setupGameInfo();

	//! kill current gameinfo
	void killGame();

	bool oHelp;
	bool oQuiet;
	bool oVQuiet;
};

#endif

