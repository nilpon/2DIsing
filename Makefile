CXX = x86_64-w64-mingw32-g++

CXXFLAGS =  -Wall -std=c++1z -I/usr/local/cross-tools/x86_64-w64-mingw32/include

OBJDIR = ./obj
BINDIR = ./bin

LIBS = \
	`/usr/local/cross-tools/x86_64-w64-mingw32/bin/sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lz -lopengl32

LDFLAGS = \
	-L/usr/local/cross-tools/x86_64-w64-mingw32/lib -static-libgcc -static-libstdc++

TARGET = wallpaper.exe

SOURCES = $(wildcard *.cpp)
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))
BINS = $(addprefix $(BINDIR)/, $(TARGET))
DEPS = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.d))

.PHONY: debug
debug: CXXFLAGS += -g -O0
debug: all

.PHONY: release
release: CXXFLAGS += -O2
release: LDFLAGS += -s
release: all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@if [ ! -d $(BINDIR) ]; then \
		mkdir $(BINDIR); \
	fi
	$(CXX) -o $(BINDIR)/$@ $^ $(LIBS) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@if [ ! -d $(OBJDIR) ]; then \
		mkdir $(OBJDIR); \
	fi
	@[ -d $(OBJDIR) ]
	$(CXX) $(CXXFLAGS) -o $@ -c -MMD -MP $<

-include $(DEPS)

.PHONY: clean

clean:
	-rm $(OBJECTS) $(BINS) $(DEPS)
