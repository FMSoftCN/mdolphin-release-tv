bin_PROGRAMS = mdtv

INCLUDES =  -I../svgui/ 

DEFS += -D_INST_PRX='"$(prefix)"'

mdtv_SOURCES = mdtv_app.cpp mdtv_app.h  mdtv_dbbuffer.cpp mdtv_dbbuffer.h \
			mdtv_common.h mdtv_common.cpp \
			mdtv_toolbar.cpp mdtv_toolbar.h mdtv_animation.cpp mdtv_animation.h \
			mdtv_toolbar_zoom.cpp mdtv_toolbar_zoom.h\
			mdtv_weather.cpp mdtv_weather.h\
			mdtv_finance.cpp mdtv_finance.h\
			mdtv_network.cpp mdtv_network.h\
			mdtv_setting.cpp mdtv_setting.h\
			mdtv_search.cpp mdtv_search.h\
			mdtv_fav.cpp mdtv_fav.h\
			mdtv_fav_file.cpp mdtv_fav_file.h\
			mdtv_browser.cpp  mdtv_browser.h \
	       	mdtv_errcode.cpp prompt_dialog.cpp \
	       	mdtv_ime.cpp mdtv_ime.h  mdtv_mgphone.cpp \
	       	mdtv_keyboard.cpp mdtv_keyboard.h \
	       	mdtv_website.cpp mdtv_website.h \
			mdtv_key.cpp mdtv_key.h \
			mdtv_setting_homepage.cpp mdtv_setting_homepage.h \
			mdtv_tooltipwin.cpp mdtv_tooltipwin.h

if MINIGUI_V3
MGLIBS= -lmgutils -lmgplus -lmgi -lxml2
else
MGLIBS= -lmgext
endif

mdtv_LDFLAGS =  ../svgui/libsvgui.a $(MGLIBS) $(LIBS) 

