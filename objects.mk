KERNEL = \
	kernel/Kernel.o \
	kernel/CoreApp.o \
	kernel/Process.o \
	kernel/idMan.o \
	kernel/GameDetector.o


KERNEL2 = \
	kernel/GameData.o \
	kernel/ObjectManager.o \
	kernel/Object.o

USECODE = \
	usecode/BitSet.o \
	usecode/UCMachine.o \
	usecode/UCProcess.o \
	usecode/Usecode.o \
	usecode/UsecodeFlex.o \
	usecode/UCList.o \
	usecode/UCStack.o

COMPILE = \
	tools/compile/CompileProcess.o \
	tools/compile/Compile.o \
	tools/compile/llcLexer.o

DISASM = \
	tools/disasm/DisasmProcess.o

FILESYS = \
	filesys/FileSystem.o \
	filesys/Flex.o \
	filesys/U8Save.o \
	filesys/Savegame.o \
	filesys/SavegameWriter.o

CONVERT = \
	convert/ConvertShape.o \
	convert/u8/ConvertShapeU8.o \
	convert/crusader/ConvertShapeCrusader.o

CONF = \
	conf/Configuration.o \
	conf/XMLTree.o \
	conf/XMLNode.o

GRAPHICS = \
	graphics/AnimDat.o \
	graphics/RenderSurface.o \
	graphics/BaseSoftRenderSurface.o \
	graphics/Font.o \
	graphics/FontShapeFlex.o \
	graphics/GumpShapeFlex.o \
	graphics/SoftRenderSurface.o \
	graphics/Texture.o \
	graphics/TextureTarga.o \
	graphics/TextureBitmap.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o \
	graphics/Palette.o \
	graphics/PaletteManager.o \
	graphics/PaletteFaderProcess.o \
	graphics/ShapeFlex.o \
	graphics/ShapeInfo.o \
	graphics/MainShapeFlex.o \
	graphics/XFormBlend.o \
	graphics/TypeFlags.o

MISC = \
	misc/Console.o \
	misc/Q_strcasecmp.o \
	misc/pent_include.o \
	misc/util.o

ARGS = \
	misc/Args.o

GUMPS = \
	gumps/AskGump.o \
	gumps/BarkGump.o \
	gumps/ConsoleGump.o \
	gumps/ContainerGump.o \
	gumps/DesktopGump.o \
	gumps/GameMapGump.o \
	gumps/Gump.o \
	gumps/GumpNotifyProcess.o \
	gumps/ItemRelativeGump.o \
	gumps/ResizableGump.o

WIDGETS = \
	gumps/widgets/ButtonWidget.o \
	gumps/widgets/SimpleTextWidget.o

WORLD = \
	world/CameraProcess.o \
	world/Egg.o \
	world/EggHatcherProcess.o \
	world/Item.o \
	world/Container.o \
	world/ItemFactory.o \
	world/ItemSorter.o \
	world/Map.o \
	world/MonsterEgg.o \
	world/CurrentMap.o \
	world/Glob.o \
	world/GlobEgg.o \
	world/TeleportEgg.o \
	world/World.o \
	world/MissileProcess.o \
	world/GravityProcess.o \
	world/SpriteProcess.o

ACTORS = \
	world/actors/Actor.o \
	world/actors/ActorAnimProcess.o \
	world/actors/MainActor.o \
	world/actors/TeleportToEggProcess.o


AUDIO = \
	audio/MusicFlex.o \
	audio/MusicProcess.o

MIDI = \
	audio/midi/XMidiEventList.o \
	audio/midi/XMidiFile.o \
	audio/midi/XMidiSequence.o \
	audio/midi/LowLevelMidiDriver.o \
	audio/midi/fmopl.o \
	audio/midi/FMOplMidiDriver.o \
	audio/midi/CoreAudioMidiDriver.o \
	audio/midi/WindowsMidiDriver.o \
	audio/midi/TimidityMidiDriver.o

TIMIDITY = \
	audio/midi/timidity/timidity.o \
	audio/midi/timidity/timidity_common.o \
	audio/midi/timidity/timidity_controls.o \
	audio/midi/timidity/timidity_filter.o \
	audio/midi/timidity/timidity_instrum.o \
	audio/midi/timidity/timidity_mix.o \
	audio/midi/timidity/timidity_output.o \
	audio/midi/timidity/timidity_playmidi.o \
	audio/midi/timidity/timidity_readmidi.o \
	audio/midi/timidity/timidity_resample.o \
	audio/midi/timidity/timidity_sdl_a.o \
	audio/midi/timidity/timidity_sdl_c.o \
	audio/midi/timidity/timidity_tables.o
