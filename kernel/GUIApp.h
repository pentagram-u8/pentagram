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
#include <stack>

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
class ObjectManager;
class MidiDriver;
class IDataSource;
class ODataSource;

const int NUM_MOUSEBUTTONS = 5;

class GUIApp : public CoreApp
{
public:
	ENABLE_RUNTIME_CLASSTYPE();
	
	GUIApp(int argc, const char* const* argv);
	virtual ~GUIApp();
	
	static GUIApp* get_instance()
		{ return p_dynamic_cast<GUIApp*>(application); }
	
	void startup();

	virtual void run();
	virtual void handleEvent(const SDL_Event& event);
	
	void U8Playground();
	virtual void paint();
	virtual bool isPainting() { return painting; }
	
	
	INTRINSIC(I_getCurrentTimerTick);
	INTRINSIC(I_setAvatarInStasis);
	INTRINSIC(I_getAvatarInStasis);
	INTRINSIC(I_getTimeInGameHours);
	INTRINSIC(I_getTimeInMinutes);
	INTRINSIC(I_getTimeInSeconds);
	INTRINSIC(I_setTimeInGameHours);

	void setAvatarInStasis(bool stat) { avatarInStasis = stat; }
	bool isAvatarInStasis() const { return avatarInStasis; }
	bool isPaintEditorItems() const { return paintEditorItems; }
	
	GameMapGump *getGameMapMapGump() { return gameMapGump; }
	Gump *getDesktopGump() { return desktopGump; }
	Gump* getGump(uint16 gumpid);

	//! save a game
	//! \param filename the file to save to
	//! \return true if succesful
	bool saveGame(std::string filename);

	//! load a game
	//! \param filename the savegame to load
	//! \return true if succesful.
	bool loadGame(std::string filename);

	MidiDriver* getMidiDriver() const { return midi_driver; }

	enum MouseCursor {
		MOUSE_NORMAL = 0,
		MOUSE_NONE = 1,
		MOUSE_TARGET = 2,
		MOUSE_PENTAGRAM = 3,
		MOUSE_HAND = 4,
		MOUSE_QUILL = 5,
		MOUSE_MAGGLASS = 6,
		MOUSE_CROSS = 7
	};

	//! set the current mouse cursor
	void setMouseCursor(MouseCursor cursor);

	//! push the current mouse cursor to the stack
	void pushMouseCursor();

	//! pop the last mouse cursor from the stack
	void popMouseCursor();

protected:
	virtual void DeclareArgs();

private:
	//! save CoreApp/GUIApp data
	void save(ODataSource* ods);

	//! load CoreApp/GUIApp data
	bool load(IDataSource* ids);
	
	// full system
	ObjectManager* objectmanager;
	UCMachine* ucmachine;
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
	
	void handleDelayedEvents();
	
	// Various dependancy flags
	bool runGraphicSysInit;
	bool runSDLInit;
	
	// Timing stuff
	sint32 lerpFactor;		//!< Interpolation factor for this frame (0-256)
	bool inBetweenFrame;	//!< Set true if we are doing an inbetween frame
	
	bool frameSkip;			//!< Set to true to enable frame skipping (default false)
	bool frameLimit;		//!< Set to true to enable frame limiting (default true)
	bool interpolate;		//!< Set to true to enable interpolation (default true)
	sint32 animationRate;	//!< The animation rate. Affects all processes! (default 100)
	
	// Sort of Camera Related Stuff, move somewhere else
	
	bool avatarInStasis;    //!< If this is set to true, Avatar can't move, 
	                        //!< nor can Avatar start more usecode
	bool paintEditorItems;  //!< If true, paint items with the SI_EDITOR flag
	
	bool painting;			//!< Set true when painting

	int mouseX, mouseY;
	bool mouseSet;			//!< mouse coordinates initialized?

	//! get the current mouse frame
	int getMouseFrame();
	std::stack<MouseCursor> cursors;
	
	static void	conAutoPaint(void);

	// mouse input state
	uint16 mouseDownGump[NUM_MOUSEBUTTONS+1];
	uint32 lastMouseDown[NUM_MOUSEBUTTONS+1];
	int mouseDownX[NUM_MOUSEBUTTONS+1], mouseDownY[NUM_MOUSEBUTTONS+1];
	int mouseState[NUM_MOUSEBUTTONS+1];
	enum MouseButtonState {
		MBS_DOWN = 0x1,
		MBS_HANDLED = 0x2
	};
	enum DraggingState {
		DRAG_NOT = 0,
		DRAG_OK = 1,
		DRAG_INVALID = 2,
		DRAG_TEMPFAIL = 3
	} dragging;
	uint16 dragging_objid;
	uint16 dragging_item_startgump;
	uint16 dragging_item_lastgump;

	void startDragging(int mx, int my);
	void moveDragging(int mx, int my);
	void stopDragging(int mx, int my);

	sint32 timeOffset;

	// Should probably go somewhere else.... perhaps in some 'audio manager'
	MidiDriver *midi_driver;
	int			midi_volume;
	void		init_midi();
	void		deinit_midi();
	static void	sdlAudioCallback(void *userdata, Uint8 *stream, int len);

public:
	enum MouseButton { //!! change this
		BUTTON_LEFT = 1,
		BUTTON_MIDDLE = 2,
		BUTTON_RIGHT = 3
	};
};

#endif
