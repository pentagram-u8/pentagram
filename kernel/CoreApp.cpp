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

CoreApp::CoreApp(const int argc, const char * const * const argv,
	const string _defaultGame, bool delayPostInit)
	: isRunning(false), framenum(0), kernel(0), filesystem(0), config(0),
		defaultGame(_defaultGame), oHelp(false), oQuiet(false), oVQuiet(false)
{
	assert(application == 0);
	application = this;

	// we need to preparse the arguments to find out if we're quiet or not
	ParseArgs(argc, argv);

	// if we're spitting out help, we probably want to avoid having the other cruft dumped too...
	if(oHelp) { oQuiet=oVQuiet=true; }
	if(oQuiet) con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO & ~MM_MINOR_WARN));
	if(oVQuiet) con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO & ~MM_MINOR_WARN & ~MM_MAJOR_WARN & ~MM_MINOR_ERR));

	sysInit();

	if(!delayPostInit)
		postInit(argc, argv);
}

CoreApp::~CoreApp()
{
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(config);

	application = 0;
}

// Init sdl
void CoreApp::SDLInit()
{
	con.Print(MM_INFO, "Initialising SDL...\n");
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
	con.Print(MM_INFO, "Creating Kernel...\n");
	kernel = new Kernel;

	con.Print(MM_INFO, "Creating FileSystem...\n");
	filesystem = new FileSystem;

	con.Print(MM_INFO, "Creating Configuration...\n");
	config = new Configuration;

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
	con.Print(MM_INFO, "Trying built-in data path\n");
	filesystem->AddVirtualPath("@data", data);
}

// load configuration files
void CoreApp::loadConfig()
{
	con.Print(MM_INFO, "Loading configuration files:\n");

	// system-wide config
	if (config->readConfigFile("@data/pentagram.cfg", "config"))
		con.Print(MM_INFO, "@data/pentagram.cfg... Ok\n");
	else
		con.Print(MM_MINOR_WARN, "@data/pentagram.cfg... Failed\n");

	// user config
	if (config->readConfigFile("@home/pentagram.cfg", "config"))
		con.Print(MM_INFO, "@home/pentagram.cfg... Ok\n");
	else
		con.Print(MM_MINOR_WARN, "@home/pentagram.cfg... Failed\n");

	con.Printf(MM_INFO, "Game: %s\n", game.c_str());

	// Question: config files can specify an alternate data path
	// Do we reload the config files if that path differs from the
	// hardcoded data path? (since the system-wide config file is in @data)

	/**********
	  load pentagram specific data path
	 **********/
	//addPath("config/general/data","@data", "Data Path");
	std::string data;
	con.Print(MM_INFO, "Reading \"config/general/data\" config key.\n");
	config->value("config/general/data", data, "");
	if (data != "") {
		con.Printf(MM_INFO, "Data Path: %s\n",data.c_str());
		filesystem->AddVirtualPath("@data", data);
	}
	else {
		con.Print(MM_MINOR_WARN, "Key not found. Data path set to default.\n");
	}

	/**********
	  load main game data path
	 **********/
	std::string gpath;
	con.Printf(MM_INFO, "Reading \"config/%s/path\" config key.\n", game.c_str());
	config->value(string("config/")+game+"/path", gpath, ".");
	filesystem->AddVirtualPath("@u8", gpath);
	con.Printf(MM_INFO, "Game Path: %s\n", gpath.c_str());

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
	con.Printf(MM_INFO, "Reading \"config/%s/work\" config key.\n", game.c_str());
	config->value(string("config/")+game+"/work", work, string(home+"/"+game+"-work").c_str());
	filesystem->AddVirtualPath("@work", work, true); // force creation if it doesn't exist
	con.Printf(MM_INFO, "U8 Workdir: %s\n", work.c_str());

	// make sure we've got a minimal sane filesystem under there...
	filesystem->MkDir("@work/usecode");
	filesystem->MkDir("@work/usecode/obj");
	filesystem->MkDir("@work/usecode/src");
	filesystem->MkDir("@work/usecode/asm");
}

void CoreApp::ParseArgs(const int argc, const char * const * const argv)
{
	parameters.declare("--game",	&game,		defaultGame.c_str());
	parameters.declare("-h",		&oHelp, 	true);
	parameters.declare("-q", 		&oQuiet,	true);
	parameters.declare("-qq",		&oVQuiet,	true);
	
	parameters.process(argc, argv);
}

void CoreApp::helpMe()
{
	con.Print("\t-h\t\t- quick help menu (this)\n");
	con.Print("\t-q\t\t- silence general logging messages\n");
	con.Print("\t-qq\t\t- silence general logging messages and non-critical warnings/errors\n");
	con.Print("\t--game {name}\t- executes the appropritate game by reading the specific 'name' section of the pentagram.cfg file\n");
}

