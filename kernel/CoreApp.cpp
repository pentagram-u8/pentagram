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

#include "CoreApp.h"

#include "Kernel.h"
#include "FileSystem.h"
#include "Configuration.h"

#include "UCMachine.h"
#include "UCProcess.h"
#include "UsecodeFlex.h"
#include "IDataSource.h"
#include "Args.h"

#include <SDL.h>

#if defined(WIN32) && defined(WIN32_USE_MY_DOCUMENTS)
#include <shlobj.h>
#endif

using std::string;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(CoreApp);

CoreApp* CoreApp::application = 0;

CoreApp::CoreApp(const int argc, const char * const * const argv, bool delayPostInit)
	: isRunning(false), framenum(0), kernel(0), ucmachine(0), filesystem(0), config(0)
{
	assert(application == 0);
	application = this;

	sysInit();

	if(!delayPostInit)
		postInit(argc, argv);
}

CoreApp::~CoreApp()
{
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(config);

	application = 0;
}

// Init sdl
void CoreApp::SDLInit()
{
	pout << "Init SDL" << std::endl;
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
}

void CoreApp::postInit(const int argc, const char * const * const argv)
{
	ParseArgs(argc, argv);
	setupVirtualPaths(); // setup @home, @data
	loadConfig();
}


void CoreApp::sysInit()
{
	SDLInit();

	// Create the kernel
	pout << "Create Kernel" << std::endl;
	kernel = new Kernel;

	pout << "Create FileSystem" << std::endl;
	filesystem = new FileSystem;

	pout << "Create Configuration" << std::endl;
	config = new Configuration;

	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine;
}

void CoreApp::setupVirtualPaths()
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
#elif defined(WIN32) && defined(WIN32_USE_MY_DOCUMENTS)
      TCHAR MyDocumentsPath[MAX_PATH];
      SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, MyDocumentsPath);
      home = MyDocumentsPath;
      home += "\\Pentagram";
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
void CoreApp::loadConfig()
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

void CoreApp::ParseArgs(const int argc, const char * const * const argv)
{
	pout << "Parsing Args" << std::endl;

	parameters.declare("--game",    &game,           "u8");
	//parameters.declare("--singlefile",	&singlefile, true);

	parameters.process(argc, argv);
}

