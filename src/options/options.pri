# base dialog stuff
HEADERS += \
	$$PWD/optionsdlg.h \
	$$PWD/optionstab.h 
SOURCES += \
	$$PWD/optionstab.cpp \
	$$PWD/optionsdlg.cpp 
INTERFACES += \
	$$PWD/ui_options.ui


# additional tabs
HEADERS += \
	$$PWD/opt_application.h \
	$$PWD/opt_chat.h \
	$$PWD/opt_events.h \
	$$PWD/opt_status.h \
	$$PWD/opt_appearance.h \
	$$PWD/opt_iconset.h \
	$$PWD/opt_groupchat.h \
	$$PWD/opt_sound.h \
	$$PWD/opt_toolbars.h \
	$$PWD/opt_advanced.h \
	$$PWD/opt_shortcuts.h

HEADERS += $$PWD/opt_tree.h

SOURCES += \
	$$PWD/opt_application.cpp \
	$$PWD/opt_chat.cpp \
	$$PWD/opt_events.cpp \
	$$PWD/opt_status.cpp \
	$$PWD/opt_appearance.cpp \
	$$PWD/opt_iconset.cpp \
	$$PWD/opt_groupchat.cpp \
	$$PWD/opt_sound.cpp \
	$$PWD/opt_toolbars.cpp \
	$$PWD/opt_advanced.cpp \
	$$PWD/opt_shortcuts.cpp

SOURCES += $$PWD/opt_tree.cpp

INTERFACES += \
	$$PWD/opt_application.ui \
	$$PWD/opt_chat.ui \
	$$PWD/opt_events.ui \
	$$PWD/opt_status.ui \
	$$PWD/opt_appearance.ui \
	$$PWD/opt_appearance_misc.ui \
	$$PWD/opt_sound.ui \
	$$PWD/opt_advanced.ui \
	$$PWD/opt_lookfeel_toolbars.ui \
	$$PWD/ui_positiontoolbar.ui \
	$$PWD/ui_isdetails.ui \
	$$PWD/opt_iconset_emo.ui \
	$$PWD/opt_iconset_system.ui \
	$$PWD/opt_iconset_roster.ui \
	$$PWD/opt_general_groupchat.ui \
	$$PWD/opt_shortcuts.ui

psi_plugins {
	INTERFACES += $$PWD/opt_plugins.ui
	SOURCES += $$PWD/opt_plugins.cpp
	HEADERS += $$PWD/opt_plugins.h
}

INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD
