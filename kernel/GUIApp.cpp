/*
Copyright (C) 2002-2004 The Pentagram team

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

#include "GUIApp.h"

#include <SDL.h>

//!! a lot of these includes are just for some hacks... clean up sometime
#include "Kernel.h"
#include "FileSystem.h"
#include "SettingManager.h"
#include "ConfigFileManager.h"
#include "ObjectManager.h"
#include "GameInfo.h"

#include "HIDManager.h"
#include "HIDBinding.h"

#include "RenderSurface.h"
#include "Texture.h"
#include "PaletteManager.h"
#include "Palette.h"
#include "GameData.h"
#include "World.h"
#include "Direction.h"

#include "U8Save.h"
#include "SavegameWriter.h"
#include "Savegame.h"

#include "Gump.h"
#include "DesktopGump.h"
#include "ConsoleGump.h"
#include "GameMapGump.h"
#include "BarkGump.h"

#include "Actor.h"
#include "ActorAnimProcess.h"
#include "Font.h"
#include "FontShapeFlex.h"
#include "u8intrinsics.h"
#include "Egg.h"
#include "CurrentMap.h"
#include "UCList.h"
#include "LoopScript.h"

#include "EggHatcherProcess.h" // for a hack
#include "UCProcess.h" // more hacking
#include "GumpNotifyProcess.h" // guess
#include "DelayProcess.h"
#include "GravityProcess.h"
#include "MissileProcess.h"
#include "TeleportToEggProcess.h"
#include "ItemFactory.h"
#include "PathfinderProcess.h"
#include "AvatarMoverProcess.h"
#include "ResurrectionProcess.h"

#include "MovieGump.h"

#include "DisasmProcess.h"
#include "CompileProcess.h"

#ifdef USE_SDLTTF
#include "SDL_ttf.h"
#include "TTFont.h"
// HACK
TTFont* ttffont;
#endif

#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
#include <windows.h>
#include "resource.h"
#endif

#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)
#include "Shape.h"
#include "ShapeFrame.h"
#include "SoftRenderSurface.h"
#include "SDL_syswm.h"

struct HWMouseCursor {
	HICON hCursor;
};

#endif

#include "XFormBlend.h"

#include "MusicProcess.h"
#include "MusicFlex.h"

#include "util.h"

#include "MidiDriver.h"
#if defined(WIN32) && !defined(UNDER_CE)
#include "WindowsMidiDriver.h"
#endif
#ifdef MACOSX
#include "CoreAudioMidiDriver.h"
#endif
#ifdef USE_FMOPL_MIDI
#include "FMOplMidiDriver.h"
#endif
#ifdef USE_TIMIDITY_MIDI
#include "TimidityMidiDriver.h"
#endif

using std::string;

DEFINE_RUNTIME_CLASSTYPE_CODE(GUIApp,CoreApp);

GUIApp::GUIApp(int argc, const char* const* argv)
	: CoreApp(argc, argv), ucmachine(0), screen(0),
	  palettemanager(0), gamedata(0), world(0), desktopGump(0),
	  consoleGump(0), gameMapGump(0), avatarMoverProcess(0),
	  runGraphicSysInit(false), runSDLInit(false),
	  frameSkip(false), frameLimit(true), interpolate(false),
	  animationRate(100), avatarInStasis(false), paintEditorItems(false),
	  painting(false), dragging(DRAG_NOT), timeOffset(0),
	  midi_driver(0), midi_volume(255),
	  drawRenderStats(false)
{
	application = this;

	for (int i = 0; i < NUM_MOUSEBUTTONS+1; ++i) {
		mouseButton[i].downGump = 0;
		mouseButton[i].lastDown = 0;
		mouseButton[i].state = MBS_HANDLED;
	}

	con.AddConsoleCommand("GUIApp::saveGame", ConCmd_saveGame);
	con.AddConsoleCommand("GUIApp::loadGame", ConCmd_loadGame);
	con.AddConsoleCommand("HIDManager::bind", HIDManager::ConCmd_bind);
	con.AddConsoleCommand("HIDManager::unbind", HIDManager::ConCmd_unbind);
	con.AddConsoleCommand("HIDManager::listbinds", HIDManager::ConCmd_listbinds);
	con.AddConsoleCommand("HIDManager::save", HIDManager::ConCmd_save);
	con.AddConsoleCommand("Kernel::processTypes", Kernel::ConCmd_processTypes);
	con.AddConsoleCommand("ObjectManager::objectTypes",
								   ObjectManager::ConCmd_objectTypes);
	con.AddConsoleCommand("MainActor::teleport", MainActor::ConCmd_teleport);
	con.AddConsoleCommand("MainActor::mark", MainActor::ConCmd_mark);
	con.AddConsoleCommand("MainActor::recall", MainActor::ConCmd_recall);
	con.AddConsoleCommand("MainActor::listmarks", MainActor::ConCmd_listmarks);
	con.AddConsoleCommand("quit", ConCmd_quit);	
	con.AddConsoleCommand("GUIApp::quit", ConCmd_quit);	
	con.AddConsoleCommand("GUIApp::drawRenderStats", ConCmd_drawRenderStats);
	con.AddConsoleCommand("MovieGump::play", MovieGump::ConCmd_play);
}

GUIApp::~GUIApp()
{
	con.RemoveConsoleCommand("GUIApp::saveGame");
	con.RemoveConsoleCommand("GUIApp::loadGame");
	con.RemoveConsoleCommand("HIDManager::bind");
	con.RemoveConsoleCommand("HIDManager::unbind");
	con.RemoveConsoleCommand("HIDManager::listbinds");
	con.RemoveConsoleCommand("HIDManager::save");
	con.RemoveConsoleCommand("Kernel::processTypes");
	con.RemoveConsoleCommand("ObjectManager::objectTypes");
	con.RemoveConsoleCommand("MainActor::teleport");
	con.RemoveConsoleCommand("MainActor::mark");
	con.RemoveConsoleCommand("MainActor::recall");
	con.RemoveConsoleCommand("MainActor::listmarks");
	con.RemoveConsoleCommand("quit");
	con.RemoveConsoleCommand("GUIApp::quit");
	con.RemoveConsoleCommand("GUIApp::drawRenderStats");
	con.RemoveConsoleCommand("MovieGump::play");

	FORGET_OBJECT(objectmanager);
	FORGET_OBJECT(hidmanager);
	deinit_midi();
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(palettemanager);
	FORGET_OBJECT(gamedata);
	FORGET_OBJECT(world);
	FORGET_OBJECT(ucmachine);

#ifdef USE_SDLTTF
	if (ttffont)
		FORGET_OBJECT(ttffont);
#endif
}

void GUIApp::startup()
{
	// Set the console to auto paint, till we have finished initing
	con.SetAutoPaint(conAutoPaint);

	// parent's startup first
	CoreApp::startup();

	//!! move this elsewhere
	kernel->addProcessLoader("DelayProcess",
							 ProcessLoader<DelayProcess>::load);
	kernel->addProcessLoader("GravityProcess",
							 ProcessLoader<GravityProcess>::load);
	kernel->addProcessLoader("PaletteFaderProcess",
							 ProcessLoader<PaletteFaderProcess>::load);
	kernel->addProcessLoader("TeleportToEggProcess",
							 ProcessLoader<TeleportToEggProcess>::load);
	kernel->addProcessLoader("ActorAnimProcess",
							 ProcessLoader<ActorAnimProcess>::load);
	kernel->addProcessLoader("AvatarMoverProcess",
							 ProcessLoader<AvatarMoverProcess>::load);
	kernel->addProcessLoader("PathfinderProcess",
							 ProcessLoader<PathfinderProcess>::load);
	kernel->addProcessLoader("SpriteProcess",
							 ProcessLoader<SpriteProcess>::load);
	kernel->addProcessLoader("MissileProcess",
							 ProcessLoader<MissileProcess>::load);
	kernel->addProcessLoader("CameraProcess",
							 ProcessLoader<CameraProcess>::load);
	kernel->addProcessLoader("MusicProcess",
							 ProcessLoader<MusicProcess>::load);
	kernel->addProcessLoader("EggHatcherProcess",
							 ProcessLoader<EggHatcherProcess>::load);
	kernel->addProcessLoader("UCProcess",
							 ProcessLoader<UCProcess>::load);
	kernel->addProcessLoader("GumpNotifyProcess",
							 ProcessLoader<GumpNotifyProcess>::load);
	kernel->addProcessLoader("ResurrectionProcess",
							 ProcessLoader<ResurrectionProcess>::load);

	gamedata = new GameData();

	pout << "Create ObjectManager" << std::endl;
	objectmanager = new ObjectManager();

	pout << "Create HIDManager" << std::endl;
	hidmanager = new HIDManager();
	
	pout << "Loading HIDBindings" << std::endl;
	GameInfo info;
	getGameInfo(game, &info);
	gamedata->setGameInfo(info);

	if (info.type == GameInfo::GAME_U8) {
		// system-wide config
		if (configfileman->readConfigFile("@data/u8bindings.ini",
										  "bindings", true))
			con.Print(MM_INFO, "@data/u8bindings.ini... Ok\n");
		else
			con.Print(MM_MINOR_WARN, "@data/u8bindings.ini... Failed\n");
	}

	hidmanager->loadBindings();
	
	pout << "Create UCMachine" << std::endl;
	ucmachine = new UCMachine(U8Intrinsics);

	GraphicSysInit();

	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);

	U8Playground();

	// This Must be AFTER U8Playground() cause it might need gamedata
	init_midi();

	// Unset the console auto paint, since we have finished initing
	con.SetAutoPaint(0);
}

void GUIApp::sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	GUIApp *app = app->get_instance();
	MidiDriver *drv = app->getMidiDriver();
	if (drv && drv->isSampleProducer())
		drv->produceSamples(reinterpret_cast<sint16*>(stream), len);
}

void GUIApp::init_midi()
{
	MidiDriver * new_driver = 0;
	SDL_AudioSpec desired, obtained;

	desired.format = AUDIO_S16SYS;
	desired.freq = 22050;
	desired.channels = 2;
	desired.samples = 4096;
	desired.callback = sdlAudioCallback;
	desired.userdata = 0;

#ifdef UNDER_CE
	desired.freq = 11025;
	desired.channels = 1;
#endif
	pout << "Initializing Midi" << std::endl;

	std::vector<const MidiDriver::MidiDriverDesc*>	midi_drivers;

#if 1
	// Open SDL Audio (even though we may not need it)
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	int ret = SDL_OpenAudio(&desired, &obtained);
	bool audio_ok = (ret == 0);

	// Now, add the drivers in order of priority.
	// Do OS Native drivers first, then then Timidity, then FMOpl

#ifdef MACOSX
	midi_drivers.push_back(CoreAudioMidiDriver::getDesc());
#endif
#if defined(WIN32) && !defined(UNDER_CE)
	midi_drivers.push_back(WindowsMidiDriver::getDesc());
#endif
#ifdef USE_TIMIDITY_MIDI
	midi_drivers.push_back(TimidityMidiDriver::getDesc());
#endif
#ifdef USE_FMOPL_MIDI
	midi_drivers.push_back(FMOplMidiDriver::getDesc());
#endif

	// First thing attempt to find the Midi driver as specified in the config
	std::string desired_driver;
	settingman->setDefault("midi_driver", "default");
	settingman->get("midi_driver", desired_driver);
	const char * drv = desired_driver.c_str();

	// Has the config file specified disabled midi?
	if (audio_ok && Pentagram::strcasecmp(drv, "disabled"))
	{
		std::vector<const MidiDriver::MidiDriverDesc*>::iterator it;

		// Ok, it hasn't so search for the driver
		for (it = midi_drivers.begin(); it < midi_drivers.end(); it++) {

			// Found it (case insensitive)
			if (!Pentagram::strcasecmp(drv, (*it)->name)) {

				pout << "Trying config specified Midi driver: `" << (*it)->name << "'" << std::endl;

				new_driver = (*it)->createInstance();
				if (new_driver) {

					if (new_driver->initMidiDriver(obtained.freq,obtained.channels==2)) {
						delete new_driver;
						new_driver = 0; 
					} 
				}
			}
		}

		// Uh oh, we didn't manage to load a driver! 
		// Search for the first working one
		if (!new_driver) for (it = midi_drivers.begin(); it < midi_drivers.end(); it++) {

			pout << "Trying: `" << (*it)->name << "'" << std::endl;

			new_driver = (*it)->createInstance();
			if (new_driver) {

				// Got it
				if (!new_driver->initMidiDriver(obtained.freq,obtained.channels==2)) 
					break;

				// Oh well, try the next one
				delete new_driver;
				new_driver = 0; 
			}
		}
	}
	else
	{
		new_driver = 0; // silence :-)
	}

#else
	new_driver = 0; // silence :-)
#endif

	// If the driver is a 'sample' producer we need to hook it to SDL
	if (new_driver)
	{
		new_driver->setGlobalVolume(midi_volume);

		SDL_LockAudio();
		midi_driver = new_driver;
		SDL_UnlockAudio();
	}

	// GO GO GO!
	SDL_PauseAudio(0);

	// Create the Music Process
	Process *mp = new MusicProcess(new_driver);
	kernel->addProcess(mp);
}

void GUIApp::deinit_midi()
{
	SDL_CloseAudio();
	if (midi_driver)  midi_driver->destroyMidiDriver();
	delete midi_driver;
	midi_driver = 0;
}

void GUIApp::DeclareArgs()
{
	// parent's arguments first
	CoreApp::DeclareArgs();

	// anything else?
}

void GUIApp::run()
{
	isRunning = true;

	sint32 next_ticks = SDL_GetTicks()*3;	// Next time is right now!
	
	// Ok, the theory is that if this is set to true then we must do a repaint
	// At the moment only it's ignored
	bool repaint;

	SDL_Event event;
	while (isRunning) {
		inBetweenFrame = true;	// Will get set false if it's not an inBetweenFrame

		if (!frameLimit) {
			repaint = false;
			
			if (kernel->runProcesses()) repaint = true;
			desktopGump->Run(kernel->getFrameNum());
			inBetweenFrame = false;
			next_ticks = animationRate + SDL_GetTicks()*3;
			lerpFactor = 256;
		}
		else 
		{
			sint32 ticks = SDL_GetTicks()*3;
			sint32 diff = next_ticks - ticks;
			repaint = false;

			while (diff < 0) {
				next_ticks += animationRate;
				if (kernel->runProcesses()) repaint = true;
				desktopGump->Run(kernel->getFrameNum());
#if 0
				perr << "--------------------------------------" << std::endl;
				perr << "NEW FRAME" << std::endl;
				perr << "--------------------------------------" << std::endl;
#endif
				inBetweenFrame = false;

				ticks = SDL_GetTicks()*3;

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time. 
				if (!frameSkip && diff <= -animationRate*2) next_ticks = animationRate + ticks;

				diff = next_ticks - ticks;
				if (!frameSkip) break;
			}

			// Calculate the lerp_factor
			lerpFactor = ((animationRate-diff)*256)/animationRate;
			//pout << "lerpFactor: " << lerpFactor << " framenum: " << framenum << std::endl;
			if (!interpolate || kernel->isPaused() || lerpFactor > 256)
				lerpFactor = 256;
		}


		repaint = true;

		// get & handle all events in queue
		while (isRunning && SDL_PollEvent(&event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Paint Screen
		paint();

		// Do a delay
		SDL_Delay(5);
	}
}

void GUIApp::U8Playground()
{
	inBetweenFrame = 0;
	lerpFactor = 256;

	// Load palette
	pout << "Load Palette" << std::endl;
	palettemanager = new PaletteManager(screen);
	IDataSource *pf = filesystem->ReadFile("@u8/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load static/u8pal.pal. Exiting" << std::endl;
		std::exit(-1);
	}
	pf->seek(4); // seek past header

	IBufferDataSource xfds(U8XFormPal,1024);
	palettemanager->load(PaletteManager::Pal_Game, *pf, xfds);
	delete pf;

	pout << "Load GameData" << std::endl;
	gamedata->loadU8Data();

#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)
	CreateHWCursors();
#endif

	// Initialize world
	pout << "Initialize World" << std::endl;
	world = new World();

	IDataSource *saveds = filesystem->ReadFile("@u8/savegame/u8save.000");
	if (!saveds) {
		perr << "Unable to load savegame/u8save.000. Exiting" << std::endl;
		std::exit(-1);
	}
	U8Save *u8save = new U8Save(saveds);

	IDataSource *nfd = u8save->get_datasource("NONFIXED.DAT");
	if (!nfd) {
		perr << "Unable to load savegame/u8save.000/NONFIXED.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	world->initMaps();
	world->loadNonFixed(nfd);

	IDataSource *icd = u8save->get_datasource("ITEMCACH.DAT");
	if (!icd) {
		perr << "Unable to load savegame/u8save.000/ITEMCACH.DAT. Exiting" << std::endl;
		std::exit(-1);
	}
	IDataSource *npcd = u8save->get_datasource("NPCDATA.DAT");
	if (!npcd) {
		perr << "Unable to load savegame/u8save.000/NPCDATA.DAT. Exiting" << std::endl;
		std::exit(-1);
	}

	world->loadItemCachNPCData(icd, npcd);
	delete u8save;

	Actor* av = world->getNPC(1);

	// avatar needs a backpack ... CONSTANTs and all that
	Container* backpack = p_dynamic_cast<Container*>(
		ItemFactory::createItem(529, 0, 0, 0, 0, 0, 0));

	// a little bonus :-)
	Item* money = ItemFactory::createItem(143, 7, 500, 0, 0, 0, 0);
	money->moveToContainer(backpack);
	money->setGumpLocation(40, 20);

	// skull of quakes
	Item *skull = ItemFactory::createItem(814, 0, 0, 0, 0, 0, 0);
	skull->moveToContainer(backpack);  
	skull->setGumpLocation(60, 20);

	// recall item
	Item *recall = ItemFactory::createItem(833, 0, 0, 0, 0, 0, 0);
	recall->moveToContainer(backpack);
	recall->setGumpLocation(20, 20);

	// sword
	Item* sword = ItemFactory::createItem(420, 0, 0, 0, 0, 0, 0);
	sword->moveToContainer(backpack);
	sword->setGumpLocation(20, 30);

	// armour
	Item* armour = ItemFactory::createItem(64, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(30, 30);

	armour = ItemFactory::createItem(532, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(40, 30);

	armour = ItemFactory::createItem(539, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(50, 30);

	armour = ItemFactory::createItem(530, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(10, 40);

	armour = ItemFactory::createItem(531, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(20, 40);

	// oil flasks
	Item* flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);

	backpack->assignObjId();
	backpack->moveToContainer(av);

	if (av)
		world->switchMap(av->getMapNum());
	else
		world->switchMap(3);

	// Create GameMapGump
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);
	
	pout << "Create GameMapGump" << std::endl;
	gameMapGump = new GameMapGump(0, 0, dims.w, dims.h);
	gameMapGump->InitGump();
	desktopGump->AddChild(gameMapGump);

	pout << "Create Camera" << std::endl;
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	pout << "Create AvatarMoverProcess" << std::endl;
	avatarMoverProcess = new AvatarMoverProcess();
	kernel->addProcess(avatarMoverProcess);

//	av->teleport(40, 16240, 15240, 64); // central Tenebrae
//	av->teleport(3, 11391, 1727, 64); // docks, near gate
//	av->teleport(39, 16240, 15240, 64); // West Tenebrae
//	av->teleport(41, 12000, 15000, 64); // East Tenebrae
//	av->teleport(8, 14462, 15178, 48); // before entrance to Mythran's house
//	av->teleport(40, 13102,9474,48); // entrance to Mordea's throne room
//	av->teleport(54, 14783,5959,8); // shrine of the Ancient Ones; Hanoi
//	av->teleport(5, 5104,22464,48); // East road (tenebrae end)

	pout << "Paint Inital display" << std::endl;
	consoleGump->HideConsole();
	paint();
}

// conAutoPaint hackery
void GUIApp::conAutoPaint(void)
{
	GUIApp *app = GUIApp::get_instance();
	if (app && !app->isPainting()) app->paint();
}

#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)

WNDPROC oldWindowProc = 0;
bool allow_set_cursor = true;

LRESULT CALLBACK GUIApp::myWindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
  )
{
	if (uMsg == WM_SETCURSOR) {
		if ( allow_set_cursor = (LOWORD(lParam) == HTCLIENT) ) {
			GUIApp *app = GUIApp::get_instance();
			GameData *gamedata = app->gamedata;

			if (gamedata) {
				Shape* mouse = gamedata->getMouse();
				if (mouse) {
					int frame = app->getMouseFrame();
					if (frame >= 0)
						SetCursor(app->hwcursors[frame].hCursor);
				}
			}
			return TRUE;
		}
	}

	return CallWindowProc(oldWindowProc, hwnd, uMsg, wParam, lParam);
}

void GUIApp::CreateHWCursors()
{
	Shape* mouse = gamedata->getMouse();
	hwcursors = new HWMouseCursor[mouse->frameCount()];
	std::memset (hwcursors, 0, sizeof(HWMouseCursor) * mouse->frameCount());

	for (uint32 frame = 0; frame < mouse->frameCount(); frame++)
	{
		ShapeFrame *f = mouse->getFrame(frame);
		uint32 bpp = BaseSoftRenderSurface::s_bpp;
		int buf_width = f->width;
		int buf_height = f->height;

		// DIB must be dword aligned
		if (bpp != 32) buf_width = (buf_width+1)&~1;	

		uint8 *buf = new uint8 [bpp/8 * buf_width * buf_height];

		RenderSurface *surf;

		if (bpp == 32)
			surf = new SoftRenderSurface<uint32>(buf_width, buf_height, buf);
		else
			surf = new SoftRenderSurface<uint16>(buf_width, buf_height, buf);

		surf->BeginPainting();
		surf->Fill32(0x00FF00FF, 0, 0, buf_width, buf_height);
		surf->Paint(mouse, frame, f->xoff, f->yoff);
		surf->EndPainting();

		int clear_col = PACK_RGB8(0xFF,0x00,0xFF);

		//
		// Mask
		//

		// 1 bit bitmap must be word aligned 
		uint32 bit_width = (buf_width+15)&~15;
		uint8 *buf_mask = new uint8 [bit_width/8 * buf_height*2];

		// Clear it
		std::memset(buf_mask, 0x00, bit_width/8 * buf_height * 2);


		if (bpp == 32)
		{
			uint32 *buf32 = (uint32*)buf;
			for (int y = 0; y < buf_height; y++)
			{
				for (int x = 0; x < buf_width; x++)
				{
					bool black = (x & 1) == (y & 1);
					uint32 bit = y * bit_width + x;
					uint32 byte = bit/8;
					bit = 7-(bit % 8);

					// If background is clear colour, mask it out
					if (buf32[buf_width*y + x] == clear_col)
					{
						buf32[buf_width*y + x] = 0;
						buf_mask[byte] |=   1<< bit;
					}
					// Make any non black make white
					else if (buf32[buf_width*y + x])
						buf_mask[byte+((buf_height*bit_width)/8)] |= 1<<bit;
				}
			}
		}
		else
		{
			uint16 *buf16 = (uint16*)buf;
			for (int y = 0; y < buf_height; y++)
			{
				for (int x = 0; x < buf_width; x++)
				{
					bool black = (x & 1) == (y & 1);
					uint32 bit = y * bit_width + x;
					uint32 byte = bit/8;
					bit = 7-(bit % 8);

					// If background is clear colour, mask it out
					if (buf16[buf_width*y + x] == clear_col)
					{
						buf16[buf_width*y + x] = 0;
						buf_mask[byte] |=   1<< bit;
					}
					// Make any non black make white
					else if (buf16[buf_width*y + x])
						buf_mask[byte+((buf_height*bit_width)/8)] |= 1<<bit;
				}
			}
		}

		// Create an icon for our cursor
		ICONINFO iconinfo;
		iconinfo.fIcon = FALSE;
		iconinfo.xHotspot = f->xoff;
		iconinfo.yHotspot = f->yoff;
		iconinfo.hbmMask = CreateBitmap(buf_width, buf_height, 1, 1, buf_mask);
		iconinfo.hbmColor = CreateBitmap(buf_width, buf_height, 1, bpp, buf);

		hwcursors[frame].hCursor = CreateIconIndirect (&iconinfo);

		DeleteObject(iconinfo.hbmMask);
		DeleteObject(iconinfo.hbmColor);

		delete [] buf;
		delete [] buf_mask;
		delete surf;
	}

	// Lets screw with the window class
	SDL_SysWMinfo info;
	info.version.major = SDL_MAJOR_VERSION;
	info.version.minor = SDL_MINOR_VERSION;
	info.version.patch = SDL_PATCHLEVEL;
	SDL_GetWMInfo(&info);
	oldWindowProc = (WNDPROC) GetWindowLongPtr(info.window, GWLP_WNDPROC);
	SetWindowLongPtr(info.window, GWLP_WNDPROC, (LONG) myWindowProc);
	
}
#endif

// Paint the screen
void GUIApp::paint()
{
	if(!runGraphicSysInit) // need to worry if the graphics system has been started. Need nicer way.
		return;

	painting = true;

	// Begin painting
	screen->BeginPainting();

	// We need to get the dims
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);

	long before_gumps = SDL_GetTicks();
	desktopGump->Paint(screen, lerpFactor);
	long after_gumps = SDL_GetTicks();

	// Mouse
	if (gamedata) {
		Shape* mouse = gamedata->getMouse();
		if (mouse) {
			int frame = getMouseFrame();
			if (frame >= 0)
			{
#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)
				if (allow_set_cursor) SetCursor(hwcursors[frame].hCursor);
#else
				screen->Paint(mouse, frame, mouseX, mouseY);
#endif
			}
		}
	}

	static long prev = 0;
	long now = SDL_GetTicks();
	long diff = now - prev;
	if (diff == 0) diff = 1;
	prev = now;

	char buf[256];
	Texture *confont = con.GetConFont();
	int v_offset = 0;
	int char_w = confont->width/16;

	if (drawRenderStats)
	{
		snprintf(buf, 255, "Rendering time %li ms %li FPS ", diff, 1000/diff);
		screen->PrintTextFixed(confont, buf, dims.w-char_w*strlen(buf), v_offset);
		v_offset += confont->height/16;

		snprintf(buf, 255, "Paint Gumps %li ms ", after_gumps-before_gumps);
		screen->PrintTextFixed(confont, buf, dims.w-char_w*strlen(buf), v_offset);
		v_offset += confont->height/16;

		snprintf(buf, 255, "t %02d:%02d gh %i ", I_getTimeInMinutes(0,0), I_getTimeInSeconds(0,0)%60, I_getTimeInGameHours(0,0));
		screen->PrintTextFixed(confont, buf, dims.w-char_w*strlen(buf), v_offset);
		v_offset += confont->height/16;
	}

	// End painting
	screen->EndPainting();

	painting = false;
}

bool GUIApp::isMouseDown(MouseButton button)
{
	return (mouseButton[button].state & MBS_DOWN);
}

int GUIApp::getMouseLength(int mx, int my)
{
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);
	// For now, reference point is (near) the center of the screen
	int dx = mx - dims.w/2;
	int dy = (dims.h/2+14) - my; //! constant

	int shortsq = (dims.w / 8);
	if (dims.h / 6 < shortsq)
		shortsq = (dims.h / 6);
	shortsq = shortsq*shortsq;
	
	int mediumsq = ((dims.w * 4) / 10);
	if (((dims.h * 4) / 10) < mediumsq)
		mediumsq = ((dims.h * 4) / 10);
	mediumsq = mediumsq * mediumsq;
	
	int dsq = dx*dx+dy*dy;
	
	// determine length of arrow
	if (dsq <= shortsq) {
		return 0;
	} else if (dsq <= mediumsq) {
		return 1;
	} else {
		return 2;
	}
}

int GUIApp::getMouseDirection(int mx, int my)
{
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);
	// For now, reference point is (near) the center of the screen
	int dx = mx - dims.w/2;
	int dy = (dims.h/2+14) - my; //! constant

	return ((Get_direction(dy*2, dx))+1)%8;
}

int GUIApp::getMouseFrame()
{
	// Ultima 8 mouse cursors:

	// 0-7 = short (0 = up, 1 = up-right, 2 = right, ...)
	// 8-15 = medium
	// 16-23 = long
	// 24 = blue dot
	// 25-32 = combat
	// 33 = red dot
	// 34 = target
	// 35 = pentagram
	// 36 = skeletal hand
	// 38 = quill
	// 39 = magnifying glass
	// 40 = red cross

	MouseCursor cursor = cursors.top();
	switch (cursor) {
	case MOUSE_NORMAL:
	{
		bool combat = false;
		MainActor* av = World::get_instance()->getMainActor();
		if (av) { combat = av->isInCombat(); }

		// Calculate frame based on direction
		int frame = getMouseDirection(mouseX, mouseY);

		/** length --- frame offset
		 *    0              0
		 *    1              8
		 *    2             16
		 *  combat          25
		 **/
		int offset = getMouseLength(mouseX, mouseY) * 8;
		if (combat && offset != 16) //combat mouse is off if running
			offset = 25;
		return frame + offset;
	}
	//!! constants...
	case MOUSE_NONE: return -1;
	case MOUSE_TARGET: return 34;
	case MOUSE_PENTAGRAM: return 35;
	case MOUSE_HAND: return 36;
	case MOUSE_QUILL: return 38;
	case MOUSE_MAGGLASS: return 39;
	case MOUSE_CROSS: return 40;
	default: return -1;
	}

}

