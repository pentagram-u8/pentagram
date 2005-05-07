/*
Copyright (C) 2004 The Pentagram team

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
#include "ConfigFileManager.h"
#include "SettingManager.h"
#include "MemoryManager.h"

#include "UCMachine.h"
#include "UCProcess.h"
#include "UsecodeFlex.h"
#include "IDataSource.h"
#include "Args.h"
#include "GameInfo.h"
#include "GameDetector.h"

#include "SDL.h"

#if defined(WIN32) && defined(WIN32_USE_MY_DOCUMENTS)
#include <shlobj.h>
#endif

using std::string;

static void ToLower(std::string& str);


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(CoreApp);

CoreApp* CoreApp::application = 0;

CoreApp::CoreApp(int argc_, const char* const* argv_)
	: isRunning(false), gameinfo(0), kernel(0), filesystem(0),
	  configfileman(0), settingman(0), memorymanager(0), argc(argc_),
	  argv(argv_), oHelp(false), oQuiet(false), oVQuiet(false)
{
	assert(application == 0);
	application = this;
}

CoreApp::~CoreApp()
{
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(settingman);
	FORGET_OBJECT(configfileman);
	FORGET_OBJECT(gameinfo);

	FORGET_OBJECT(memorymanager);
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
	parameters.declare("--game",	&gamename,	"");
	parameters.declare("-h",		&oHelp, 	true);
	parameters.declare("--help",	&oHelp,		true);
	parameters.declare("-q", 		&oQuiet,	true);
	parameters.declare("-qq",		&oVQuiet,	true);	
}

// Init sdl
void CoreApp::SDLInit()
{
	con.Print(MM_INFO, "Initialising SDL...\n");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);
}

void CoreApp::sysInit()
{
	SDLInit();

	gameinfo = new GameInfo();

	// Create the kernel
	con.Print(MM_INFO, "Creating Kernel...\n");
	kernel = new Kernel;

	con.Print(MM_INFO, "Creating FileSystem...\n");
	filesystem = new FileSystem;

	con.Print(MM_INFO, "Creating Configuration...\n");
	configfileman = new ConfigFileManager();
	settingman = new SettingManager();
	settingman->setDomainName(SettingManager::DOM_GLOBAL, "pentagram");
	settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);

	con.Print(MM_INFO, "Creating MemoryManager...\n");
	memorymanager = new MemoryManager();

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
//#elif defined(UNDER_CE)
//	home = "\\\\Pierce\\Moo\\UC";
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
	bool ok = filesystem->AddVirtualPath("@data", data);
	if (!ok) {
#ifndef BUILTIN_DATA
		pout << "Error opening default data directory: " << data << std::endl;
		pout << "Trying custom data path specified in configuration file."
			 << std::endl;
#endif
	} else {
		pout << "Default data path: " << data << std::endl;
	}
}

// load configuration files
void CoreApp::loadConfig()
{
	pout << "Loading configuration files:" << std::endl;

	bool dataconf, homeconf;

	// system-wide config, read-only
	dataconf = settingman->readConfigFile("@data/pentagram.ini", true);

	// user config
	homeconf = settingman->readConfigFile("@home/pentagram.ini");

	if (!homeconf && !dataconf) {
		pout << "No configuration files found." << std::endl;
	} else {

		if (dataconf)
			pout << "@data/pentagram.ini" << std::endl;
		if (homeconf)
			pout << "@home/pentagram.ini" << std::endl;
	}

	//  load pentagram specific data path
	std::string data;
	if (settingman->get("data", data, SettingManager::DOM_GLOBAL)) {
		pout << "Setting custom data path: " << data << std::endl;
		bool ok = filesystem->AddVirtualPath("@data", data);
		if (!ok) {
			perr << "Error opening data directory." << std::endl;
		}
	}
}

void CoreApp::initGame()
{
	std::vector<Pentagram::istring> games;
	games = settingman->listGames();
	con.Print(MM_INFO, "Scanning config file for games:\n");
	std::vector<Pentagram::istring>::iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		std::string game = *iter;
		GameInfo info;
		bool detected = getGameInfo(game, &info);
		con.Printf(MM_INFO, "%s: ", game.c_str());
		if (detected) {
			switch(info.type) {
			case GameInfo::GAME_U8:
				con.Print(MM_INFO, "Ultima 8, ");
				break;
			case GameInfo::GAME_REMORSE:
				con.Print(MM_INFO, "Crusader: No Remorse, ");
				break;
			case GameInfo::GAME_REGRET:
				con.Print(MM_INFO, "Crusader: No Regret, ");
				break;
			default:
				con.Print(MM_INFO, "Unknown, ");
			} 

			switch(info.language) {
			case GameInfo::GAMELANG_ENGLISH:
				con.Print(MM_INFO, "English");
				break;
			case GameInfo::GAMELANG_FRENCH:
				con.Print(MM_INFO, "French");
				break;
			case GameInfo::GAMELANG_GERMAN:
				con.Print(MM_INFO, "German");
				break;
			case GameInfo::GAMELANG_SPANISH:
				con.Print(MM_INFO, "Spanish");
				break;
			default:
				con.Print(MM_INFO, "Unknown");
			}
		} else {
			con.Print(MM_INFO, "(unknown)");
		}
		con.Print(MM_INFO, "\n");
	}

	if (gamename == "") {
		std::string defaultgame;
		bool defaultset = settingman->get("defaultgame", defaultgame,
										  SettingManager::DOM_GLOBAL);
		if (defaultset) {
			// default game specified in config file
			gamename = defaultgame;
		} else if (games.size() == 1) {
			// only one game in config file, so pick that
			gamename = (*(games.begin())).c_str();
		} else if (games.size() == 0) {
			perr << "No games set up in configuration. "
				 << "Please read the README for instructions." << std::endl;
			exit(1); //!! FIXME
		} else {		
			perr << "Multiple games found in configuration, but no default "
				 << "game is selected." << std::endl
				 << "Either start Pentagram with the \"--game <gamename>\","
				 << std::endl
				 << "or set pentagram/defaultgame in pentagram.ini"
				 << std::endl;
			exit(1); //!! FIXME
		}
	}

	pout << "Selected game: " << gamename << std::endl;

	bool foundgame = false;
	for (unsigned int i = 0; i < games.size(); ++i) {
		if (games[i] == gamename) {
			foundgame = true;
			break;
		}
	}

	if (!foundgame) {
		perr << "Game \"" << gamename << "\" not found." << std::endl;
		exit(1);
	}

	// fill our GameInfo struct
	bool ok = getGameInfo(gamename, gameinfo);

	if (!ok) {
		perr << "No installed game found." << std::endl;
		exit(-1);
	}

	setupGamePaths(gamename, gameinfo);
}

bool CoreApp::getGameInfo(std::string& game, GameInfo* gameinfo)
{
	// first try getting the information from the config file
	// if that fails, try to autodetect it

	gameinfo->type = GameInfo::GAME_UNKNOWN;
	gameinfo->version = 0;
	gameinfo->language = GameInfo::GAMELANG_UNKNOWN;

	Pentagram::istring gamekey = "settings/";
	gamekey += game;

	std::string gametype;
	if (!configfileman->get(gamekey+"/type", gametype))
		gametype = "unknown";
	ToLower(gametype);

	std::string version;
	if (!configfileman->get(gamekey+"/version", version))
		version = "unknown";

	std::string language;
	if (!configfileman->get(gamekey+"/language", language))
		language = "unknown";
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
		if (!configfileman->get(gamekey+"/path", path)) return false;

		return GameDetector::detect(path, gameinfo);
	}

	if (gameinfo->type == GameInfo::GAME_UNKNOWN) {
		return false;
	}
		
	return true;
}

void CoreApp::setupGamePaths(std::string& game, GameInfo* /*gameinfo*/)
{
	settingman->setDomainName(SettingManager::DOM_GAME, game);
	settingman->setCurrentDomain(SettingManager::DOM_GAME);

	// load main game data path
	std::string gpath;
	settingman->get("path", gpath, SettingManager::DOM_GAME);
	filesystem->AddVirtualPath("@u8", gpath); //!!FIXME (u8 specific)
	con.Printf(MM_INFO, "Game Path: %s\n", gpath.c_str());


	// load work path. Default is @home/game-work
	// where 'game' in the above is the specified 'game' loaded
	std::string work;
	if (!settingman->get("work", work, SettingManager::DOM_GAME))
		work = "@home/"+game+"-work";

#if 0
	// force creation if it doesn't exist

	// TODO: I don't like these being created here.
	//       I'd prefer them to be created when needed. (-wjp)

	filesystem->AddVirtualPath("@work", work, true);
	con.Printf(MM_INFO, "U8 Workdir: %s\n", work.c_str()); //!!FIXME (u8)

	// make sure we've got a minimal sane filesystem under there...
	filesystem->MkDir("@work/usecode");
	filesystem->MkDir("@work/usecode/obj");
	filesystem->MkDir("@work/usecode/src");
	filesystem->MkDir("@work/usecode/asm");
#endif

	// load savegame path. Default is @home/game-save
	std::string save;
	if (!settingman->get("save", save, SettingManager::DOM_GAME))
		save = "@home/"+game+"-save";

	// force creation if it doesn't exist
	filesystem->AddVirtualPath("@save", save, true);
	con.Printf(MM_INFO, "Savegame directory: %s\n", save.c_str());
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
