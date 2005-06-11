# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LPRODUCTS := pentagram
LINSTALL := pentagram

include $(srcdir)/objects.mk

pentagram_OBJ = \
	$(KERNEL) \
	$(KERNEL2) \
	$(USECODE) \
	$(FILESYS) \
	$(UNZIP) \
	$(GAMES) \
	$(GAMES2) \
	$(GRAPHICS) \
	$(SCALERS) \
	$(FONTS) \
	$(MISC) \
	$(CONVERT) \
	$(CONF) \
	$(GUMPS) \
	$(WIDGETS) \
	$(WORLD) \
	$(ACTORS) \
	$(ARGS) \
	$(AUDIO) \
	$(MIDI) \
	$(TIMIDITY) \
	kernel/GUIApp.o \
	misc/version.o \
	pentagram.o

llc_OBJ = \
	$(KERNEL) \
	$(KERNEL2) \
	$(USECODE) \
	$(FILESYS) \
	$(UNZIP) \
	$(GAMES) \
	$(GAMES2) \
	$(GRAPHICS) \
	$(SCALERS) \
	$(FONTS) \
	$(MISC) \
	$(CONVERT) \
	$(CONF) \
	$(GUMPS) \
	$(WIDGETS) \
	$(WORLD) \
	$(ACTORS) \
	$(COMPILE) \
	$(DISASM) \
	$(ARGS) \
	$(AUDIO) \
	$(MIDI) \
	$(TIMIDITY) \
	kernel/GUIApp.o \
	kernel/ConApp.o \
	tools/compile/llc.o
# Unfortunately we still need to split things a bit more cleanly before
# removing the dependancy of the console stuff on the GUIApp.

# make version.o depend on everything to force a rebuild (for build time/date)
misc/version.o: $(filter-out misc/version.o,$(pentagram_OBJ))


# install rules

install-bin: pentagram$(EXEEXT)
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) "$(top_builddir)/pentagram$(EXEEXT)" "$(DESTDIR)$(bindir)/pentagram$(EXEEXT)"

ifeq ($(USE_BUILTIN_DATA),yes)
install-data:
else
install-data: $(DATA_FILES)
	$(INSTALL) -d "$(DESTDIR)$(datapath)"
	$(INSTALL_DATA) $(patsubst %,$(top_srcdir)/%,$(DATA_FILES)) "$(DESTDIR)$(datapath)"
endif

install: install-bin install-data


# Common rules
include $(srcdir)/common.mk

.PHONY: install install-bin install-data