void GUIApp::setMouseCursor(MouseCursor cursor)
{
	cursors.pop();
	cursors.push(cursor);
}

void GUIApp::pushMouseCursor()
{
	cursors.push(MOUSE_NORMAL);
}

void GUIApp::popMouseCursor()
{
	cursors.pop();
}

void GUIApp::GraphicSysInit()
{
	// if we've already done this...
	if(runGraphicSysInit) return;
	//else...

#ifdef USE_SDLTTF
	TTF_Init();
	atexit(TTF_Quit);
	//HACK;
//	IDataSource* fontids = FileSystem::get_instance()->ReadFile("@data/verdanab.ttf");
	IDataSource* fontids = FileSystem::get_instance()->ReadFile("@data/VeraBd.ttf");
	if (fontids)
	  ttffont = new TTFont(fontids, 0xFFFFFF, 10);
	else {
	  ttffont = 0;
	  perr << "Warning: unable to open @data/VeraBd.ttf" << std::endl;
	}
#endif

	// Set Screen Resolution
	pout << "Set Video Mode" << std::endl;

	settingman->setDefault("fullscreen", false);
	settingman->setDefault("width", 640);
	settingman->setDefault("height", 480);
	settingman->setDefault("bpp", 32);

	bool fullscreen;
	int width, height, bpp;
	settingman->get("fullscreen", fullscreen);
	settingman->get("width", width);
	settingman->get("height", height);
	settingman->get("bpp", bpp);

#ifdef UNDER_CE
	width = 240;
	height = 320;
#endif

#if 0
	// store values in user's config file
	settingman->set("width", width);
	settingman->set("height", height);
	settingman->set("bpp", bpp);
	settingman->set("fullscreen", fullscreen);
#endif

	screen = RenderSurface::SetVideoMode(width, height, bpp,
										 fullscreen, false);

	if (!screen)
	{
		perr << "Unable to set video mode. Exiting" << std::endl;
		std::exit(-1);
	}

	pout << "Create Desktop" << std::endl;
	desktopGump = new DesktopGump(0,0, width, height);
	desktopGump->InitGump();
	desktopGump->MakeFocus();

	pout << "Create Graphics Console" << std::endl;
	consoleGump = new ConsoleGump(0, 0, width, height);
	consoleGump->InitGump();
	desktopGump->AddChild(consoleGump);

	LoadConsoleFont();

	// Create desktop
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);

	runGraphicSysInit=true;

	// setup normal mouse cursor
	pushMouseCursor();

	// Do initial console paint
	paint();
}

