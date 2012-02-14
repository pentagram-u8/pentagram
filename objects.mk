KERNEL = \
	kernel/Allocator.o \
	kernel/CoreApp.o \
	kernel/DelayProcess.o \
	kernel/HIDManager.o \
	kernel/HIDKeys.o \
	kernel/Joystick.o \
	kernel/Kernel.o \
	kernel/MemoryManager.o \
	kernel/Mouse.o \
	kernel/Object.o \
	kernel/ObjectManager.o \
	kernel/Process.o \
	kernel/Pool.o \
	kernel/SegmentedAllocator.o \
	kernel/SegmentedPool.o

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
	filesys/Archive.o \
	filesys/ArchiveFile.o \
	filesys/DirFile.o \
	filesys/FileSystem.o \
	filesys/FlexFile.o \
	filesys/RawArchive.o \
	filesys/U8SaveFile.o \
	filesys/Savegame.o \
	filesys/SavegameWriter.o \
	filesys/ZipFile.o \
	filesys/data.o \
	filesys/zip/unzip.o \
	filesys/zip/ioapi.o \
	filesys/zip/zip.o

CONVERT = \
	convert/ConvertShape.o \
	convert/u8/ConvertShapeU8.o \
	convert/crusader/ConvertShapeCrusader.o

CONF = \
	conf/ConfigFileManager.o \
	conf/INIFile.o \
	conf/SettingManager.o

GAMES = \
	games/GameDetector.o \
	games/GameInfo.o

GAMES2 = \
	games/Game.o \
	games/GameData.o \
	games/RemorseGame.o \
	games/StartU8Process.o \
	games/TreasureLoader.o \
	games/U8Game.o

GRAPHICS = \
	graphics/AnimDat.o \
	graphics/RenderSurface.o \
	graphics/BaseSoftRenderSurface.o \
	graphics/FrameID.o \
	graphics/GumpShapeArchive.o \
	graphics/InverterProcess.o \
	graphics/SoftRenderSurface.o \
	graphics/Texture.o \
	graphics/TextureTarga.o \
	graphics/TextureBitmap.o \
	graphics/TexturePNG.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o \
	graphics/SKFPlayer.o \
	graphics/Palette.o \
	graphics/PaletteManager.o \
	graphics/PaletteFaderProcess.o \
	graphics/PNGWriter.o \
	graphics/ShapeArchive.o \
	graphics/ShapeInfo.o \
	graphics/MainShapeArchive.o \
	graphics/XFormBlend.o \
	graphics/TypeFlags.o \
	graphics/WpnOvlayDat.o \
	graphics/ScalerManager.o

SCALERS = \
	graphics/scalers/BilinearScaler.o \
	graphics/scalers/BilinearScalerInternal_2x.o \
	graphics/scalers/BilinearScalerInternal_Arb.o \
	graphics/scalers/BilinearScalerInternal_X1Y12.o \
	graphics/scalers/BilinearScalerInternal_X2Y24.o \
	graphics/scalers/PointScaler.o \
	graphics/scalers/Scale2xScaler.o \
	graphics/scalers/hq2xScaler.o \
	graphics/scalers/hq2xScaler_16Nat.o \
	graphics/scalers/hq2xScaler_16Sta.o \
	graphics/scalers/hq2xScaler_32Nat.o \
	graphics/scalers/hq2xScaler_32Sta.o \
	graphics/scalers/hq2xScaler_888A.o \
	graphics/scalers/hq2xScaler_A888.o \
	graphics/scalers/hq3xScaler.o \
	graphics/scalers/hq3xScaler_16Nat.o \
	graphics/scalers/hq3xScaler_16Sta.o \
	graphics/scalers/hq3xScaler_32Nat.o \
	graphics/scalers/hq3xScaler_32Sta.o \
	graphics/scalers/hq3xScaler_888A.o \
	graphics/scalers/hq3xScaler_A888.o \
	graphics/scalers/hq4xScaler.o \
	graphics/scalers/hq4xScaler_16Nat.o \
	graphics/scalers/hq4xScaler_16Sta.o \
	graphics/scalers/hq4xScaler_32Nat.o \
	graphics/scalers/hq4xScaler_32Sta.o \
	graphics/scalers/hq4xScaler_888A.o \
	graphics/scalers/hq4xScaler_A888.o \
	graphics/scalers/hqScaler.o \
	graphics/scalers/2xSaIScalers.o

FONTS = \
	graphics/fonts/Font.o \
	graphics/fonts/FontManager.o \
	graphics/fonts/FontShapeArchive.o \
	graphics/fonts/JPFont.o \
	graphics/fonts/JPRenderedText.o \
	graphics/fonts/RenderedText.o \
	graphics/fonts/ShapeFont.o \
	graphics/fonts/ShapeRenderedText.o \
	graphics/fonts/TTFont.o \
	graphics/fonts/TTFRenderedText.o \
	graphics/fonts/FixedWidthFont.o

