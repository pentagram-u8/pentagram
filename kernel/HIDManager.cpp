/*
Copyright (C) 2003-2004 The Pentagram team

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
#include "HIDManager.h"

#include <string>
#include "stdbindings.h"
#include "u8bindings.h"
#include "GUIApp.h"
#include "SettingManager.h"

#include "ConfigFileManager.h" // temporary!

HIDManager* HIDManager::hidmanager = 0;

HIDManager::HIDManager()
{
	uint16 key, button, js;
	assert(hidmanager == 0);
	hidmanager = this;

	for (key=0; key < SDLK_LAST; ++key)
	{
		keybindings[key] = 0;
	}
	
	for (button=0; button < NUM_MOUSEBUTTONS+1; ++button)
	{
		mousebindings[button] = 0;
	}

	for (js=0; js < NUM_JOYSTICKS; ++js)
	{
		for (button=0; button < NUM_MOUSEBUTTONS; ++button)
		{
			joybindings[js][button] = 0;
		}
	}

	InitJoystick();

	bindingMap.insert( HIDBINDING_PAIR(quickSave) );
	bindingMap.insert( HIDBINDING_PAIR(quickLoad) );
	bindingMap.insert( HIDBINDING_PAIR(avatarInStatis) );
	bindingMap.insert( HIDBINDING_PAIR(engineStats) );
	bindingMap.insert( HIDBINDING_PAIR(paintEditorItems) );
	bindingMap.insert( HIDBINDING_PAIR(showTouchingItems) );
	bindingMap.insert( HIDBINDING_PAIR(itemLocator) );
	bindingMap.insert( HIDBINDING_PAIR(toggleCombat) );
	bindingMap.insert( HIDBINDING_PAIR(openInventory) );
	bindingMap.insert( HIDBINDING_PAIR(openBackpack) );
	bindingMap.insert( HIDBINDING_PAIR(recall) );
	bindingMap.insert( HIDBINDING_PAIR(useKeyring) );
	bindingMap.insert( HIDBINDING_PAIR(useBedroll) );
	bindingMap.insert( HIDBINDING_PAIR(runFirstEgg) );
	bindingMap.insert( HIDBINDING_PAIR(runExecutionEgg) );
	bindingMap.insert( HIDBINDING_PAIR(toggleFrameByFrame) );
	bindingMap.insert( HIDBINDING_PAIR(advanceFrameByFrame) );
	bindingMap.insert( HIDBINDING_PAIR(u8ShapeViewer) );
	bindingMap.insert( HIDBINDING_PAIR(showMenu) );
	bindingMap.insert( HIDBINDING_PAIR(quit) );
	bindingMap.insert( HIDBINDING_PAIR(toggleConsole) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveUp) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveDown) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveLeft) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveRight) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveAscend) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveDescend) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveQuarterSpeed) );
	bindingMap.insert( HIDBINDING_PAIR(quickMoveClipping) );
	keybindings[SDLK_ESCAPE] = &HIDBindings::quit;
	keybindings[SDLK_BACKQUOTE] = &HIDBindings::toggleConsole;

	HIDBindingMap::iterator i;
	Pentagram::istring conCmd;
	for (i = bindingMap.begin(); i != bindingMap.end(); ++i)
	{
		conCmd = "HIDBinding::";
		conCmd.append(i->first);
		con.AddConsoleCommand(conCmd, HIDManager::ConCmd_execBinding);
	}
}

HIDManager::~HIDManager()
{
	HIDBindingMap::iterator i;
	Pentagram::istring conCmd;
	for (i = bindingMap.begin(); i != bindingMap.end(); ++i)
	{
		conCmd = "HIDBinding::";
		conCmd.append(i->first);
		con.RemoveConsoleCommand(conCmd);
	}

	ShutdownJoystick();
	hidmanager = 0;
	bindingMap.clear();
}

HIDBinding HIDManager::getBinding(const SDL_Event& event)
{
	HIDBinding binding = 0;
	switch (event.type) {
	case SDL_KEYUP: case SDL_KEYDOWN:
	{
		uint16 key = event.key.keysym.sym;
		if (key < SDLK_LAST)
			binding = keybindings[key];
	}
	break;
	case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
	{
		uint16 button = event.button.button;
		if (button < NUM_MOUSEBUTTONS)
			binding = mousebindings[button];
	}
	break;
	case SDL_JOYBUTTONDOWN: case SDL_JOYBUTTONUP:
	{
		uint16 js = event.jbutton.which;
		uint16 button = event.jbutton.button;
		if (js < NUM_JOYSTICKS && button < NUM_JOYBUTTONS)
			binding = joybindings[js][button];
	}
	break;
	}

	return binding;
}

HIDBinding HIDManager::getBinding(const Pentagram::istring& bindingName)
{
	HIDBinding binding = 0;
	HIDBindingMap::iterator j = bindingMap.find(bindingName);
	if (j != bindingMap.end())
	{
		binding = j->second;
	}
	return binding;
}

void HIDManager::buildEvent(HID_Event& hidEvent, const SDL_Event& sdlEvent)
{
	hidEvent.xrel = 0;
	hidEvent.yrel = 0;
	hidEvent.value = 0;

	switch (sdlEvent.type) {
		case SDL_MOUSEBUTTONUP:
			hidEvent.type = HID_UP;
			hidEvent.device = HID_MOUSE;
			break;
		case SDL_KEYUP:
			hidEvent.type = HID_UP;
			hidEvent.device = HID_KEYBOARD;
			break;
		case SDL_JOYBUTTONUP:
			hidEvent.type = HID_UP;
			hidEvent.device = HID_JOYSTICK;
			break;
		case SDL_MOUSEBUTTONDOWN:
			hidEvent.type = HID_DOWN;
			hidEvent.device = HID_MOUSE;
			break;
		case SDL_KEYDOWN:
			hidEvent.type = HID_DOWN;
			hidEvent.device = HID_KEYBOARD;
			break;
		case SDL_JOYBUTTONDOWN:
			hidEvent.type = HID_DOWN;
			hidEvent.device = HID_JOYSTICK;
			break;
		case SDL_MOUSEMOTION:
			hidEvent.type = HID_MOTION;
			hidEvent.device = HID_MOUSE;
			hidEvent.xrel = sdlEvent.motion.xrel;
			hidEvent.yrel = sdlEvent.motion.yrel;
			break;
		case SDL_JOYAXISMOTION:
			hidEvent.type = HID_MOTION;
			hidEvent.device = HID_JOYSTICK;
			hidEvent.value = sdlEvent.jaxis.value;
			break;
		default:
			hidEvent.type = HID_UNHANDLED;
			hidEvent.device = HID_OTHER;
	}
}

void HIDManager::loadBindings()
{
	SettingManager* settings = SettingManager::get_instance();
	std::map<Pentagram::istring, std::string> keys;
	keys = settings->listDataValues("keys");

	std::map<Pentagram::istring, std::string>::iterator i = keys.begin();
	std::map<Pentagram::istring, std::string>::iterator end = keys.end();
	
	if (i == end)
	{
		pout << "No custom HIDBinding found:" << std::endl
			<< "\tLoading default HIDBindings..." << std::endl;
		ConfigFileManager* config = ConfigFileManager::get_instance();
		keys = config->listKeyValues("bindings/bindings");
		i = keys.begin();
		end = keys.end();
	}

	while (i != keys.end())
	{
		Pentagram::istring bindingName = i->second.c_str();
		bind((*i).first, bindingName);
		++i;
	}
	listBindings();
}

void HIDManager::saveBindings()
{
	uint16 key, button, js;
	SettingManager* settings = SettingManager::get_instance();
	Pentagram::istring section = "keys/";
	Pentagram::istring confkey;

	// first clear old bindings which are now unused
	for (key=0; key < SDLK_LAST; ++key)
	{
		if (keybindings[key] == 0)
		{
			confkey = section + SDL_GetKeyName((SDLKey) key);
			if (settings->exists(confkey))
			{
				settings->unset(confkey);
			}
		}
	}

	for (button=1; button < NUM_MOUSEBUTTONS+1; ++button)
	{
		if (mousebindings[button] == 0)
		{
			confkey = section + GetMouseButtonName((MouseButton) button);
			if (settings->exists(confkey))
			{
				settings->unset(confkey);
			}
		}
	}

	for (js=0; js < NUM_JOYSTICKS; ++js)
	{
		for (button=0; button < NUM_MOUSEBUTTONS; ++button)
		{
			confkey = section + GetJoystickButtonName(js, button);
			if (settings->exists(confkey))
			{
				settings->unset(confkey);
			}
		}
	}

	HIDBindingMap::iterator i;
	for (i = bindingMap.begin(); i != bindingMap.end(); ++i)
	{
		for (key=0; key < SDLK_LAST; ++key)
		{
			if (keybindings[key] == i->second)
			{
				confkey = section + SDL_GetKeyName((SDLKey) key);
				settings->set(confkey, i->first);
			}
		}

		for (button=1; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			if (mousebindings[button] == i->second)
			{
				confkey = section + GetMouseButtonName((MouseButton) button);
				settings->set(confkey, i->first);
			}
		}

		for (js=0; js < NUM_JOYSTICKS; ++js)
		{
			for (button=0; button < NUM_MOUSEBUTTONS; ++button)
			{
				if (joybindings[js][button] == i->second)
				{
					confkey = section + GetJoystickButtonName(js, button);
					settings->set(confkey, i->first);
				}
			}
		}
	}
}

void HIDManager::bind(const Pentagram::istring& control, const Pentagram::istring& bindingName)
{
	uint16 key, button, js;
	const char * name = 0;
	HIDBindingMap::iterator j = bindingMap.find(bindingName);

	if (j != bindingMap.end())
	{

		for (key=0; key < SDLK_LAST; ++key)
		{
			if (key == SDLK_ESCAPE || key == SDLK_BACKQUOTE)
			{	// We will not allow these keys to be rebound
				++key; 
			}
			name = SDL_GetKeyName((SDLKey) key);
			if (control == name)
			{
				keybindings[key] = j->second;

				// We found the matching SDLKey. Stop searching;
				return;
			}
		}

		// Only bind Mouse 2 and up for now
		for (button=2; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			name = GetMouseButtonName((MouseButton) button);
			if (control == name)
			{
				mousebindings[button] = j->second;

				// We found the matching Mouse Button. Stop searching;
				return;
			}
		}

		for (js=0; js < NUM_JOYSTICKS; ++js)
		{
			for (button=0; button < NUM_MOUSEBUTTONS; ++button)
			{
				name = GetJoystickButtonName(js, button);
				if (control == name)
				{
					joybindings[js][button] = j->second;

					//We found the matching Joystick Button. Stop searching;
					return;
				}
			}
		}
	}
}

void HIDManager::unbind(const Pentagram::istring& control)
{
	uint16 key, button, js;
	const char * name = 0;
	HIDBindingMap::iterator j = bindingMap.find(control);
	if (j != bindingMap.end())
	{	// we are unbinding all keys with the HIDBinding "control"
		for (key=0; key < SDLK_LAST; ++key)
		{
			if (key == SDLK_ESCAPE || key == SDLK_BACKQUOTE)
			{	// We will not allow these keys to be rebound
				++key; 
			}
			if (keybindings[key] == j->second)
			{
				keybindings[key] = 0;
			}
		}

		for (button=2; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			if (mousebindings[button] == j->second)
			{
				mousebindings[button] = 0;
			}
		}

		for (js=0; js < NUM_JOYSTICKS; ++js)
		{
			for (button=0; button < NUM_MOUSEBUTTONS; ++button)
			{
				if (joybindings[js][button] == j->second)
				{
					joybindings[js][button] = 0;
				}
			}
		}
	}
	else
	{	// assume we are unbinding a specific key
		for (key=0; key < SDLK_LAST; ++key)
		{
			if (key == SDLK_ESCAPE || key == SDLK_BACKQUOTE)
			{	// We will not allow these keys to be rebound
				++key; 
			}
			name = SDL_GetKeyName((SDLKey) key);
			if (control == name)
			{
				keybindings[key] = 0;
			}
		}

		for (button=2; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			name = GetMouseButtonName((MouseButton) button);
			if (control == name)
			{
				mousebindings[button] = 0;
			}
		}

		for (js=0; js < NUM_JOYSTICKS; ++js)
		{
			for (button=0; button < NUM_MOUSEBUTTONS; ++button)
			{
				name = GetJoystickButtonName(js, button);
				if (control == name)
				{
					joybindings[js][button] = 0;
				}
			}
		}
	}
}

void HIDManager::ConCmd_bind(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size() != 3)
	{
		if (! argv.empty())
			pout << "Usage: " << argv[0] << " <key> <action>: binds a key or button to an action" << std::endl;
		return;
	}
	HIDManager * hidmanager = HIDManager::get_instance();
	
	Pentagram::istring control(argv[1]);
	Pentagram::istring bindingName(argv[2]);

	hidmanager->bind(control, bindingName);
}

void HIDManager::ConCmd_unbind(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	if (argv.size() != 2)
	{
		if (! argv.empty())
			pout << "Usage: " << argv[0] << " <key or action>: unbinds a key, button, or action" << std::endl;
		return;
	}
	HIDManager * hidmanager = HIDManager::get_instance();
	
	Pentagram::istring control(argv[1]);

	hidmanager->unbind(control);
}

void HIDManager::ConCmd_listbinds(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	HIDManager * hidmanager = HIDManager::get_instance();
	hidmanager->listBindings();
}

void HIDManager::ConCmd_save(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	HIDManager * hidmanager = HIDManager::get_instance();
	hidmanager->saveBindings();

	SettingManager* settings = SettingManager::get_instance();
	settings->write();
}

void HIDManager::ConCmd_execBinding(const Console::ArgsType &args, const Console::ArgvType &argv)
{
	uint32 pos = argv[0].find_last_of(':');
	assert (pos != Pentagram::istring::npos);
	Pentagram::istring bindingName = argv[0].substr( pos + 1 );
	HID_Event event;
	event.xrel = 0;
	event.yrel = 0;
	event.value = 0;
	event.device = HID_OTHER;
	event.type = HID_DOWN;

	if (argv.size() > 1)
	{
		if (argv[1] == "HID_UP" || argv[1] == "OFF" || argv[1] == "0")
		{
			event.type = HID_UP;
		}
	}

	HIDManager * hidmanager = HIDManager::get_instance();
	HIDBinding binding = hidmanager->getBinding(bindingName);
	if (binding)
		binding(event);
}

void HIDManager::listBindings()
{
	HIDBindingMap::iterator it;
	std::vector<const char *> controls;
	std::vector<const char *>::iterator j;

	con.Printf("%-25sKeys\n", "Controls");
	for (it = bindingMap.begin(); it != bindingMap.end(); ++it)
	{
		hidmanager->getBindings(it->first, controls);
		con.Printf(" %-25s", it->first.c_str());
		for (j = controls.begin(); j != controls.end(); ++j)
		{
			con.Printf("%s ", *j);
		}
		con.Putchar('\n');
	}
}


void HIDManager::getBindings(const Pentagram::istring& bindingName, std::vector<const char *>& controls)
{
	controls.clear();
	HIDBindingMap::iterator j = bindingMap.find(bindingName);
	if (j != bindingMap.end())
	{
		uint16 key, button, js;

		for (key=0; key < SDLK_LAST; ++key)
		{
			if (keybindings[key] == j->second)
			{
				controls.push_back(SDL_GetKeyName((SDLKey) key));
			}
		}

		for (button=1; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			if (mousebindings[button] == j->second)
			{
				controls.push_back(GetMouseButtonName((MouseButton) button));
			}
		}

		for (js=0; js < NUM_JOYSTICKS; ++js)
		{
			for (button=0; button < NUM_MOUSEBUTTONS; ++button)
			{
				if (joybindings[js][button] == j->second)
				{
					controls.push_back(GetJoystickButtonName(js, button));
				}
			}
		}
	}
}