void GUIApp::LoadConsoleFont()
{
#if defined(WIN32) && defined(COLOURLESS_WANTS_A_DATA_FREE_PENATGRAM)
	HRSRC res = FindResource(NULL,  MAKEINTRESOURCE(IDR_FIXED_FONT_TGA), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.tga", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));

	res = FindResource(NULL, MAKEINTRESOURCE(IDR_FIXED_FONT_INI), RT_RCDATA);
	if (res) filesystem->MountFileInMemory("@data/fixedfont.ini", static_cast<uint8*>(LockResource(LoadResource(NULL, res))), SizeofResource(NULL, res));
#endif

	std::string data;
	std::string confontfile;
	std::string confontini("@data/fixedfont.ini");

	pout << "Searching for alternate console font... ";
	if (settingman->get("console_font", data)) {
		confontini = data;
		pout << "Found." << std::endl;
	}
	else
		pout << "Not Found." << std::endl;

	// try to load the file
	pout << "Loading console font config: " << confontini << "... ";
	if(configfileman->readConfigFile(confontini, "font", true))
		pout << "Ok" << std::endl;
	else
		pout << "Failed" << std::endl;

	// search for the path to the font...
	if (!configfileman->get("font/font/path", confontfile))
	{
		pout << "Error: Console font path not found! Unable to continue. "
			 << "Exiting." << std::endl;
		std::exit(-1);
	}

	// Load confont
	pout << "Loading Confont: " << confontfile << std::endl;
	IDataSource *cf = filesystem->ReadFile(confontfile.c_str());
	Texture *confont;
	if (cf) confont = Texture::Create(*cf, confontfile.c_str());
	else confont = 0;
	if (!confont)
	{
		perr << "Unable to load " << confontfile << ". Exiting" << std::endl;
		std::exit(-1);
	}
	delete cf;

	con.SetConFont(confont);
}

//
// Hacks are us!
//

class QuickAvatarMoverProcess : public Process
{
	int dx, dy, dz, dir;
public:
	static  QuickAvatarMoverProcess	*amp[6];
	static	bool				clipping;
	static	bool				quarter;
	static	bool				hitting;

	QuickAvatarMoverProcess(int x, int y, int z, int _dir) : Process(1), dx(x), dy(y), dz(z), dir(_dir)
	{
		if (amp[dir]) amp[dir]->terminate();
		amp[dir] = this;
	}

	virtual bool run(const uint32 framenum)
	{
		if (GUIApp::get_instance()->isAvatarInStasis()) 
		{
			terminate();
			return false;
		}
		Actor* avatar = World::get_instance()->getNPC(1);
		sint32 x,y,z;
		avatar->getLocation(x,y,z);
		sint32 ixd,iyd,izd;
		avatar->getFootpadWorld(ixd, iyd, izd);

		CurrentMap* cm = World::get_instance()->getCurrentMap();

		sint32 dx = this->dx;
		sint32 dy = this->dy;
		sint32 dz = this->dz;

		for (int j = 0; j < 3; j++)
		{
			dx = this->dx;
			dy = this->dy;
			dz = this->dz;

			if (j == 1) dx = 0;
			else if (j == 2) dy = 0;

			if (quarter) 
			{
				dx /= 4;
				dy /= 4;
				dz /= 4;
			}

			bool ok = false;

			while (dx || dy || dz) {

				if (!clipping || cm->isValidPosition(x+dx,y+dy,z+dz,ixd,iyd,izd,1,0,0))
				{
					if (clipping && !dz)
					{
						if (cm->isValidPosition(x+dx,y+dy,z-8,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-8,ixd,iyd,izd,1,0,0))
						{
							dz = -8;
						}
						else if (cm->isValidPosition(x+dx,y+dy,z-16,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-16,ixd,iyd,izd,1,0,0))
						{
							dz = -16;
						}
						else if (cm->isValidPosition(x+dx,y+dy,z-24,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-24,ixd,iyd,izd,1,0,0))
						{
							dz = -24;
						}
						else if (cm->isValidPosition(x+dx,y+dy,z-32,ixd,iyd,izd,1,0,0) &&
								!cm->isValidPosition(x,y,z-32,ixd,iyd,izd,1,0,0))
						{
							dz = -32;
						}
					}					
					ok = true;
					break;
				}
				else if (cm->isValidPosition(x+dx,y+dy,z+dz+8,ixd,iyd,izd,1,0,0))
				{
					dz+=8;
					ok = true;
					break;
				}
				dx/=2;
				dy/=2;
				dz/=2;
			}
			if (ok) break;
		}

#if 0
		if (hitting) 
		{
			std::list<CurrentMap::SweepItem> items;

			sint32 start[3] = { x, y, z };
			sint32 end[3] = { x+dx, y+dy, z+dz };
			sint32 dims[3] = { ixd, iyd, izd };
			uint16 skip = 0;

			bool hit = cm->sweepTest(start,end,dims,1,false,&items);

			if (hit)
			{
				std::list<CurrentMap::SweepItem>::iterator it;

				for (it = items.begin(); it != items.end(); it++)
				{
					Item *item = World::get_instance()->getItem((*it).item);

					uint16 proc_gothit = 0, proc_rel = 0;

					// If hitting at start, we should have already 
					// called gotHit
					if ((*it).hit_time > 0) 
					{
						item->setExtFlag(Item::EXT_HIGHLIGHT);
						proc_gothit = item->callUsecodeEvent_gotHit(1,0);
					}

					// If not hitting at end, we will need to call release
					if ((*it).end_time < 0x4000)
					{
						item->clearExtFlag(Item::EXT_HIGHLIGHT);
						proc_rel = item->callUsecodeEvent_release();
					}

					// We want to make sure that release is called AFTER gotHit.
					if (proc_rel && proc_gothit)
					{
						Process *p = Kernel::get_instance()->getProcess(proc_rel);
						p->waitFor(proc_gothit);
					}
				}
			}
		}
#endif

		// Yes, i know, not entirely correct
		avatar->collideMove(x+dx,y+dy,z+dz, false, true);
		return true;
	}

	virtual void terminate()
	{
		Process::terminate();
		amp[dir] = 0;
	}
};

QuickAvatarMoverProcess	*QuickAvatarMoverProcess::amp[6] = { 0, 0, 0, 0, 0, 0 };
bool QuickAvatarMoverProcess::clipping = false;
bool QuickAvatarMoverProcess::quarter = false;
bool QuickAvatarMoverProcess::hitting = false;

bool hitting() { return QuickAvatarMoverProcess::hitting; }

static int volumelevel = 255;

void GUIApp::enterTextMode(Gump *gump)
{
	if (!textmodes.empty()) textmodes.remove(gump->getObjId());
	else SDL_EnableUNICODE(1);
	textmodes.push_front(gump->getObjId());
}

void GUIApp::leaveTextMode(Gump *gump)
{
	if (textmodes.empty()) return;
	textmodes.remove(gump->getObjId());
	if (textmodes.empty()) SDL_EnableUNICODE(0);
}

void GUIApp::handleEvent(const SDL_Event& event)
{
	uint32 now = SDL_GetTicks();
	bool handled = false;

	// Text mode input. A few hacks here
	if (!textmodes.empty() &&
		(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
		event.key.keysym.sym != SDLK_BACKQUOTE) {

		Gump *gump = 0;

		while (!textmodes.empty())
		{
			gump = p_dynamic_cast<Gump*>(objectmanager->getObject(textmodes.front()));
			if (gump) break;

			textmodes.pop_front();
		}

		if (gump) {

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.unicode >= ' ' &&
					event.key.keysym.unicode <= 255)
				{
					gump->OnTextInput(event.key.keysym.unicode);
				}

				gump->OnKeyDown(event.key.keysym.sym, event.key.keysym.mod);
			} else {
				gump->OnKeyUp(event.key.keysym.sym);
			}

			return;
		}
	}
	
	// Old style input begins here
	switch (event.type) {

	//!! TODO: handle mouse handedness. (swap left/right mouse buttons here)

	case SDL_QUIT:
	{
		isRunning = false;
	}
	break;

	case SDL_ACTIVEEVENT:
	{
		// pause when lost focus?
	}
	break;


	// most of these events will probably be passed to a gump manager,
	// since almost all (all?) user input will be handled by a gump

	case SDL_MOUSEBUTTONDOWN:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		Gump *mousedowngump = desktopGump->OnMouseDown(button, mx, my);
		if (mousedowngump)
		{
			mouseButton[button].downGump = mousedowngump->getObjId();
			handled = true;
		}
		else
			mouseButton[button].downGump = 0;

		mouseButton[button].curDown = now;
		mouseButton[button].downX = mx;
		mouseButton[button].downY = my;
		mouseButton[button].state |= MBS_DOWN;
		mouseButton[button].state &= ~MBS_HANDLED;

		if (now - mouseButton[button].lastDown < DOUBLE_CLICK_TIMEOUT) {
			if (dragging == DRAG_NOT) {
				Gump* gump = getGump(mouseButton[button].downGump);
				if (gump)
					gump->OnMouseDouble(button, mx, my);
				mouseButton[button].state |= MBS_HANDLED;
			}
		}
		mouseButton[button].lastDown = now;
	}
	break;

	case SDL_MOUSEBUTTONUP:
	{
		int button = event.button.button;
		int mx = event.button.x;
		int my = event.button.y;
		assert(button >= 0 && button <= NUM_MOUSEBUTTONS);

		mouseButton[button].state &= ~MBS_DOWN;
		
		// Need to store the last down position of the mouse
		// when the button is released.
		mouseButton[button].downX = mx;
		mouseButton[button].downY = my;

		if (button == BUTTON_LEFT && dragging != DRAG_NOT) {
			stopDragging(mx, my);
			handled = true;
			break;
		}

		if (dragging == DRAG_NOT) {
			Gump* gump = getGump(mouseButton[button].downGump);
			if (gump)
			{
				gump->OnMouseUp(button, mx, my);
				handled = true;
			}
		}
	}
	break;

	case SDL_MOUSEMOTION:
	{
		int mx = event.button.x;
		int my = event.button.y;
		mouseX = mx; mouseY = my;
		if (dragging == DRAG_NOT) {
			if (mouseButton[BUTTON_LEFT].state & MBS_DOWN) {
				int startx = mouseButton[BUTTON_LEFT].downX;
				int starty = mouseButton[BUTTON_LEFT].downY;
				if (abs(startx - mx) > 2 ||
					abs(starty - my) > 2)
				{
					startDragging(startx, starty);
				}
			}
		}

		if (dragging == DRAG_OK || dragging == DRAG_TEMPFAIL) {
			moveDragging(mx, my);
		}
	}
	break;

	case SDL_KEYDOWN:
	{
		if (dragging != DRAG_NOT) break;

		switch (event.key.keysym.sym) {
			case SDLK_LSHIFT: 
			case SDLK_RSHIFT: {
				QuickAvatarMoverProcess::quarter = true;
				pout << "QuickAvatarMoverProcess::quarter = " << QuickAvatarMoverProcess::quarter << std::endl; 
			} break;
			case SDLK_c: {
				QuickAvatarMoverProcess::clipping = !QuickAvatarMoverProcess::clipping;
				pout << "QuickAvatarMoverProcess::clipping = " << QuickAvatarMoverProcess::clipping << std::endl; 
			} break;
			case SDLK_h: {
				QuickAvatarMoverProcess::hitting = !QuickAvatarMoverProcess::hitting;
				pout << "QuickAvatarMoverProcess::hitting = " << QuickAvatarMoverProcess::hitting << std::endl; 
			} break;
			case SDLK_UP: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(-64,-64,0,0);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_DOWN: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(+64,+64,0,1);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_LEFT: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(-64,+64,0,2);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_RIGHT: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(+64,-64,0,3);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_HOME: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(0,0,8,4);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_END: {
				if (!avatarInStasis) { 
					Process *p = new QuickAvatarMoverProcess(0,0,-8,5);
					Kernel::get_instance()->addProcess(p);
				} else { 
					pout << "Can't: avatarInStasis" << std::endl; 
				}
			} break;
			case SDLK_KP_PLUS: {
				midi_volume+=8;
				if (midi_volume>255) midi_volume =255;
				pout << "Midi Volume is now: " << midi_volume << std::endl; 
				if (midi_driver) midi_driver->setGlobalVolume(midi_volume);
			} break;
			case SDLK_KP_MINUS: {
				midi_volume-=8;
				if (midi_volume<0) midi_volume = 0;
				pout << "Midi Volume is now: " << midi_volume << std::endl; 
				if (midi_driver) midi_driver->setGlobalVolume(midi_volume);
			} break;
			default:
				break;
		}
	}
	break;

	case SDL_KEYUP:
	{
		if (dragging != DRAG_NOT) break;

		switch (event.key.keysym.sym) {
		case SDLK_q: // Quick quit
		{
			if (event.key.keysym.mod & KMOD_CTRL)
				ForceQuit();
		} break;
		case SDLK_LSHIFT: 
		case SDLK_RSHIFT: {
			QuickAvatarMoverProcess::quarter = false;
			pout << "QuickAvatarMoverProcess::quarter = " << QuickAvatarMoverProcess::quarter << std::endl; 
		} break;
		case SDLK_UP: {
			if (QuickAvatarMoverProcess::amp[0]) QuickAvatarMoverProcess::amp[0]->terminate();
		} break;
		case SDLK_DOWN: {
			if (QuickAvatarMoverProcess::amp[1]) QuickAvatarMoverProcess::amp[1]->terminate();
		} break;
		case SDLK_LEFT: {
			if (QuickAvatarMoverProcess::amp[2]) QuickAvatarMoverProcess::amp[2]->terminate();
		} break;
		case SDLK_RIGHT: {
			if (QuickAvatarMoverProcess::amp[3]) QuickAvatarMoverProcess::amp[3]->terminate();
		} break;
		case SDLK_HOME: {
			if (QuickAvatarMoverProcess::amp[4]) QuickAvatarMoverProcess::amp[4]->terminate();
		} break;
		case SDLK_END: {
			if (QuickAvatarMoverProcess::amp[5]) QuickAvatarMoverProcess::amp[5]->terminate();
		} break;

		case SDLK_LEFTBRACKET: gameMapGump->IncSortOrder(-1); break;
		case SDLK_RIGHTBRACKET: gameMapGump->IncSortOrder(+1); break;

		case SDLK_KP1: case SDLK_KP2: case SDLK_KP3:
		case SDLK_KP4: case SDLK_KP6: case SDLK_KP7:
		case SDLK_KP8: case SDLK_KP9: { // quick animation test
			if (!avatarInStasis) { 
				int dir;
				Animation::Sequence action = Animation::walk;
				if (event.key.keysym.mod & KMOD_LSHIFT) action = Animation::run;
				if (event.key.keysym.mod & KMOD_LCTRL) action = Animation::step;
				if (event.key.keysym.mod & KMOD_LALT) action = Animation::jump;
				switch (event.key.keysym.sym) {
				case SDLK_KP1: dir=4; break;
				case SDLK_KP2: dir=3; break;
				case SDLK_KP3: dir=2; break;
				case SDLK_KP4: dir=5; break;
				case SDLK_KP6: dir=1; break;
				case SDLK_KP7: dir=6; break;
				case SDLK_KP8: dir=7; break;
				case SDLK_KP9: dir=0; break;
				default: dir=0; break;
				}
                MainActor* av = World::get_instance()->getMainActor();
				if (!av->tryAnim(action, dir)) {
					perr << "tryAnim: failed!" << std::endl;
				}
				av->doAnim(action, dir);
			} else { 
				pout << "Can't: avatarInStasis" << std::endl; 
			} 
		} break;
		default: break;
		}
	}
	break;

	// any more useful events?

	default:
		break;
	}

	if (dragging == DRAG_NOT && ! handled) {

		HIDBinding binding = hidmanager->getBinding(event);

		if (binding) {
			handled = binding(event);
			if (handled)
				return;
		}
	}

}

