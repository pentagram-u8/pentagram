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
	assert(hidmanager == 0);
	hidmanager = this;

	for (uint16 key=0; key < SDLK_LAST; ++key)
	{
		keybindings[key] = 0;
	}
	
	for (uint16 button=0; button < NUM_MOUSEBUTTONS+1; ++button)
	{
		mousebindings[button] = 0;
	}

	bindingMap.insert( HIDBINDING_PAIR(quickSave) );
	bindingMap.insert( HIDBINDING_PAIR(quickLoad) );
	bindingMap.insert( HIDBINDING_PAIR(avatarInStatis) );
	bindingMap.insert( HIDBINDING_PAIR(engineStats) );
	bindingMap.insert( HIDBINDING_PAIR(paintEditorItems) );
	bindingMap.insert( HIDBINDING_PAIR(itemLocator) );
	bindingMap.insert( HIDBINDING_PAIR(toggleCombat) );
	bindingMap.insert( HIDBINDING_PAIR(openInventory) );
	bindingMap.insert( HIDBINDING_PAIR(openBackpack) );
	bindingMap.insert( HIDBINDING_PAIR(recall) );
	bindingMap.insert( HIDBINDING_PAIR(runFirstEgg) );
	bindingMap.insert( HIDBINDING_PAIR(runExecutionEgg) );
	bindingMap.insert( HIDBINDING_PAIR(toggleFrameByFrame) );
	bindingMap.insert( HIDBINDING_PAIR(advanceFrameByFrame) );
	bindingMap.insert( HIDBINDING_PAIR(u8ShapeViewer) );
	bindingMap.insert( HIDBINDING_PAIR(showMenu) );
	bindingMap.insert( HIDBINDING_PAIR(quit) );
	bindingMap.insert( HIDBINDING_PAIR(toggleConsole) );

	keybindings[SDLK_ESCAPE] = &HIDBindings::quit;
	keybindings[SDLK_BACKQUOTE] = &HIDBindings::toggleConsole;
}

HIDManager::~HIDManager()
{
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
		binding = keybindings[key];
	}
	break;
	case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
	{
		uint16 button = event.button.button;
		binding = mousebindings[button];
	}
	break;
	}

	return binding;
}

void HIDManager::loadBindings()
{
#if 0
	SettingManager* settings = SettingManager::get_instance();
	std::map<Pentagram::istring, std::string> keys;
	keys = settings->listDataValues("keys");
#else
	ConfigFileManager* config = ConfigFileManager::get_instance();
	std::map<Pentagram::istring, std::string> keys;
	keys = config->listKeyValues("bindings/bindings");
#endif

	std::map<Pentagram::istring, std::string>::iterator i;
	for (i = keys.begin(); i != keys.end(); ++i)
	{
		Pentagram::istring bindingName = (*i).second.c_str();
		bind((*i).first, bindingName);
	}
}

void HIDManager::bind(const Pentagram::istring& control, const Pentagram::istring& bindingName)
{
	uint16 key = 0;
	uint16 button = 1;
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
				pout << "Binding \"" << name
					<< "\" to " << (*j).first.c_str() << std::endl;
				keybindings[key] = (*j).second;

				// We found the matching SDLKey. Stop searching;
				break;
			}
		}

		if (key >= SDLK_LAST) // we did not find a matching SDLKey
		{
			// Only bind Mouse 3 and up for now
			for (button=3; button < NUM_MOUSEBUTTONS+1; ++button)
			{
				name = GetMouseButtonName((MouseButton) button);
				if (control == name)
				{
					pout << "Binding \"" << name
						<< "\" to " << (*j).first.c_str() << std::endl;
					mousebindings[button] = (*j).second;

					// We found the matching Mouse Button. Stop searching;
					break;
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

void HIDManager::getBindings(const Pentagram::istring& bindingName, std::vector<const char *>& controls)
{
	controls.clear();
	HIDBindingMap::iterator j = bindingMap.find(bindingName);
	if (j != bindingMap.end())
	{

		for (uint16 key=0; key < SDLK_LAST; ++key)
		{
			if (keybindings[key] == (*j).second)
			{
				controls.push_back(SDL_GetKeyName((SDLKey) key));
			}
		}

		for (uint16 button=1; button < NUM_MOUSEBUTTONS+1; ++button)
		{
			if (mousebindings[button] == (*j).second)
			{
				controls.push_back(GetMouseButtonName((MouseButton) button));
			}
		}
	}	
}
