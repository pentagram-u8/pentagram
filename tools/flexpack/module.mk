# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/flexpack

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := flexpack

include $(srcdir)/objects.mk

flexpack_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(KERNEL) \
	$(GAMES) \
	filesys/FileSystem.o \
	filesys/Flex.o \
	tools/flexpack/FlexWriter.o \
	tools/flexpack/FlexPack.o


# ../../filesys/Flex.o
# Common rules
include $(srcdir)/common.mk