void GUIApp::handleDelayedEvents()
{
	uint32 now = SDL_GetTicks();
	for (int button = 0; button <= NUM_MOUSEBUTTONS; ++button) {
		if (!(mouseButton[button].state & (MBS_HANDLED | MBS_DOWN)) &&
			now - mouseButton[button].lastDown > DOUBLE_CLICK_TIMEOUT)
		{
			Gump* gump = getGump(mouseButton[button].downGump);
			if (gump)
				gump->OnMouseClick(button, mouseButton[button].downX,
								   mouseButton[button].downY);

			mouseButton[button].downGump = 0;
			mouseButton[button].state |= MBS_HANDLED;
		}
	}

}

void GUIApp::startDragging(int startx, int starty)
{
	dragging_objid = desktopGump->TraceObjId(startx, starty);
	
	Gump *gump = getGump(dragging_objid);
	Item *item= World::get_instance()->getItem(dragging_objid);
	
	// for a Gump, notify the Gump's parent that we started
	// dragging:
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int px = startx, py = starty;
		parent->ScreenSpaceToGump(px, py);
		if (parent->StartDraggingChild(gump, px, py))
			dragging = DRAG_OK;
		else {
			dragging_objid = 0;
			return;
		}
	} else
	// for an Item, notify the gump the item is in that we started dragging
	if (item) {
		// find gump item was in
		gump = desktopGump->FindGump(startx, starty);
		int gx = startx, gy = starty;
		gump->ScreenSpaceToGump(gx, gy);
		bool ok = !isAvatarInStasis() &&
			gump->StartDraggingItem(item,gx,gy);
		if (!ok) {
			dragging = DRAG_INVALID;
		} else {
			dragging = DRAG_OK;
			
			// this is the gump that'll get StopDraggingItem
			dragging_item_startgump = gump->getObjId();
			
			// this is the gump the item is currently over
			dragging_item_lastgump = gump->getObjId();
		}
	} else {
		dragging = DRAG_INVALID;
	}

	Object* obj = ObjectManager::get_instance()->getObject(dragging_objid);
	perr << "Dragging object " << dragging_objid << " (class=" << (obj ? obj->GetClassType().class_name : "NULL") << ")" << std::endl;
	pushMouseCursor();
	setMouseCursor(MOUSE_NORMAL);
	
	// pause the kernel
	kernel->pause();
	
	mouseButton[BUTTON_LEFT].state |= MBS_HANDLED;

	if (dragging == DRAG_INVALID) {
		setMouseCursor(MOUSE_CROSS);
	}
}

