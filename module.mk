# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := pentagram
LINSTALL := pentagram

pentagram_OBJ = \
	pentagram.o \
	kernel/Kernel.o \
	kernel/Application.o \
	kernel/Process.o \
	kernel/Object.o \
	usecode/UCMachine.o \
	usecode/UCProcess.o \
	usecode/UsecodeFlex.o \
	usecode/UCList.o \
	filesys/FileSystem.o \
	filesys/Flex.o \
	misc/Args.o \
	misc/Console.o \
	misc/Q_strcasecmp.o \
	misc/pent_include.o \
	misc/util.o \
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
	graphics/MainShapeFlex.o \
	graphics/XFormBlend.o \
	graphics/TypeFlags.o \
	gumps/ConsoleGump.o \
	gumps/GameMapGump.o \
	gumps/Gump.o \
	gumps/ResizableGump.o \
	world/Item.o \
	world/Container.o \
	world/ItemFactory.o \
	world/Map.o \
	world/CurrentMap.o \
	world/Glob.o \
	world/GlobEgg.o \
	world/World.o \
	world/actors/Actor.o \
	world/actors/MainActor.o \
	convert/ConvertShape.o \
	convert/u8/ConvertShapeU8.o \
	convert/crusader/ConvertShapeCrusader.o \
	conf/Configuration.o \
	conf/XMLTree.o \
	conf/XMLNode.o

# Common rules
include common.mk
