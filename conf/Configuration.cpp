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

#include "Configuration.h"
#include "XMLTree.h"
#include "ConfigNode.h"

using std::string;
using Pentagram::istring;

Configuration::Configuration()
{

}

Configuration::~Configuration()
{
	for (std::vector<XMLTree*>::iterator i = trees.begin();
		 i != trees.end(); ++i)
	{
		delete (*i);
	}
}

bool Configuration::readConfigFile(string fname, istring root,
								   bool readonly)
{
	XMLTree* tree = new XMLTree();
	tree->clear(root);
	if (!tree->readConfigFile(fname)) {
		delete tree;
		return false;
	}
	if (readonly)
		tree->setReadonly();
	trees.push_back(tree);
	return true;
}

void Configuration::write()
{
	for (std::vector<XMLTree*>::iterator i = trees.begin();
		 i != trees.end(); ++i)
	{
		if (!(*i)->isReadonly())
			(*i)->write();
	}
}

void Configuration::clear()
{
	for (std::vector<XMLTree*>::iterator i = trees.begin();
		 i != trees.end(); ++i)
	{
		delete (*i);
	}
	trees.clear();
}

void Configuration::value(istring key, string &ret, const char *defaultvalue)
{
	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

void Configuration::value(istring key, int &ret, int defaultvalue)
{
	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

void Configuration::value(istring key, bool &ret, bool defaultvalue)
{
	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if ((*i)->hasNode(key)) {
			(*i)->value(key, ret, defaultvalue);
			return;
		}
	}

	ret = defaultvalue;
}

bool Configuration::set(istring key, string value)
{
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if (!((*i)->isReadonly()) && 
			(*i)->checkRoot(key))
		{
			(*i)->set(key, value);
			return true;
		}
	}

	PERR("No writable config file found: unable to set value");
	// we could maybe add a non-file XMLTree to the list and save to that?
	// OTOH, that might make things very untransparent
	return false;
}

bool Configuration::set(istring key, const char* value)
{
	return set(key, string(value));
}


bool Configuration::set(istring key, int value)
{
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if (!((*i)->isReadonly()) && 
			(*i)->checkRoot(key))
		{
			(*i)->set(key, value);
			return true;
		}
	}

	PERR("No writable config file found: unable to set value");
	return false;
}

bool Configuration::set(istring key, bool value)
{
	// Currently a value is written to the last writable tree with
	// the correct root.

	for (std::vector<XMLTree*>::reverse_iterator i = trees.rbegin();
		 i != trees.rend(); ++i)
	{
		if (!((*i)->isReadonly()) && 
			(*i)->checkRoot(key))
		{
			(*i)->set(key, value);
			return true;
		}
	}

	PERR("No writable config file found: unable to set value");
	return false;
}

ConfigNode* Configuration::getNode(istring key)
{
	return new ConfigNode(*this, key);
}

std::set<istring> Configuration::listKeys(istring key, bool longformat)
{
	std::set<istring> keys;
	for (std::vector<XMLTree*>::iterator i = trees.begin();
		 i != trees.end(); ++i)
	{
		std::vector<istring> k = (*i)->listKeys(key, longformat);
		for (std::vector<istring>::iterator iter = k.begin();
			 iter != k.end(); ++iter)
		{
			keys.insert(*iter);
		}
	}
	return keys;
}

void Configuration::getSubkeys(KeyTypeList &ktl, istring basekey)
{
	for (std::vector<XMLTree*>::iterator tree = trees.begin();
		 tree != trees.end(); ++tree)
	{
		KeyTypeList l;
		(*tree)->getSubkeys(l, basekey);

		for (KeyTypeList::iterator i = l.begin();
			 i != l.end(); ++i)
		{
			bool found = false;
			for (KeyTypeList::iterator j = ktl.begin();
				 j != ktl.end() && !found; ++j)
			{
				if (j->first == i->first) {
					// already have this subkey, so just replace the value
					j->second = i->second;
					found = true;
				}
			}
			if (!found) {
				// new subkey
				ktl.push_back(*i);
			}
		}
	}
}