void GUIApp::moveDragging(int mx, int my)
{
	Gump* gump = getGump(dragging_objid);
	Item *item= World::get_instance()->getItem(dragging_objid);

	setMouseCursor(MOUSE_NORMAL);
	
	// for a gump, notify Gump's parent that it was dragged
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		int px = mx, py = my;
		parent->ScreenSpaceToGump(px, py);
		parent->DraggingChild(gump, px, py);
	} else
	// for an item, notify the gump it's on
	if (item) {
		gump = desktopGump->FindGump(mx, my);
		assert(gump);
			
		if (gump->getObjId() != dragging_item_lastgump) {
			// item switched gump, so notify previous gump item left
			Gump *last = getGump(dragging_item_lastgump);
			if (last) last->DraggingItemLeftGump(item);
		}
		dragging_item_lastgump = gump->getObjId();
		int gx = mx, gy = my;
		gump->ScreenSpaceToGump(gx, gy);
		bool ok = gump->DraggingItem(item,gx,gy);
		if (!ok) {
			dragging = DRAG_TEMPFAIL;
		} else {
			dragging = DRAG_OK;
		}
	} else {
		CANT_HAPPEN();
	}

	if (dragging == DRAG_TEMPFAIL) {
		setMouseCursor(MOUSE_CROSS);
	}
}


