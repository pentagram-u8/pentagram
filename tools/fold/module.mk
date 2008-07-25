# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := tools/fold

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LCONPRODUCTS := fold

fold_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(GAMES) \
	$(SYSTEM) \
	kernel/CoreApp.o \
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
	tools/fold/Fold.o \
	tools/fold/Folder.o \
	tools/fold/Type.o \
	tools/fold/IfNode.o \
	tools/fold/OperatorNodes.o \
	tools/fold/CallNodes.o \
	tools/fold/VarNodes.o \
	tools/fold/FuncNodes.o \
	tools/fold/LoopScriptNodes.o

# Common rules
include $(srcdir)/common.mk
