# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LPRODUCTS := pentagram llc
LINSTALL := pentagram

include $(srcdir)/objects.mk

pentagram_OBJ = \
	$(KERNEL) \
	$(KERNEL2) \
	$(USECODE) \
	$(FILESYS) \
	$(GAMES) \
	$(GAMES2) \
	$(GRAPHICS) \
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
	pentagram.o

llc_OBJ = \
	$(KERNEL) \
	$(KERNEL2) \
	$(USECODE) \
	$(FILESYS) \
	$(GAMES) \
	$(GAMES2) \
	$(GRAPHICS) \
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

# Common rules
include $(srcdir)/common.mk
