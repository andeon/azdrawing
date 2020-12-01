# debug mode : make debug
# disable XInput2 : make xi2=off


###########################
# dir
###########################

prefix := /usr/local

bindir  := $(prefix)/bin
datadir := $(prefix)/share/azdrawing
menudir := $(prefix)/share/applications
icondir := $(prefix)/share/icons/hicolor/48x48/apps

freetype_dir := /usr/include/freetype2


###########################
# macro
###########################

TARGET := azdrawing
AXLIB  := azxclass/libazxc.a

CXX := g++ -pthread
CXXFLAGS := -Wall -O2
LFLAGS := -s
LINKS := -lX11 -lXext -lXft -lfontconfig -lfreetype -lz -ljpeg -lrt
MACRO := -D_AZDRAW_RESDIR="\"$(datadir)\""
MACRO_OPT :=
INCLUDE := -Iazxclass/include -I$(freetype_dir)
INCLUDE_MAIN := -Isrc/include $(INCLUDE)

AR := ar rv
RM := rm -f
INSTALL_DAT := install -v -m 644
INSTALL_DIR := install -dv

SRCS := $(wildcard src/*.cpp)
SRCS += $(wildcard src/dat/*.cpp)
SRCS += $(wildcard src/draw/*.cpp)
SRCS += $(wildcard src/img/*.cpp)
SRCS += $(wildcard src/win/*.cpp)

OBJS := $(SRCS:%.cpp=%.o)

OBJS_AXLIB := $(wildcard azxclass/src/*.cpp)
OBJS_AXLIB := $(OBJS_AXLIB:%.cpp=%.o)

DATAFILES := files/image.tar files/azdrawing.png files/brush_def.dat \
  files/ja_JP.axt files/zh_CN.axt 

###########################
# option
###########################

ifneq ($(xi2),off)
  MACRO_OPT += -D_AX_OPT_XI2
  LINKS += -lXi
endif

###########################
# rule
###########################

.PHONY: all debug clean install uninstall

all: $(TARGET)

debug: CXXFLAGS=-g -Wall
debug: LFLAGS=
debug: all

$(TARGET): $(AXLIB) $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $(OBJS) $(AXLIB) $(LINKS)

$(AXLIB): $(OBJS_AXLIB)
	$(AR) $@ $^

azxclass/src/%.o: azxclass/src/%.cpp
	$(CXX) $(CXXFLAGS) $(MACRO_OPT) $(INCLUDE) -c $< -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(MACRO) $(MACRO_OPT) $(INCLUDE_MAIN) -c $< -o $@

clean:
	-$(RM) $(TARGET) $(AXLIB) $(OBJS) $(OBJS_AXLIB)

install:
	@install -Dv $(TARGET) $(bindir)/$(TARGET)
	@$(INSTALL_DIR) $(datadir)
	@$(INSTALL_DAT) $(DATAFILES) $(datadir)
	cp -r ./files/brush $(datadir)
	cp -r ./files/texture $(datadir)
	@install -Dv -m 644 ./files/azdrawing.desktop $(menudir)/azdrawing.desktop
	@install -Dv -m 644 ./files/azdrawing.png $(icondir)/azdrawing.png
	@echo "Updating GTK icon cache."
	-gtk-update-icon-cache -f -t $(prefix)/share/icons/hicolor

uninstall:
	-$(RM) $(bindir)/$(TARGET)
	-rm -fr $(datadir)
	-$(RM) $(menudir)/azdrawing.desktop
	-$(RM) $(icondir)/azdrawing.png