void GUIApp::stopDragging(int mx, int my)
{
	perr << "Dropping object " << dragging_objid << std::endl;
	
	Gump *gump = getGump(dragging_objid);
	Item *item= World::get_instance()->getItem(dragging_objid);
	// for a Gump: notify parent
	if (gump) {
		Gump *parent = gump->GetParent();
		assert(parent); // can't drag root gump
		parent->StopDraggingChild(gump);
	} else 
	// for an item: notify gumps
	if (item) {
		if (dragging != DRAG_INVALID) {
			Gump *startgump = getGump(dragging_item_startgump);
			assert(startgump); // can't have disappeared
			bool moved = (dragging == DRAG_OK);
			startgump->StopDraggingItem(item, moved);
		}
		
		if (dragging == DRAG_OK) {
			gump = desktopGump->FindGump(mx, my);
			int gx = mx, gy = my;
			gump->ScreenSpaceToGump(gx, gy);
			gump->DropItem(item,gx,gy);
		}
	} else {
		assert(dragging == DRAG_INVALID);
	}

	dragging = DRAG_NOT;

	kernel->unpause();

	popMouseCursor();
}

bool GUIApp::saveGame(std::string filename)
{
	pout << "Saving..." << std::endl;

	ODataSource* ods = filesystem->WriteFile(filename);
	if (!ods) return false;

	SavegameWriter* sgw = new SavegameWriter(ods);
	sgw->start(10); // 9 files + version
	sgw->writeVersion(1);

	// We'll make it 2KB intially
	OAutoBufferDataSource buf(2048);

	kernel->save(&buf);
	sgw->writeFile("KERNEL", &buf);
	buf.clear();

	objectmanager->save(&buf);
	sgw->writeFile("OBJECTS", &buf);
	buf.clear();

	world->save(&buf);
	sgw->writeFile("WORLD", &buf);
	buf.clear();

	world->saveMaps(&buf);
	sgw->writeFile("MAPS", &buf);
	buf.clear();

	world->getCurrentMap()->save(&buf);
	sgw->writeFile("CURRENTMAP", &buf);
	buf.clear();

	ucmachine->saveStrings(&buf);
	sgw->writeFile("UCSTRINGS", &buf);
	buf.clear();

	ucmachine->saveGlobals(&buf);
	sgw->writeFile("UCGLOBALS", &buf);
	buf.clear();

	ucmachine->saveLists(&buf);
	sgw->writeFile("UCLISTS", &buf);
	buf.clear();

	save(&buf);
	sgw->writeFile("APP", &buf);
	buf.clear();

	sgw->finish();
	delete sgw;

	pout << "Done" << std::endl;

	return true;
}

