/*
 *
 *
 *  Copyright (C) 2002-2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Many things really, truely suck. One of them is the interface to the c++
	FlexLexer... */
#undef yyFlexLexer
#define yyFlexLexer llcFlexLexer
#include "llcLexer.h"
/* One of the problems being that it expects that flex is installed and that
	FlexLexer.h is found in one of the global include paths, which may
	not be the case, so we'll likely have to include it in our cvs
	staticly. *bleah* */

#include <list>
using std::list;
#include <fstream>
using std::ifstream;
#include <iomanip>
#include <string>
#include <cassert>

#include "llcTokens.h"
#include "CompileUnit.h"

#include "pent_include.h"
#include "Application.h"

//#define CTRACE(X)
#define CTRACE(x) perr << x << std::endl

const char *UNKNOWN_ERR = "Unknown Expression.";

const char * const random_generic_errors[] =
{
	"Ugh! That's got to be the sickest thing I've ever seen!\n ...next to that poncy git there.\n\t--Giants: Citizen Kabuto",
	""
};

/****************************************************************************
	Warning Messages
 ****************************************************************************/
enum MessageType { MT_MSG, MT_WARN, MT_ERR, MT_ICE };

class CompileUnit;

void msg(const MessageType msgType, const char * const msgDetails, CompileUnit *cu=0, const sint32 retCode=-1);
/****************************************************************************/

// Compile Unit **************************************************************

// Wheeee! Debugging stuff...
void CompileUnit::debugPrint(std::ostream &o, CompileNode *n) const
{
	debugPrintHead(o, n);
	n->print_unk(o);
	debugPrint(o);
}

void CompileUnit::debugPrint(std::ostream &o) const
{
	debugPrintHead(o);
	debugPrintBody(o);
}

void CompileUnit::debugPrintHead(std::ostream &o, CompileNode *n) const
{
	o << "Compile Stack:" << std::endl;
	if(n!=0)
		o << std::setw(4) << n->linenum << ": ";
	else
		if(nodes.size())
			o << std::setw(4) << nodes.front()->linenum << ": ";
		else
			assert(false); // this really shouldn't be able to happen...

}

void CompileUnit::debugPrintBody(std::ostream &o) const
{
	for(std::list<CompileNode *>::const_iterator i=nodes.begin(); i!=nodes.end(); i++)
	{
		o << '\t';
		(*i)->print_unk(o);
		o << std::endl;
	}
}

#ifdef WANT_COMPILE_TEST

enum TestX { TEST_END=0, TEST_XFAIL, TEST_XPASS, TEST_XWARN };

struct CTestS
{
	const char * const filename;
	TestX xpect;
};

CTestS ctests[] = {
	{ "@data/test-warn/empty-class.llc",	TEST_XPASS },
	{ "@data/test-warn/nothing.llc",		TEST_XWARN },
	{ "@data/test-err/invalid-block.llc", 	TEST_XFAIL },
	{ "@data/test-err/invalid-class-2.llc",	TEST_XFAIL },

	{ "", TEST_END },
};

#endif

bool CompileUnit::parse()
{
	CTRACE("CompileUnit::parse()");

	std::string fname;
	//fname = "@data/test-warn/empty-class.llc";
	//fname = "@data/test-warn/nothing.llc";
	//fname = "@data/test-err/invalid-block.llc";
	//fname = "@data/test-err/invalid-class-2.llc"; // should err, doesn't

	if(!parser)
	{
		#ifdef WANT_COMPILE_TEST
		fname = ctests[testidx].filename;
		#endif

		idatasource = filesys->ReadFile(fname.c_str());
		assert(idatasource!=0); // FIXME: Error checking!
		ifile = GetRawIfstream(idatasource);

		assert((ifile!=0) && (!ifile->fail())); //FIXME: Error checking!

		parser = new llcFlexLexer(ifile, 0);
		//parser->switch_streams(ifile, 0);
	}

	sint32 token=parser->yylex();
	switch(token)
	{
		case LLC_CLASS:
			nodes.push_back(new ClassNode(parser->lineno()));
			break;
		case LLC_IDENT:
			nodes.push_back(new VarIdentNode(parser->YYText(), parser->lineno()));
			break;
		case LLC_OPEN_BRACE:
			// adding this is rather pointless, but makes error reporting cleaner
			nodes.push_back(new CStringNode("{", parser->lineno()));
			return parse_openblock();
			break;
		/*case LLC_UNIT:
			{
				* Unit declarations take the form:
					unit = "Unit Name";
				*
				// got 'unit'
				const char * const unitErr = "Unit declaration of the form:\n\tunit = \"Unit Name\";\nexpected.";
				expect(LLC_EQUALS, parser, unitErr, ERR_UNIT); // want '='
				expect(LLC_STRING, LLC_CSTRING, parser, unitErr, ERR_UNIT); // want "Unit Name" or 'Unit Name'
				name=parser.YYText();
				expect(LLC_SEMICOLON, parser, unitErr, ERR_UNIT); // want ';'
			}
			break;*/
		case LLC_EOF:
			debugPrint(pout);
			if(currclass==0)
				msg(MT_WARN, "Encountered end-of-file without encountering a class definition.");
			setState(CSTATE_FINISHED); // FIXME: Should handle multiple files in the future...
			return true;
			break;
		default:
			con.Printf("%d\n", token);
			return false;
	}

	return false;
}

