# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/flexpack

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LPRODUCTS := flexpack

include $(srcdir)/objects.mk

flexpack_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(KERNEL) \
	$(GAMES) \
	filesys/FileSystem.o \
	filesys/RawArchive.o \
	filesys/Archive.o \
	filesys/ArchiveFile.o \
	filesys/FlexFile.o \
	filesys/ZipFile.o \
	filesys/U8SaveFile.o \
	filesys/DirFile.o \
	filesys/zip/ioapi.o \
	filesys/zip/unzip.o \
	tools/flexpack/FlexWriter.o \
	tools/flexpack/FlexPack.o

# Common rules
include $(srcdir)/common.mk
