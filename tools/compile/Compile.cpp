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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

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

#include "CoreApp.h"

//#define CTRACE(X)
#define CTRACE(x) perr << x << std::endl

const char *UNKNOWN_ERR = "Unknown Expression.";

const char * const random_generic_errors[] =
{
	"Ugh! That's got to be the sickest thing I've ever seen!\n ...next to that poncy git there.\n\t--Giants: Citizen Kabuto",
	"Hark! Is that the sweet song of lamentation I hear?\n\t-- The Guardian, Ultima 8",
	"I do so enjoy the cries of torment.\n\t-- The Guardian, Ultima 8",
	"Ouch! -That- must have hurt, Avatar!\n\t-- The Guardian, Ultima 8",
	
	""
};

#ifdef USE_CQUIRKS
#define QUIRK(X, Y) X
#else
#define QURIK(X, Y) Y
#endif

#define ICE_ME(X) { msg(MT_ICE, X, this); return true; }

/****************************************************************************
	Warning Messages
 ****************************************************************************/
enum MsgType { MT_MSG, MT_WARN, MT_ERR, MT_ICE };

class CompileUnit;

MsgType msg(const MsgType msgType, const char * const msgDetails, CompileUnit *cu=0);
/****************************************************************************/

// Compile Unit **************************************************************
#include "CompileUnit.h"

CompileUnit::CompileUnit(FileSystem *filesystem) : currclass(0), ifile(0), idatasource(0),
	parser(0), filesys(filesystem), _state(CSTATE_NEW), _ccomplete(false),
	_expect(LLC_NONE), _warned(false)
	#ifdef COMPILER_TEST
	, testidx(0)
	#endif
{
	setState(CSTATE_NEW);
	filesys->ListFiles("@work/usecode/src/*", filelist);
	for(FileSystem::FileList::iterator i=filelist.begin(); i!=filelist.end(); i++)
		pout << "FILE: " << *i << std::endl;

	#ifdef COMPILER_TEST
	//filelist.push_back("@data/test-err/invalid-class-2.llc");
	//filelist.push_back("@data/test-err/invalid-block.llc");
	filelist.push_back("@work/usecode/src/nothing.llc");
	//filelist.push_back("@data/test-err/invalid-class-3.llc");
	filelist.push_back("@work/usecode/src/empty-class.llc");
	#endif
}

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

#ifdef COMPILER_TEST

/*enum TestX { TEST_END=0, TEST_XFAIL, TEST_XPASS, TEST_XWARN };

struct CTestS
{
	const char * filename;
	TestX xpect;
};*/

#endif

bool CompileUnit::consume(const LLCToken &tok)
{
	if(found(tok))
		nodes.pop_front();
	else
		ICE_ME("Errornous token encountered in stream in CompileUnit::consume()");
		
	return false;
}

