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

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <string>
#include "istring.h"
#include <map>
#include <vector>

class ConfigFileManager;

class SettingManager
{
 public:
	SettingManager();
	~SettingManager();

	static SettingManager* get_instance() { return settingmanager; }

	enum Domain {
		DOM_DEFAULTS = 0,
		DOM_GLOBAL   = 1,
		DOM_GAME     = 2,
		DOM_CURRENT  = 100
	};

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(std::string fname, bool readonly=false);

	//! write all (writable) config files
	void write();

	//! does the key exist?
	bool exists(Pentagram::istring key, Domain dom = DOM_CURRENT);

	//! get value
	bool get(Pentagram::istring key,std::string &ret,Domain dom = DOM_CURRENT);
	//! get value
	bool get(Pentagram::istring key, int &ret, Domain dom = DOM_CURRENT);
	//! get value
	bool get(Pentagram::istring key, bool &ret, Domain dom = DOM_CURRENT);

	//! set value
	void set(Pentagram::istring key,std::string value,Domain dom=DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key,const char* value,Domain dom=DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key, int value, Domain dom = DOM_CURRENT);
	//! set value
	void set(Pentagram::istring key, bool value, Domain dom = DOM_CURRENT);

	//! remove key
	void unset(Pentagram::istring key, Domain dom = DOM_CURRENT);

	//! set default value
	void setDefault(Pentagram::istring key, std::string value);
	//! set default value
	void setDefault(Pentagram::istring key, const char* value);
	//! set default value
	void setDefault(Pentagram::istring key, int value);
	//! set default value
	void setDefault(Pentagram::istring key, bool value);

	//! set the current domain
	void setCurrentDomain(Domain dom);
	//! set the configuration section for a domain
	void setDomainName(Domain dom, Pentagram::istring section);

	typedef void (*ConfigCallback)(Pentagram::istring key);

	//! register a function to be called when the key changes
	void registerCallback(Pentagram::istring key, ConfigCallback callback);
	//! unregister a callback
	void unregisterCallback(Pentagram::istring key, ConfigCallback callback);

	//! list all games
	std::vector<Pentagram::istring> listGames();

	//! list all keys in a game data section
	//! \param section The section to return setkeys of
	//! \return the keys. They have no guaranteed order.
	std::vector<Pentagram::istring> listDataKeys(Pentagram::istring section);

	//! list all key-value pairs in the given section.
	//! \param section The section to list
	//! \return the key-value pairs. They have no guaranteed order.
	std::map<Pentagram::istring,std::string> listDataValues(
		Pentagram::istring section);

private:

	bool findKeyDomain(Pentagram::istring key, Domain dom, Domain& keydom);
	Pentagram::istring getConfigKey(Pentagram::istring key, Domain dom);
	void callCallbacks(Pentagram::istring key);

	std::map<Pentagram::istring, std::vector<ConfigCallback> > callbacks;
	std::vector<Pentagram::istring> domains;

	Domain currentDomain;

	ConfigFileManager* conffileman;

	static SettingManager* settingmanager;
};

#endif