bool GUIApp::loadGame(std::string filename)
{
	pout << "Loading..." << std::endl;

	IDataSource* ids = filesystem->ReadFile(filename);
	if (!ids) {
		perr << "Can't find file: " << filename << std::endl;
		return false;
	}

	Savegame* sg = new Savegame(ids);
	int version = sg->getVersion();
	if (version != 1) {
		perr << "Unsupported savegame version (" << version << ")"
			 << std::endl;
		return false;
	}

	// kill music
	if (midi_driver) {
		for (int i = 0; i < midi_driver->maxSequences(); i++) {
			midi_driver->finishSequence(i);
		}
	}

	// now, reset everything (order matters)
	world->reset();
	ucmachine->reset();
	// ObjectManager, Kernel have to be last, because they kill
	// all processes/objects
	objectmanager->reset();
	kernel->reset();
	textmodes.clear();

	bool ok, totalok = true;

	// TODO: reset mouse state in GUIApp

 	// and load everything back (order matters)
	IDataSource* ds;


	// UCSTRINGS, UCGLOBALS, UCLISTS don't depend on anything else,
	// so load these first
	ds = sg->get_datasource("UCSTRINGS");
	ok = ucmachine->loadStrings(ds);
	totalok &= ok;
	perr << "UCSTRINGS: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("UCGLOBALS");
	ok = ucmachine->loadGlobals(ds);
	totalok &= ok;
	perr << "UCGLOBALS: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("UCLISTS");
	ok = ucmachine->loadLists(ds);
	totalok &= ok;
	perr << "UCLISTS: " << (ok ? "ok" : "failed")<< std::endl;
	delete ds;

	// KERNEL must be before OBJECTS, for the egghatcher
	// KERNEL must be before APP, for the avatarMoverProcess
	ds = sg->get_datasource("KERNEL");
	ok = kernel->load(ds);
	totalok &= ok;
	perr << "KERNEL: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("APP");
	ok = load(ds);
	totalok &= ok;
	perr << "APP: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	// WORLD must be before OBJECTS, for the egghatcher
	ds = sg->get_datasource("WORLD");
	ok = world->load(ds);
	totalok &= ok;
	perr << "WORLD: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("CURRENTMAP");
	ok = world->getCurrentMap()->load(ds);
	totalok &= ok;
	perr << "CURRENTMAP: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("OBJECTS");
	ok = objectmanager->load(ds);
	totalok &= ok;
	perr << "OBJECTS: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	ds = sg->get_datasource("MAPS");
	ok = world->loadMaps(ds);
	totalok &= ok;
	perr << "MAPS: " << (ok ? "ok" : "failed") << std::endl;
	delete ds;

	if (!ok) {
		perr << "Loading failed!" << std::endl;
		exit(1);
	}

	//!! temporary hack: reset desktopgump, gamemapgump, consolegump:
	desktopGump = 0;
	for (unsigned int i = 256; i < 65535; ++i) {
		DesktopGump* dg = p_dynamic_cast<DesktopGump*>(getGump(i));
		if (dg) {
			desktopGump = dg;
			break;
		}
	}
	assert(desktopGump);
	gameMapGump = p_dynamic_cast<GameMapGump*>(desktopGump->
											   FindGump(GameMapGump::ClassType));
	assert(gameMapGump);

	consoleGump = p_dynamic_cast<ConsoleGump*>(desktopGump->
											   FindGump(ConsoleGump::ClassType));
	assert(consoleGump);

	pout << "Done" << std::endl;

	delete sg;
	return false;
}