bool CompileUnit::parse()
{
	CTRACE("CompileUnit::parse()");

	std::string fname;
	
	if(!parser || state()==CSTATE_FINISHED)
	{
		#ifdef COMPILER_TEST
		//fname = ctests[testidx].filename;
		//pout << "Testing: " << fname << "..." << std::endl;
		#endif
		assert(filelist.size());
		fname = filelist.back();
		filelist.pop_back();
		pout << "Opening... " << fname << std::endl;

		idatasource = filesys->ReadFile(fname.c_str());
		assert(idatasource!=0); // FIXME: Error checking!
		ifile = idatasource->GetRawIfstream();

		assert((ifile!=0) && (!ifile->fail())); //FIXME: Error checking!

		parser = new llcFlexLexer(ifile, 0);
		//parser->switch_streams(ifile, 0);
	}

	sint32 token=parser->yylex();
	switch(token)
	{
		// #expect cases
		case LLC_XFAIL: _expect=LLC_XFAIL; return true; break;
		case LLC_XPASS: _expect=LLC_XPASS; return true; break;
		case LLC_XWARN: _expect=LLC_XWARN; return true; break;
	
		// more normal cases...
		case LLC_CLASS:
			nodes.push_back(new ClassNode(parser->lineno()));
			break;
		case LLC_IDENT:
			nodes.push_back(new VarIdentNode(parser->YYText(), parser->lineno()));
			break;
		case LLC_OPEN_BRACE:
			// adding this is rather pointless, but makes error reporting cleaner
			nodes.push_back(new FencePostNode(LLC_OPEN_BRACE, parser->lineno()));
			return parse_openblock();
			break;
		case LLC_CLOSE_BRACE:
			nodes.push_back(new FencePostNode(LLC_CLOSE_BRACE, parser->lineno()));
			return parse_closeblock();
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
			if(currclass==0)
				msg(MT_WARN, "Encountered end-of-file without encountering a class definition.");
			setState(CSTATE_FINISHED); // FIXME: Should handle multiple files in the future...
			return true;
			break;
		default:
			con.Printf("\n%d\n", token);
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
		ICE_ME("Empty nodes stack should not occur within CompileUnit::parse_openblock()");

	// >class< Fnord {
	// >class< Fnord inherits class FnordBase {
	if(found(LLC_CLASS))
	{
		if(currclass!=0)
		{
			msg(MT_ERR, "Class already defined in this file.", this);
			return true;
		}
		
		currclass = static_cast<ClassNode *>(nodes.front());
		nodes.pop_front();
		
		// class >Fnord< {
		// class >Fnord< inherits class FnordBase {
		if(!found(LLC_IDENT))
		{
			nodes.push_front(currclass); currclass=0;
			msg(MT_ERR, "Class name not found in declaration.", this);
			return true;
		}
		
		currclass->name=static_cast<VarIdentNode *>(nodes.front())->str;
		nodes.pop_front();

		if(!found(LLC_INHERITS) && !found(LLC_OPEN_BRACE))
		{
			nodes.push_front(currclass); currclass=0;
			msg(MT_ERR, "Badly formed class expression: 'inherits' or '{' expected.", this);
			return true;
		}
		
		// class Fnord >{<
		if(found(LLC_OPEN_BRACE))
		{
			consume(LLC_OPEN_BRACE);
			return true;
		}
		
		if(found(LLC_INHERITS))
		{

		}

		return false;
	}
	// general case where we've got a 'block' but nothing to define it. Really shouldn't happen...
	else if(found(LLC_OPEN_BRACE))
	{
		msg(MT_ERR, "Incorrectly defined block {", this);
		return true;
	}
	else
	{
		debugPrint(pout); assert(false); //FIXME: Error checking!
	}

	return false;
};


bool CompileUnit::parse_closeblock()
{
	CTRACE("CompileUnit::parse_openblock()");
	/* This should handle the closing of any opened blocks.
		Basically it should 'just' be a '}' on the nodes stack, and it should
		just have to finish off any special things one might need to do terminate
		a function/class/if/whatever block.
	*/

	// first see if we *have* any nodes, if we don't then we've got serious problems.
	if(nodes.size()==0)
		ICE_ME("Empty nodes stack should not occur within CompileUnit::parse_closeblock()");
	
	// honestly, there shouldn't be anything on the stack except a }, if there is... we've got problems!
	if(nodes.size()>1 || !found(LLC_CLOSE_BRACE))
		ICE_ME("Unknown tokens before block terminator.");
	
	consume(LLC_CLOSE_BRACE);

	return false;
}

bool CompileUnit::setState(const CState cs)
{
	switch(cs)
	{
		// we're going to set outselves up nice and pristine for the new compile...
		case CSTATE_NEW:
			// if we've just succeeded, there should be no nodes...
			if(_state==CSTATE_FINISHED && nodes.size()!=0)
				ICE_ME("Nodes stack should be empty on successful compile, it isn't! CompileUnit::setState()");
			nodes.clear(); // remove any excess nodes, should they be there...
			if(currclass!=0)
				tailclasses.push_back(currclass); // archive our class, should we need it's info later...
			currclass=0;
			FORGET_OBJECT(idatasource); // delete our data stream...
			ifile=0; // ... and our ifstream should have been clobbered also
			FORGET_OBJECT(parser); // blow away our parser. technically not needed and should be 'fixed' later... FIXME:
			_warned=false;
			_expect=LLC_NONE;
			_state=cs;
			break;
		case CSTATE_FINISHED:
		case CSTATE_FAIL:
		case CSTATE_WORKING:
			_state=cs;
			break;
		case CSTATE_WARNED: // doesn't actually change the state, but sets a flag instead...
			_warned=true;
			break;
		default:
			CANT_HAPPEN();
			break;
	}
	return false;
}

/****************************************************************************
	Warning Messages
 ****************************************************************************/
MsgType msg(const MsgType msgType, const char * const msgDetails, CompileUnit *cu)
{
	switch(msgType)
	{
		case MT_MSG:
			con.Printf_err("%s\n", msgDetails);
			break;
		case MT_WARN:
			if(cu!=0) cu->debugPrint(pout);
			con.Printf_err("Warning: %s\n", msgDetails);
			cu->setState(CompileUnit::CSTATE_WARNED);
			break;
		case MT_ERR:
			assert(cu!=0);
			cu->debugPrint(pout);
			con.Printf_err("Error: %s\n", msgDetails);
			cu->setState(CompileUnit::CSTATE_FAIL);
			break;
		case MT_ICE:
			con.Printf_err("Internal Compile Error: %s\n", msgDetails);
			con.Printf_err(">> Attempting to dump internal datastacks...\n");
			assert(cu!=0);
			cu->debugPrint(pout);
			cu->setState(CompileUnit::CSTATE_FAIL);
			break;
		default:
			assert(false); // can't happen
	}
	return msgType;
}


