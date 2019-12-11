#Release and debug
ifeq ($(RELEASE), 1)
	CXXFLAGS = -O3 -flto -Wall -Wextra -pedantic
	LDFLAGS = -s
	FILENAME_DEF = release
else
	CXXFLAGS = -Og -ggdb3 -Wall -Wextra -pedantic -DDEBUG
	FILENAME_DEF = debug
endif

FILENAME ?= $(FILENAME_DEF)

PKGCONFIG = pkg-config

ifeq ($(OS), Windows_NT)
	WINDOWS ?= 1
endif

ifeq ($(SWITCH), 1)
	ifeq ($(strip $(DEVKITPRO)),)
		$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
	endif

	WINDOWS = 0

	include $(DEVKITPRO)/libnx/switch_rules
	LIBNX:=$(DEVKITPRO)/libnx
	INCLUDE=-I$(LIBNX)/include -I$(PORTLIBS)/include/SDL2 -I$(PORTLIBS)/include

	CXXFLAGS += -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -DSWITCH $(INCLUDE)
	LIBS += -specs=$(DEVKITPRO)/libnx/switch.specs -L$(LIBNX)/lib -lnx -lm
	PKGCONFIG = $(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config

	ifdef ENABLE_NXLINK
		CXXFLAGS += -DENABLE_NXLINK
	endif
endif

ifeq ($(WINDOWS), 1)
	CXXFLAGS += -DWINDOWS
	
	ifneq ($(RELEASE), 1)
		CXXFLAGS += -mconsole
	endif
endif

#Use SDL2 as the default backend if one wasn't explicitly chosen
BACKEND ?= SDL2

#Backend flags and libraries
ifeq ($(BACKEND), SDL2)
	CXXFLAGS += `$(PKGCONFIG) --cflags sdl2` -DBACKEND_SDL2

	ifeq ($(STATIC), 1)
		LDFLAGS += -static
		LIBS += `$(PKGCONFIG) --libs sdl2 --static`
	else
		LIBS += `$(PKGCONFIG) --libs sdl2`
	endif
endif

#Other CXX flags
CXXFLAGS += -faligned-new -MMD -MP -MF $@.d

#Sources to compile
SOURCES = \
	Main \
	MathUtil \
	Fade \
	Mappings \
	Level \
	LevelCollision \
	SpecialStage \
	Background \
	Game \
	GM_Splash \
	GM_Title \
	GM_Game \
	GM_SpecialStage \
	Player \
	Object \
	Camera \
	TitleCard \
	Hud \
	LevelSpecific/GHZ \
	LevelSpecific/EHZ \
	Objects/PathSwitcher \
	Objects/Ring \
	Objects/BouncingRing \
	Objects/AttractRing \
	Objects/Monitor \
	Objects/Spring \
	Objects/Explosion \
	Objects/Bridge \
	Objects/Goalpost \
	Objects/Spiral \
	Objects/Sonic1Scenery \
	Objects/Motobug \
	Objects/Chopper \
	Objects/Crabmeat \
	Objects/BuzzBomber \
	Objects/Newtron \
	Objects/GHZWaterfallSound \
	Objects/GHZPlatform \
	Objects/GHZLedge \
	Objects/GHZSwingingPlatform \
	Objects/GHZSpikes \
	Objects/GHZEdgeWall \
	Objects/GHZSmashableWall \
	Objects/PurpleRock \
	Objects/Minecart \
	Audio_stb_vorbis \
	Audio_miniaudio

#Backend source files
ifeq ($(BACKEND), SDL2)
	SOURCES += \
		Backend_SDL2/Error \
		Backend_SDL2/Filesystem \
		Backend_SDL2/Render \
		Backend_SDL2/Event \
		Backend_SDL2/Input \
		Backend_SDL2/Audio
endif

#What to compile
OBJECTS = $(addprefix obj/$(FILENAME)/, $(addsuffix .o, $(SOURCES)))
DEPENDENCIES = $(addprefix obj/$(FILENAME)/, $(addsuffix .o.d, $(SOURCES)))

#If compiling a windows build, add the Windows icon object into our executable
ifeq ($(WINDOWS), 1)
	OBJECTS += obj/$(FILENAME)/WindowsIcon.o
endif

#Compilation code
ifeq ($(SWITCH), 1)
all: build/$(FILENAME).nro

build/$(FILENAME).nro: build/$(FILENAME)
	@elf2nro $< $@ $(NROFLAGS)
	@echo built ... $(notdir $@)

else
all: build/$(FILENAME)
endif

build/$(FILENAME): $(OBJECTS)
	@mkdir -p $(@D)
	@echo Linking...
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	@echo Finished linking $@

obj/$(FILENAME)/%.o: src/%.cpp
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) $< -o $@ -c
	@echo Compiled!
	
include $(wildcard $(DEPENDENCIES))

#Compile the Windows icon file into an object
obj/$(FILENAME)/WindowsIcon.o: res/icon.rc res/icon.ico
	@mkdir -p $(@D)
	@windres $< $@

#Remove all our compiled objects
clean:
	@rm -rf obj