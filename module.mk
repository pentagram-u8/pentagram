# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := pentagram
LINSTALL := pentagram

KERNEL = \
	kernel/Kernel.o \
	kernel/Application.o \
	kernel/Process.o \
	kernel/Object.o \
	kernel/GameData.o \
	kernel/idMan.o 

USECODE = \
	usecode/UCMachine.o \
	usecode/UCProcess.o \
	usecode/Usecode.o \
	usecode/UsecodeFlex.o \
	usecode/UCList.o

COMPILE = \
	tools/compile/CompileProcess.o \
	tools/compile/Compile.o \
	tools/compile/llcLexer.o

FILESYS = \
	filesys/FileSystem.o \
	filesys/Flex.o \
	filesys/U8Save.o

CONVERT = \
	convert/ConvertShape.o \
	convert/u8/ConvertShapeU8.o \
	convert/crusader/ConvertShapeCrusader.o

CONF = \
	conf/Configuration.o \
	conf/XMLTree.o \
	conf/XMLNode.o

pentagram_OBJ = \
	$(KERNEL) \
	$(USECODE) \
	$(FILESYS) \
	misc/Console.o \
	misc/Args.o \
	misc/Q_strcasecmp.o \
	misc/pent_include.o \
	misc/util.o \
	graphics/AnimDat.o \
	graphics/RenderSurface.o \
	graphics/BaseSoftRenderSurface.o \
	graphics/SoftRenderSurface.o \
	graphics/Texture.o \
	graphics/TextureTarga.o \
	graphics/TextureBitmap.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o \
	graphics/Palette.o \
	graphics/PaletteManager.o \
	graphics/ShapeFlex.o \
	graphics/ShapeInfo.o \
	graphics/MainShapeFlex.o \
	graphics/XFormBlend.o \
	graphics/TypeFlags.o \
	gumps/ConsoleGump.o \
	gumps/GameMapGump.o \
	gumps/Gump.o \
	gumps/ResizableGump.o \
	world/Egg.o \
	world/Item.o \
	world/Container.o \
	world/ItemFactory.o \
	world/ItemSorter.o \
	world/Map.o \
	world/CurrentMap.o \
	world/Glob.o \
	world/GlobEgg.o \
	world/World.o \
	world/ItemMoveProcess.o \
	world/actors/Actor.o \
	world/actors/ActorAnimProcess.o \
	world/actors/MainActor.o \
	$(CONVERT) \
	$(CONF) \
	tools/disasm/DisasmProcess.o \
	$(COMPILE) \
	pentagram.o 

# Common rules
include common.mk
