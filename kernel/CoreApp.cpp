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
#include "GameInfo.h"
#include "GameDetector.h"

#include <SDL.h>

#if defined(WIN32) && defined(WIN32_USE_MY_DOCUMENTS)
#include <shlobj.h>
#endif

using std::string;

static void ToLower(std::string& str);


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(CoreApp);

CoreApp* CoreApp::application = 0;

CoreApp::CoreApp(int argc_, const char* const* argv_)
	: isRunning(false), framenum(0), kernel(0), filesystem(0), config(0),
	  argc(argc_), argv(argv_), oHelp(false), oQuiet(false), oVQuiet(false)
{
	assert(application == 0);
	application = this;
}

CoreApp::~CoreApp()
{
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(config);

	application = 0;
}

void CoreApp::startup()
{
	DeclareArgs(); // Note: this is virtual

	ParseArgs(argc, argv);

	// if we're spitting out help, we probably want to avoid having the
	// other cruft dumped too...
	if(oHelp) { oQuiet=oVQuiet=true; }
	if(oQuiet)
		con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO &
											~MM_MINOR_WARN));
	if(oVQuiet)
		con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO & ~MM_MINOR_WARN
											& ~MM_MAJOR_WARN & ~MM_MINOR_ERR));

	if (oHelp) {
		helpMe(); // Note: this is virtual
		exit(0);
	}


	sysInit();

	setupVirtualPaths(); // setup @home, @data
	loadConfig(); // load config files
}

void CoreApp::DeclareArgs()
{
	parameters.declare("--game",	&game,		"");
	parameters.declare("-h",		&oHelp, 	true);
	parameters.declare("--help",	&oHelp,		true);
	parameters.declare("-q", 		&oQuiet,	true);
	parameters.declare("-qq",		&oVQuiet,	true);	
}

// Init sdl
void CoreApp::SDLInit()
{
	con.Print(MM_INFO, "Initialising SDL...\n");
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
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
	if (config->readConfigFile("@data/pentagram.cfg", "config", true))
		con.Print(MM_INFO, "@data/pentagram.cfg... Ok\n");
	else
		con.Print(MM_MINOR_WARN, "@data/pentagram.cfg... Failed\n");

	// user config
	if (config->readConfigFile("@home/pentagram.cfg", "config"))
		con.Print(MM_INFO, "@home/pentagram.cfg... Ok\n");
	else
		con.Print(MM_MINOR_WARN, "@home/pentagram.cfg... Failed\n");

	con.Printf(MM_INFO, "Game: %s\n", game.c_str());


	//  load pentagram specific data path
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


	//!! move this to some other init function
	std::set<std::string> games;
	games = config->listKeys("config/games", false);
	pout << "Scanning config file for games:" << std::endl;
	std::set<std::string>::iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		std::string game = *iter;
		GameInfo info;
		bool detected = getGameInfo(game, &info);
		pout << game << ": ";
		if (detected) {
			if (info.type == GameInfo::GAME_U8) {
				pout << "U8, ";
			} /* else...*/

			if (info.language == GameInfo::GAMELANG_ENGLISH) {
				pout << "English";
			} else if (info.language == GameInfo::GAMELANG_FRENCH) {
				pout << "French";
			} else if (info.language == GameInfo::GAMELANG_GERMAN) {
				pout << "German";
			} else if (info.language == GameInfo::GAMELANG_SPANISH) {
				pout << "Spanish";
			} /* else...*/
		} else {
			pout << "(unknown)";
		}
		pout << std::endl;
	}

	if (game == "") {
		std::string defaultgame;
		config->value("config/general/defaultgame", defaultgame, "");
		if (defaultgame != "") {
			// default game specified in config file
			game = defaultgame;
		} else if (games.size() == 1) {
			// only one game in config file, so pick that
			game = *(games.begin());
		} else if (games.size() == 0) {
			perr << "No games set up in configuration. "
				 << "Please read the README for instructions." << std::endl;
			exit(1); //!! FIXME
		} else {		
			perr << "Multiple games found in configuration, but no default "
				 << "game is selected." << std::endl
				 << "Either start Pentagram with the \"--game <gamename>\","
				 << std::endl
				 << "or set config/general/defaultgame in pentagram.cfg"
				 << std::endl;
			exit(1); //!! FIXME
		}
	}

	pout << "Selected game: " << game << std::endl;

	if (games.find(game) == games.end()) {
		perr << "Game \"" << game << "\" not found." << std::endl;
		exit(1);
	}

	//!! use GameData's info
	GameInfo info;
	getGameInfo(game, &info);
	setupGamePaths(game, &info);
}

