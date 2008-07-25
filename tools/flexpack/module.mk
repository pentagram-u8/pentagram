# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/flexpack

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LCONPRODUCTS := flexpack

include $(srcdir)/objects.mk

flexpack_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(GAMES) \
	$(FILESYS) \
	$(SYSTEM) \
	kernel/CoreApp.o \
	tools/flexpack/FlexWriter.o \
	tools/flexpack/FlexPack.o

# Common rules
include $(srcdir)/common.mk
