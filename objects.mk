KERNEL = \
	kernel/DelayProcess.o \
	kernel/Kernel.o \
	kernel/CoreApp.o \
	kernel/Process.o

KERNEL2 = \
	kernel/HIDManager.o \
	kernel/ObjectManager.o \
	kernel/Object.o \
	kernel/Mouse.o \
	kernel/Joystick.o

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
	conf/ConfigFileManager.o \
	conf/INIFile.o \
	conf/SettingManager.o

GAMES = \
	games/GameDetector.o

GAMES2 = \
	games/Game.o \
	games/GameData.o \
	games/TreasureLoader.o \
	games/U8Game.o \
	games/u8bindings.o \
	games/stdbindings.o

GRAPHICS = \
	graphics/AnimDat.o \
	graphics/RenderSurface.o \
	graphics/BaseSoftRenderSurface.o \
	graphics/FrameID.o \
	graphics/GumpShapeFlex.o \
	graphics/SoftRenderSurface.o \
	graphics/Texture.o \
	graphics/TextureTarga.o \
	graphics/TextureBitmap.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o \
	graphics/SKFPlayer.o \
	graphics/Palette.o \
	graphics/PaletteManager.o \
	graphics/PaletteFaderProcess.o \
	graphics/ShapeFlex.o \
	graphics/ShapeInfo.o \
	graphics/MainShapeFlex.o \
	graphics/XFormBlend.o \
	graphics/TypeFlags.o \
	graphics/WpnOvlayDat.o

FONTS = \
	graphics/fonts/Font.o \
	graphics/fonts/FontShapeFlex.o \
	graphics/fonts/RenderedText.o \
	graphics/fonts/ShapeFont.o \
	graphics/fonts/ShapeRenderedText.o \
	graphics/fonts/TTFont.o \
	graphics/fonts/TTFRenderedText.o

MISC = \
	misc/istring.o \
	misc/Console.o \
	misc/idMan.o \
	misc/pent_include.o \
	misc/util.o

ARGS = \
	misc/Args.o

GUMPS = \
	gumps/AskGump.o \
	gumps/BarkGump.o \
	gumps/BindGump.o \
	gumps/BookGump.o \
	gumps/ConsoleGump.o \
	gumps/ContainerGump.o \
	gumps/ControlsGump.o \
	gumps/DesktopGump.o \
	gumps/GameMapGump.o \
	gumps/Gump.o \
	gumps/GumpNotifyProcess.o \
	gumps/ItemRelativeGump.o \
	gumps/MiniStatsGump.o \
	gumps/MenuGump.o \
	gumps/ModalGump.o \
	gumps/MovieGump.o \
	gumps/OptionsGump.o \
	gumps/PagedGump.o \
	gumps/PaperdollGump.o \
	gumps/ReadableGump.o \
	gumps/ResizableGump.o \
	gumps/ScrollGump.o \
	gumps/ShapeViewerGump.o \
	gumps/SliderGump.o \
	gumps/TargetGump.o \
	gumps/QuitGump.o

WIDGETS = \
	gumps/widgets/ButtonWidget.o \
	gumps/widgets/SlidingWidget.o \
	gumps/widgets/TextWidget.o

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
	world/SplitItemProcess.o \
	world/SpriteProcess.o

ACTORS = \
	world/actors/Actor.o \
	world/actors/ActorAnimProcess.o \
	world/actors/AnimAction.o \
	world/actors/Animation.o \
	world/actors/AnimationTracker.o \
	world/actors/AvatarMoverProcess.o \
	world/actors/QuickAvatarMoverProcess.o \
	world/actors/ClearFeignDeathProcess.o \
	world/actors/DeleteActorProcess.o \
	world/actors/LoiterProcess.o \
	world/actors/MainActor.o \
	world/actors/Pathfinder.o \
	world/actors/PathfinderProcess.o \
	world/actors/ResurrectionProcess.o \
	world/actors/TargetedAnimProcess.o \
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
	audio/midi/ALSAMidiDriver.o \
	audio/midi/CoreAudioMidiDriver.o \
	audio/midi/WindowsMidiDriver.o \
	audio/midi/TimidityMidiDriver.o \
	audio/midi/UnixSeqMidiDriver.o

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
