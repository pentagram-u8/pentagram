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

#ifndef XMLNODE_H
#define XMLNODE_H

#include <string>
#include <vector>
#include "istring.h"


class	XMLNode
{
protected:
	Pentagram::istring		id;
	std::string				content;
	std::vector<XMLNode*>	nodelist;
	bool					no_close;
	
public:
	XMLNode() : no_close(false)
		{  }
	XMLNode(const Pentagram::istring &i) : id(i), no_close(false)
		{  }
	XMLNode(const XMLNode &n)
		: id(n.id),content(n.content),nodelist(n.nodelist), no_close(false)
		{  }
	~XMLNode();
	
	XMLNode &operator=(const XMLNode &n) {
		id=n.id; content=n.content; nodelist=n.nodelist; no_close = n.no_close;
		return *this;
	}

	const std::string &reference(const Pentagram::istring &,bool &);
	const XMLNode *subtree(const Pentagram::istring &) const;
	
	const std::string &value(void) const { 
		return content;
	}
	
	typedef std::pair<Pentagram::istring, std::string> KeyType;
	typedef std::vector<KeyType> KeyTypeList;
	
	bool searchpairs(KeyTypeList &ktl, const Pentagram::istring &basekey,
					 const Pentagram::istring currkey, const unsigned int pos);
	void selectpairs(KeyTypeList &ktl, const Pentagram::istring currkey);
	
	Pentagram::istring dump(int depth = 0);
	
	void xmlassign(const Pentagram::istring &key, const std::string &value);
	void xmlparse(const std::string &s,std::size_t &pos);

	void listkeys(const Pentagram::istring &,std::vector<Pentagram::istring> &,
				  bool longformat=true) const;
};




#endif
