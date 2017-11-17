SHELL := cmd.exe
TARGET ?= console
WINLIBS := kernel32 gdi32 ole32 oleaut32 uuid comdlg32 winspool comctl32 shlwapi version
WXINC := extern/wxWidgets-3.1.0/include
WXLIB := extern/wxWidgets-3.1.0/lib
CXX := g++
CXXFLAGS := -std=c++11 -fabi-version=11 -Wall -Wpedantic -O3 -D__WXMSW__ -I"$(WXINC)" -I"$(WXLIB)\mswu"
LDFLAGS := -Wl,-subsystem=$(TARGET) -L"$(WXLIB)"
LDLIBS := $(addprefix -l,$(WINLIBS)) -lwxmsw31u

NAME := wender

.PHONY : all dirs run clean reset

deploy/$(NAME).exe : build deploy
deploy/$(NAME).exe : extern/wxWidgets-3.1.0
deploy/$(NAME).exe : build/main.o
deploy/$(NAME).exe : build/App.o
deploy/$(NAME).exe : build/MainFrame.o
deploy/$(NAME).exe : build/FontList.o
deploy/$(NAME).exe : build/FontList2.o
deploy/$(NAME).exe : build/ProgressBar.o
deploy/$(NAME).exe : build/resource.o
deploy/$(NAME).exe : build/util.o
deploy/$(NAME).exe : build/lib_font.o
deploy/$(NAME).exe : $(WXLIB)/libwxmsw31u.a
deploy/$(NAME).exe : ; $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter %.o %.a,$^) $(LDLIBS)

build/%.o : src/%.cpp ; $(CXX) $(CXXFLAGS) -o $@ -c $<
extern/%.png : deps/open-iconic-master.zip ; 7z e -o$(dir $@) $< */png/$(notdir $@)

build/main.o : src/main.cpp src/resource.h
build/lib_font.o : src/lib_font.cpp src/lib_font.h

extern/wxWidgets-3.1.0 : extern
extern/wxWidgets-3.1.0 : deps/wxWidgets-3.1.0-i686-w64-mingw32-dejbug.7z
extern/wxWidgets-3.1.0 : ; IF NOT EXIST $(subst /,\,$@) 7z x -oextern $(filter %.7z,$^)

build/resource.o : extern/arrow-circle-left-2x.png
build/resource.o : extern/data-transfer-download-2x.png
build/resource.o : extern/clipboard-2x.png
build/resource.o : extern/crop-2x.png
build/resource.o : src/resource.rc src/resource.h
build/resource.o : ; windres src/resource.rc $@

dirs : build deploy extern
build deploy extern : ; IF NOT EXIST $@ MKDIR $@

run : deploy/$(NAME).exe ; $<

clean :
	IF EXIST build RMDIR /S/Q build

reset :
	IF EXIST build RMDIR /S/Q build
	IF EXIST deploy RMDIR /S/Q deploy
	IF EXIST extern RMDIR /S/Q extern