Gump* GUIApp::getGump(uint16 gumpid)
{
	return p_dynamic_cast<Gump*>(ObjectManager::get_instance()->
								 getObject(gumpid));
}

void GUIApp::save(ODataSource* ods)
{
	ods->write2(1); //version
	uint8 s = (avatarInStasis ? 1 : 0);
	ods->write1(s);
	ods->write4(static_cast<uint32>(timeOffset)); //!! FIXME
	ods->write2(avatarMoverProcess->getPid());

	Pentagram::Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) ods->write2(pal->matrix[i]);
	ods->write2(pal->transform);
}

bool GUIApp::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	avatarInStasis = (ids->read1() != 0);
	timeOffset = static_cast<sint32>(ids->read4());

	uint16 amppid = ids->read2();
	avatarMoverProcess = p_dynamic_cast<AvatarMoverProcess*>(Kernel::get_instance()->getProcess(amppid));

	sint16 matrix[12];
	for (int i = 0; i < 12; i++)
		matrix[i] = ids->read2();

	PaletteManager::get_instance()->transformPalette(PaletteManager::Pal_Game, matrix);
	Pentagram::Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	pal->transform = static_cast<PaletteManager::PalTransforms>(ids->read2());

	return true;
}

//
// Console Commands
//

void GUIApp::ConCmd_saveGame(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size()==1)
	{
		pout << "Usage: GUIApp::saveGame <filename>" << std::endl;
		return;
	}

	std::string filename = "@save/";
	filename += argv[1].c_str();
	GUIApp::get_instance()->saveGame(filename);
}

void GUIApp::ConCmd_loadGame(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size()==1)
	{
		pout << "Usage: GUIApp::loadGame <filename>" << std::endl;
		return;
	}

	std::string filename = "@save/";
	filename += argv[1].c_str();
	GUIApp::get_instance()->loadGame(filename);
}


void GUIApp::ConCmd_quit(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	GUIApp::get_instance()->isRunning = false;
}

void GUIApp::ConCmd_drawRenderStats(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size() == 1)
	{
		pout << "GUIApp::drawRenderStats = " << GUIApp::get_instance()->drawRenderStats << std::endl;
	}
	else
	{
		GUIApp::get_instance()->drawRenderStats = std::strtol(argv[1].c_str(), 0, 0) != 0;
	}
}

//
// Intrinsics
//

uint32 GUIApp::I_getCurrentTimerTick(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// number of ticks of a 60Hz timer, with the default animrate of 30Hz
	return Kernel::get_instance()->getFrameNum()*2;
}

uint32 GUIApp::I_setAvatarInStasis(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(statis);
	get_instance()->setAvatarInStasis(statis!=0);
	return 0;
}

uint32 GUIApp::I_getAvatarInStasis(const uint8* /*args*/, unsigned int /*argsize*/)
{
	if (get_instance()->avatarInStasis)
		return 1;
	else
		return 0;
}

uint32 GUIApp::I_getTimeInGameHours(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 game hour per every 27000 frames
	return (Kernel::get_instance()->getFrameNum()+get_instance()->timeOffset)/27000;
}

uint32 GUIApp::I_getTimeInMinutes(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 minute per every 1800 frames
	return (Kernel::get_instance()->getFrameNum()+get_instance()->timeOffset)/1800;
}

uint32 GUIApp::I_getTimeInSeconds(const uint8* /*args*/,
										unsigned int /*argsize*/)
{
	// 1 second per every 30 frames
	return (Kernel::get_instance()->getFrameNum()+get_instance()->timeOffset)/30;
}

uint32 GUIApp::I_setTimeInGameHours(const uint8* args,
										unsigned int /*argsize*/)
{
	ARG_UINT16(newhour);

	// 1 game hour per every 27000 frames
	sint32	absolute = newhour*27000;
	get_instance()->timeOffset = absolute-Kernel::get_instance()->getFrameNum();

	return 0;
}

