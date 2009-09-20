/*
Copyright (C) 2002-2009 The Pentagram team

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

#include <SDL.h>
#include "GUIApp.h"
#include "MemoryManager.h"
#include "version.h"

#ifdef _WIN32
// Disable SDLMain in Windows
#ifdef main
#undef main
#endif
#endif

int main(int argc, char* argv[])
{
	// Initialize Memory Manager here to avoid extra tools depending on it
	MemoryManager mm;
	GUIApp app(argc, argv);

#ifdef SAFE_CONSOLE_STREAMS
	console_streambuf<char> fb;
	ppout = new console_ostream<char>(&fb);

	console_err_streambuf<char> err_fb;
	pperr = new console_err_ostream<char>(&err_fb);

#endif

	pout << "Pentagram version " << PentagramVersion::version << std::endl;
	pout << "Built: " << PentagramVersion::buildtime << std::endl;
	pout << "Optional features: " << PentagramVersion::features << std::endl;
	pout << std::endl;

	app.startup();
	app.run();

	return 0;
}

#ifdef _WIN32
#include <windows.h>
#include <cstdio>

 PCHAR*
 CommandLineToArgvA(
     PCHAR CmdLine,
     int* _argc
     )
 {
     PCHAR* argv;
     PCHAR  _argv;
     ULONG   len;
     ULONG   argc;
     CHAR   a;
     ULONG   i, j;

     BOOLEAN  in_QM;
     BOOLEAN  in_TEXT;
     BOOLEAN  in_SPACE;

     len = strlen(CmdLine);
     i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

     argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
         i + (len+2)*sizeof(CHAR));

     _argv = (PCHAR)(((PUCHAR)argv)+i);

     argc = 0;
     argv[argc] = _argv;
     in_QM = FALSE;
     in_TEXT = FALSE;
     in_SPACE = TRUE;
     i = 0;
     j = 0;

     while( a = CmdLine[i] ) {
         if(in_QM) {
             if(a == '\"') {
                 in_QM = FALSE;
             } else {
                 _argv[j] = a;
                 j++;
             }
         } else {
             switch(a) {
             case '\"':
                 in_QM = TRUE;
                 in_TEXT = TRUE;
                 if(in_SPACE) {
                     argv[argc] = _argv+j;
                     argc++;
                 }
                 in_SPACE = FALSE;
                 break;
             case ' ':
             case '\t':
             case '\n':
             case '\r':
                 if(in_TEXT) {
                     _argv[j] = '\0';
                     j++;
                 }
                 in_TEXT = FALSE;
                 in_SPACE = TRUE;
                 break;
             default:
                 in_TEXT = TRUE;
                 if(in_SPACE) {
                     argv[argc] = _argv+j;
                     argc++;
                 }
                 _argv[j] = a;
                 j++;
                 in_SPACE = FALSE;
                 break;
             }
         }
         i++;
     }
     _argv[j] = '\0';
     argv[argc] = NULL;

     (*_argc) = argc;
     return argv;
 }


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int argc;
	char **argv = CommandLineToArgvA(GetCommandLineA(), &argc);

	// FIXME - Multiuser aware please

	std::freopen("pstdout.txt","wt", stdout);
	std::freopen("pstderr.txt","wt", stderr);

	int res = main(argc, argv);

	if (!std::ftell(stdout)) {
		std::fclose(stdout);
		std::remove("pstdio.txt");
	}
	if (!std::ftell(stderr)) {
		std::fclose(stderr);
		std::remove("pstderr.txt");
	}

	// FIXME - Memory leak here. Need to free memory allocated for argv.

	return  res;
}
#endif
