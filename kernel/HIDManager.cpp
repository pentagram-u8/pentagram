/*
Copyright (C) 2003 The Pentagram team

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
#include "Q_strcasecmp.h"
#include "GUIApp.h"
#include "Configuration.h"

HIDManager* HIDManager::hidmanager = 0;

HIDManager::HIDManager()
{
	assert(hidmanager == 0);
	hidmanager = this;

	for (uint16 key=0; key < SDLK_LAST; ++key)
	{
		keybindings[key] = 0;
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
	}

	return binding;
}

void HIDManager::loadBindings()
{
	Configuration * config= GUIApp::get_instance()->getConfig();
	Configuration::KeyTypeList ktl;
	config->getSubkeys(ktl, "bindings");

	for( std::vector<Configuration::KeyType>::iterator i = ktl.begin();
		i != ktl.end(); ++i)
	{
		Pentagram::istring bindingName = (*i).second.c_str();
		HIDBindingMap::iterator j = bindingMap.find(bindingName);
		if (j != bindingMap.end())
		{
			for (uint16 key=0; key < SDLK_LAST; ++key)
			{
				if (! Pentagram::Q_strcasecmp((*i).first.c_str(),
					SDL_GetKeyName((SDLKey) key)))
				{
					pout << "Binding \"" << SDL_GetKeyName((SDLKey) key)
						<< "\" to " << (*i).second << std::endl;
					keybindings[key] = (*j).second;

					// We found the matching SDLKey. Stop searching;
					break;
				}
			}
		}
	}
	
}
