noinst_LIBRARIES = libsvgui.a
noinst_PROGRAMS = svgui_test

libsvgui_a_SOURCES = svgui.h svgui.c

svgui_test_SOURCES = svgui_test.c svgui.c

if MINIGUI_V3
MGLIBS=-lmgutils -lmgplus
else
MGLIBS=-lmgext
endif

svgui_test_LDFLAGS = $(MGLIBS) $(LIBS)
