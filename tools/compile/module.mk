# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/compile

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)

tools/compile/llcLexer.l: 

tools/compile/llcLexer.cpp: tools/compile/llcLexer.l
	flex++ -+ -otools/compile/llcLexer.preproc1 tools/compile/llcLexer.l
	# Fix the reference to <FlexLexer.h>...
	sed 's/#include <FlexLexer.h>/#include "llcLexer.h"/' tools/compile/llcLexer.preproc1  >tools/compile/llcLexer.preproc2
	# Fix the misnaming of the #line's...
	sed 's/"tools\/compile\/llcLexer.preproc1"/"tools\/compile\/llcLexer.cpp"/' tools/compile/llcLexer.preproc2 >tools/compile/llcLexer.preproc3
	# What the heck, lets fix a few of the warnings whilst we're at it!
	sed 's/#define YY_SC_TO_UI(c) ((unsigned int) (unsigned char) c)/#define YY_SC_TO_UI(c) (static_cast<unsigned int>(static_cast<unsigned char>(c)))/' tools/compile/llcLexer.preproc3 >tools/compile/llcLexer.preproc4
	sed 's/(unsigned int) yy_c/static_cast<unsigned int>(yy_c)/' tools/compile/llcLexer.preproc4 >tools/compile/llcLexer.preproc5
	sed 's/(int) yy_def\[yy_current_state\]/static_cast<int>(yy_def\[yy_current_state\])/' tools/compile/llcLexer.preproc5 >tools/compile/llcLexer.preproc6
	sed 's/(int) /static_cast<int>/' tools/compile/llcLexer.preproc6 >tools/compile/llcLexer.preproc7
	# Add the necessary One True File
	sed 's/llcFlexLexer/llcFlexLexer\n#include "pent_include.h"/' tools/compile/llcLexer.preproc7 >tools/compile/llcLexer.cpp
	# Do a little cleanup
	$(RM) tools/compile/llcLexer.preproc1 tools/compile/llcLexer.preproc2 tools/compile/llcLexer.preproc3 tools/compile/llcLexer.preproc4 tools/compile/llcLexer.preproc5 tools/compile/llcLexer.preproc6 tools/compile/llcLexer.preproc7

# Common rules
include $(srcdir)/common.mk
