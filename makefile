TARGET ?= console
STATIC ?= 0
PRECISE ?= 1

SHELL := cmd.exe

MINGWBINS := libgcc_s_dw2-1.dll  libstdc++-6.dll

WXINC := extern/wxWidgets-3.1.0/include
WXLIB := extern/wxWidgets-3.1.0/lib

ifneq ($(STATIC),0)
# STATIC = 1
WXDEP := deps/wxWidgets-3.1.0-i686-w64-mingw32-sta-dejbug.7z
WXLIBS := wxmsw31u
WXBINS :=
WINLIBS := advapi32 comctl32 comdlg32 gdi32 kernel32 msvcrt ole32 oleaut32 shell32 shlwapi user32 uuid version winspool
else
# STATIC = 0
WXDEP := deps/wxWidgets-3.1.0-i686-w64-mingw32-dyn-dejbug.7z
WXLIBS := wxbase31u  wxmsw31u_core  wxmsw31u_html
WXBINS := wxbase310u_gcc_custom.dll  wxmsw310u_core_gcc_custom.dll  wxmsw310u_html_gcc_custom.dll
WINLIBS := gdi32 kernel32 msvcrt user32
endif

CXX := g++
CXXFLAGS := -std=c++11 -fabi-version=11 -Wall -Wpedantic -O3 -D__WXMSW__ -I"$(WXINC)" -I"$(WXLIB)\mswu"
LDFLAGS := -Wl,-subsystem=$(TARGET) -L"$(WXLIB)"
LDLIBS := $(addprefix -l,$(WINLIBS))
LDLIBS += $(addprefix -l,$(WXLIBS))

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
ifneq ($(STATIC),0)
deploy/$(NAME).exe : $(WXLIB)/libwxmsw31u.a
else
deploy/$(NAME).exe : $(foreach bin,$(WXBINS),deploy/$(bin))
deploy/$(NAME).exe : $(foreach bin,$(MINGWBINS),deploy/$(bin))
endif
deploy/$(NAME).exe : ; $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(filter %.o %.a,$^) $(LDLIBS)

ifneq ($(PRECISE),0)
include $(patsubst src/%.cpp,build/%.target,$(wildcard src/*.cpp))
endif

build/%.target : src/%.cpp | build ; g++ -MF $@ -MM $< -MT $(subst .target,.o,$@)
build/%.o : src/%.cpp | build ; $(CXX) $(CXXFLAGS) -o $@ -c $<
extern/%.png : deps/open-iconic-master.zip | extern ; 7z e -o$(dir $@) $< */png/$(notdir $@)

build/resource.o : extern/arrow-circle-left-2x.png
build/resource.o : extern/data-transfer-download-2x.png
build/resource.o : extern/clipboard-2x.png
build/resource.o : extern/crop-2x.png
build/resource.o : src/resource.rc src/resource.h
build/resource.o : ; windres src/resource.rc $@

extern/wxWidgets-3.1.0 : extern
extern/wxWidgets-3.1.0 : $(WXDEP)
extern/wxWidgets-3.1.0 : ; IF NOT EXIST $(subst /,\,$@) 7z x -oextern $(filter %.7z,$^)

WXBINS_TARGET := $(foreach bin,$(WXBINS),deploy/$(bin))
$(WXBINS_TARGET) : deploy
$(WXBINS_TARGET) : extern/wxWidgets-3.1.0
$(WXBINS_TARGET) : ; IF NOT EXIST $(subst /,\,$@) COPY extern\wxWidgets-3.1.0\lib\$(notdir $@) deploy

MINGWBINS_TARGET := $(foreach bin,$(MINGWBINS),deploy/$(bin))
$(MINGWBINS_TARGET) : deploy
$(MINGWBINS_TARGET) : deps/MinGW-W64-builds-4.3.0.7z
$(MINGWBINS_TARGET) : ; IF NOT EXIST $(subst /,\,$@) 7z e -odeploy $(filter %.7z,$^) $(notdir $@)
 :

dirs : build deploy extern
build deploy extern : ; IF NOT EXIST $@ MKDIR $@

run : deploy/$(NAME).exe ; $<

clean :
	IF EXIST build RMDIR /S/Q build

reset :
	IF EXIST build RMDIR /S/Q build
	IF EXIST deploy RMDIR /S/Q deploy
	IF EXIST extern RMDIR /S/Q extern