MISC = \
	misc/encoding.o \
	misc/istring.o \
	misc/Console.o \
	misc/idMan.o \
	misc/md5.o \
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
	gumps/CreditsGump.o \
	gumps/DesktopGump.o \
	gumps/FastAreaVisGump.o \
	gumps/GameMapGump.o \
	gumps/Gump.o \
	gumps/GumpNotifyProcess.o \
	gumps/InverterGump.o \
	gumps/ItemRelativeGump.o \
	gumps/MainMenuProcess.o \
	gumps/MenuGump.o \
	gumps/MiniMapGump.o \
	gumps/MiniStatsGump.o \
	gumps/ModalGump.o \
	gumps/MovieGump.o \
	gumps/OptionsGump.o \
	gumps/PagedGump.o \
	gumps/PaperdollGump.o \
	gumps/PentagramMenuGump.o \
	gumps/QuitGump.o \
	gumps/ReadableGump.o \
	gumps/ResizableGump.o \
	gumps/ScalerGump.o \
	gumps/ScrollGump.o \
	gumps/ShapeViewerGump.o \
	gumps/SliderGump.o \
	gumps/TargetGump.o \
	gumps/U8SaveGump.o \
	gumps/MessageBoxGump.o

WIDGETS = \
	gumps/widgets/ButtonWidget.o \
	gumps/widgets/EditWidget.o \
	gumps/widgets/GameWidget.o \
	gumps/widgets/SlidingWidget.o \
	gumps/widgets/TextWidget.o

WORLD = \
	world/CameraProcess.o \
	world/Container.o \
	world/CreateItemProcess.o \
	world/CurrentMap.o \
	world/DestroyItemProcess.o \
	world/Egg.o \
	world/EggHatcherProcess.o \
	world/FireballProcess.o \
	world/MapGlob.o \
	world/GlobEgg.o \
	world/GravityProcess.o \
	world/Item.o \
	world/ItemFactory.o \
	world/ItemSorter.o \
	world/Map.o \
	world/MissileProcess.o \
	world/MissileTracker.o \
	world/MonsterEgg.o \
	world/SplitItemProcess.o \
	world/SpriteProcess.o \
	world/TeleportEgg.o \
	world/World.o \
	world/getObject.o

ACTORS = \
	world/actors/Actor.o \
	world/actors/ActorAnimProcess.o \
	world/actors/ActorBarkNotifyProcess.o \
	world/actors/AmbushProcess.o \
	world/actors/AnimAction.o \
	world/actors/Animation.o \
	world/actors/AnimationTracker.o \
	world/actors/AvatarDeathProcess.o \
	world/actors/AvatarGravityProcess.o \
	world/actors/AvatarMoverProcess.o \
	world/actors/ClearFeignDeathProcess.o \
	world/actors/CombatProcess.o \
	world/actors/GrantPeaceProcess.o \
	world/actors/HealProcess.o \
	world/actors/LoiterProcess.o \
	world/actors/MainActor.o \
	world/actors/Pathfinder.o \
	world/actors/PathfinderProcess.o \
	world/actors/QuickAvatarMoverProcess.o \
	world/actors/ResurrectionProcess.o \
	world/actors/SchedulerProcess.o \
	world/actors/TargetedAnimProcess.o \
	world/actors/TeleportToEggProcess.o

AUDIO = \
	audio/MusicFlex.o \
	audio/MusicProcess.o \
	audio/AudioChannel.o \
	audio/AudioMixer.o \
	audio/AudioProcess.o \
	audio/AudioSample.o \
	audio/RawAudioSample.o \
	audio/SonarcAudioSample.o \
	audio/SoundFlex.o \
	audio/SpeechFlex.o

MIDI = \
	audio/midi/XMidiEventList.o \
	audio/midi/XMidiFile.o \
	audio/midi/XMidiSequence.o \
	audio/midi/MidiDriver.o \
	audio/midi/LowLevelMidiDriver.o \
	audio/midi/fmopl.o \
	audio/midi/FMOplMidiDriver.o \
	audio/midi/ALSAMidiDriver.o \
	audio/midi/CoreAudioMidiDriver.o \
        audio/midi/CoreMidiDriver.o \
	audio/midi/WindowsMidiDriver.o \
	audio/midi/TimidityMidiDriver.o \
	audio/midi/UnixSeqMidiDriver.o \
	audio/midi/FluidSynthMidiDriver.o

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

SYSTEM_MACOSX = \
	system/macosx/macosx_utils.o

DATA_FILES = \
	data/fixedfont.ini \
	data/fixedfont.tga \
	data/remorsebindings.ini \
	data/u8.ini \
	data/u8armour.ini \
	data/u8weapons.ini \
	data/u8monsters.ini \
	data/u8french.ini \
	data/u8german.ini \
	data/u8japanese.ini \
	data/u8spanish.ini \
	data/u8bindings.ini \
	data/VeraBd.ttf \
	data/Vera.ttf \
	data/mouse.tga \
	data/title.png \
	data/navbar.png \
	data/flags.png \
	data/covers.png