bool CompileUnit::parse_openblock()
{
	CTRACE("CompileUnit::parse_openblock()");
	/* Ok... so we get here when we recieve a { in the stream. This should only be from a:
		class Fnord ... {
		if (...) {
		else if(...) {
		else {
		foreach ... {

		So our task, for the moment, is theorietically simple, just fold all the following
		tokens *back* into the original c++class new-ed for the primary nodes.
	*/

	// first see if we *have* any nodes, if we don't then we've got serious problems.
	if(nodes.size()==0)
		msg(MT_ICE, "Empty nodes stack should not occur within CompileUnit::parse_openblock()", this);

	// class Fnord {
	// class Fnord inherits FnordBase {
	if(found(LLC_CLASS)) //(nodes.front()->isA(LLC_CLASS))
	{
		ClassNode *c = static_cast<ClassNode *>(nodes.front());
		nodes.pop_front();
		if(!found(LLC_IDENT))
		{
			nodes.push_front(c);
			msg(MT_ERR, "Class name not found in declaration.", this);
			return true;
		}
		else
		{
			c->name=static_cast<VarIdentNode *>(nodes.front())->str;
			nodes.pop_front();
			return false;
		}
	}
	// general case where we've got a 'block' but nothing to define it. Really shouldn't happen...
	else if(found(LLC_STRING) && static_cast<CStringNode *>(nodes.front())->str=="{")
	{
		msg(MT_ERR, "Incorrectly defined block {", this);
		return true;
	}
	else
	{
		debugPrint(pout); assert(false); //FIXME: Error checking!
	}

	debugPrint(pout);
	return false;
};

void CompileUnit::setState(const CState cs)
{
	switch(cs)
	{
		// we're going to set outselves up nice and pristine for the new compile...
		case CSTATE_NEW:
			// if we've just succeeded, there should be no nodes...
			if(_state==CSTATE_FINISHED && nodes.size()!=0)
				msg(MT_ICE, "Nodes stack should be empty on successful compile, it isn't! CompileUnit::setState()", this);
			nodes.clear(); // remove any excess nodes, should they be there...
			if(currclass!=0)
				tailclasses.push_back(currclass); // archive our class, should we need it's info later...
			currclass=0;
			FORGET_OBJECT(idatasource); // delete our data stream...
			ifile=0; // ... and our ifstream should have been clobbered also
			FORGET_OBJECT(parser); // blow away our parser. technically not needed and should be 'fixed' later... FIXME:
			_state=cs;
			break;
		case CSTATE_FINISHED:
		case CSTATE_FAIL:
		case CSTATE_WORKING:
			_state=cs;
			break;
		default:
			CANT_HAPPEN();
			break;
	}
}

/****************************************************************************
	Warning Messages
 ****************************************************************************/
void msg(const MessageType msgType, const char * const msgDetails, CompileUnit *cu, const sint32 retCode)
{
	switch(msgType)
	{
		case MT_MSG:
			con.Printf_err("%s\n", msgDetails);
			break;
		case MT_WARN:
			if(cu!=0) cu->debugPrint(pout);
			con.Printf_err("Warning: %s\n", msgDetails);
			break;
		case MT_ERR:
			assert(cu!=0);
			cu->debugPrint(pout);
			con.Printf_err("Error: %s\n", msgDetails);
			cu->setState(CompileUnit::CSTATE_FAIL);
			//Application::application->ForceQuit();
			break;
		case MT_ICE:
			con.Printf_err("Internal Compile Error: %s\n", msgDetails);
			con.Printf_err(">> Attempting to dump internal datastacks...\n");
			assert(cu!=0);
			cu->debugPrint(pout);
			cu->setState(CompileUnit::CSTATE_FAIL);
			//Application::application->ForceQuit();
			break;
		default:
			assert(false); // can't happen
	}
}