bool CoreApp::getGameInfo(std::string& game, GameInfo* gameinfo)
{
	// first try getting the information from the config file
	// if that fails, try to autodetect it

	gameinfo->type = GameInfo::GAME_UNKNOWN;
	gameinfo->version = 0;
	gameinfo->language = GameInfo::GAMELANG_UNKNOWN;

	std::string gamekey = "config/games/"+game;

	std::string gametype;
	config->value(gamekey+"/type", gametype, "unknown");
	ToLower(gametype);

	std::string version;
	config->value(gamekey+"/version", version, "unknown");

	std::string language;
	config->value(gamekey+"/version", language, "unknown");
	ToLower(language);

	if (gametype == "u8") {
		gameinfo->type = GameInfo::GAME_U8;
	}

	//!! TODO: version parsing

	if (language == "english") {
		gameinfo->language = GameInfo::GAMELANG_ENGLISH;
	} else if (language == "french") {
		gameinfo->language = GameInfo::GAMELANG_FRENCH;
	} else if (language == "german") {
		gameinfo->language = GameInfo::GAMELANG_GERMAN;
	} else if (language == "spanish") {
		gameinfo->language = GameInfo::GAMELANG_SPANISH;
	}

	if (gameinfo->type == GameInfo::GAME_UNKNOWN ||
		/* gameinfo->version == 0 || */
		gameinfo->language == GameInfo::GAMELANG_UNKNOWN)
	{
		std::string path;
		config->value(gamekey+"/path", path, "");

		if (path == "") return false;

		return GameDetector::detect(path, gameinfo);
	}
		
	//!! TODO: game detection
		
	return true;
}

void CoreApp::setupGamePaths(std::string& game, GameInfo* gameinfo)
{
	std::string gamekey = "config/games/"+game;

	// load main game data path
	std::string gpath;
	con.Printf(MM_INFO, "Reading \"config/games/%s/path\" config key.\n", game.c_str());
	config->value(gamekey+"/path", gpath, ".");
	filesystem->AddVirtualPath("@u8", gpath); //!!FIXME
	con.Printf(MM_INFO, "Game Path: %s\n", gpath.c_str());



	// load work path. Default is @home/game-work
	// where 'game' in the above is the specified 'game' loaded
	std::string work("@home/"+game+"-work");
	con.Printf(MM_INFO, "Reading \"config/games/%s/work\" config key.\n",
			   game.c_str());
	config->value(gamekey+"/work", work,
				  string("@home/"+game+"-work").c_str());

	// force creation if it doesn't exist
	filesystem->AddVirtualPath("@work", work, true);
	con.Printf(MM_INFO, "U8 Workdir: %s\n", work.c_str());

	// make sure we've got a minimal sane filesystem under there...
	filesystem->MkDir("@work/usecode");
	filesystem->MkDir("@work/usecode/obj");
	filesystem->MkDir("@work/usecode/src");
	filesystem->MkDir("@work/usecode/asm");
}

void CoreApp::ParseArgs(const int argc, const char * const * const argv)
{
	parameters.process(argc, argv);
}

void CoreApp::helpMe()
{
	con.Print("\t-h\t\t- quick help menu (this)\n");
	con.Print("\t-q\t\t- silence general logging messages\n");
	con.Print("\t-qq\t\t- silence general logging messages and\n\t\t\t  non-critical warnings/errors\n");
	con.Print("\t--game {name}\t- select a game\n");
}



static void ToLower(std::string& str)
{
	for (unsigned int i = 0; i < str.size(); ++i)
	{
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((str[i] >= 'A') && (str[i] <= 'Z')) str[i] += 32;
#else
		str[i] = static_cast<char>(std::tolower(str[i]));
#endif
	}

}
