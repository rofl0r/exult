# Exult makefile for use in BeOS
# It may require a little tweaking. (paths)

# Base of the exult source
SRC=.
VPATH=$(SRC):$(SRC)/files:$(SRC)/pathfinder:$(SRC)/flic:$(SRC)/conf:$(SRC)/audio:$(SRC)/audio/midi_drivers

### Modify these paths
SDL_INCLUDES=-I/boot/develop/tools/gnupro/include/SDL
SDL_LIBS=-L/boot/develop/tools/gnupro/lib -lSDLmain -lSDL

CPPFLAGS=-DRELNUM=26 -DBEOS -DDEBUG -DAUTOCONFIGURED -I$(SRC)/files \
	-I$(SRC) -I$(SRC)/audio -I$(SRC)/conf -I$(SRC)/pathfinder \
	$(SDL_INCLUDES)
CXXFLAGS=-g -Wall

LFLAGS=-g
LIBS=$(SDL_LIBS) -lmidi -lbe

EXEC=exult
MAIN_OBJS=actions.o actorio.o actors.o args.o delobjs.o drag.o effects.o \
	exult.o gameclk.o gamedat.o \
	gametxt.o gamewin.o gumps.o imagewin.o items.o lists.o mouse.o \
	npcnear.o objs.o readnpcs.o segfile.o spells.o tqueue.o usecode.o \ 
	utils.o vec.o vgafile.o
# unused: npctest.o
PATH_OBJS=Astar.o PathFinder.o Zombie.o path.o
CONF_OBJS=Configuration.o XMLEntity.o xmldump.o xmlparse.o
# unused: xmain.o
MIDI_DRV_OBJS=be_midi.o
# unused: KMIDI.o Timidity_binary.o forked_player.o win_MCI.o
AUDIO_OBJS=Audio.o Midi.o Mixer.o SDL_mapping.o $(MIDI_DRV_OBJS)
# unused: test.o u7audiotool.o
FLIC_OBJS=playfli.o
FILE_OBJS=U7file.o Flex.o IFF.o Table.o
# unused: test.o
OBJS=$(MAIN_OBJS) $(PATH_OBJS) $(CONF_OBJS) $(AUDIO_OBJS) $(FLIC_OBJS) $(FILE_OBJS)

$(EXEC) : Makefile $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

Makefile: Makefile.be
	cp Makefile.be Makefile

clean:
	rm -f $(OBJS) $(EXEC)

install:
	strip $(EXEC)

run:
	./$(EXEC)


