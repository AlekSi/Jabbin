#
# Psi qmake profile
#

# Configuration
TEMPLATE = app
TARGET   = jabbin
CONFIG  += qt thread x11 qt3support

# code coverage
#coverage.CONFIG += recursive
#QMAKE_EXTRA_TARGETS += coverage

#CONFIG(debug,debug|release) {
#    QMAKE_EXTRA_TARGETS += cov_cxxflags cov_lflags
#    cov_cxxflags.target  = coverage
#    cov_cxxflags.depends = CXXFLAGS += -fprofile-arcs -ftest-coverage
#    cov_lflags.target  = coverage
#    cov_lflags.depends = LFLAGS += -fprofile-arcs -ftest-coverage
#    coverage.commands = @echo "Built with coverage support..."
#    build_pass|!debug_and_release:coverage.depends = all
#    QMAKE_CLEAN += $(OBJECTS_DIR)/*.gcda $(OBJECTS_DIR)/*.gcno
#}

# end code coverage

windows: include(../conf.pri)

yapsi_activex_server {
	CONFIG  += qaxserver qaxcontainer
	contains(CONFIG, static): DEFINES += QT_NODLL

	# DEF_FILE = ../win32/jabbinpsiserver.def
	contains(QMAKE_HOST.arch, x86_64) {
		RC_FILE  = ../win64/jabbinpsiserver.rc
	} else {
		RC_FILE  = ../win32/jabbinpsiserver.rc
	}
}

!yapsi_activex_server:win32 {
	contains(QMAKE_HOST.arch, x86_64) {
		RC_FILE = ../win64/psi.rc
	} else {
		RC_FILE = ../win32/psi.rc
	}
}

QT += xml network qt3support phonon script webkit

#CONFIG += use_crash
#CONFIG += console
CONFIG += pep
CONFIG += jingle
DEFINES += QT_STATICPLUGIN

# Import several very useful Makefile targets
# as well as set up default directories for
# generated files
include(../qa/valgrind/valgrind.pri)
include(../qa/oldtest/unittest.pri)

unix {
	include(../conf.pri)

	# Target
	target.path = $$BINDIR
	INSTALLS += target

	# Shared files
	sharedfiles.path  = $$PSI_DATADIR
	sharedfiles.files = ../COPYING ../certs
	INSTALLS += sharedfiles

	# Widgets
	#widgets.path = $$PSI_DATADIR/designer
	#widgets.files = ../libpsi/psiwidgets/libpsiwidgets.so
	#INSTALLS += widgets

	# icons and desktop files
	dt.path=$$PREFIX/share/applications/
	dt.extra    = cp -f ../psi.desktop $(INSTALL_ROOT)$$dt.path/jabbin.desktop
	icon1.path=$$PREFIX/share/icons/hicolor/16x16/apps
	icon1.extra = cp -f ../iconsets/system/default/logo_16.png $(INSTALL_ROOT)$$icon1.path/jabbin.png
	icon2.path=$$PREFIX/share/icons/hicolor/32x32/apps
	icon2.extra = cp -f ../iconsets/system/default/logo_32.png $(INSTALL_ROOT)$$icon2.path/jabbin.png
	icon3.path=$$PREFIX/share/icons/hicolor/48x48/apps
	icon3.extra = cp -f ../iconsets/system/default/logo_48.png $(INSTALL_ROOT)$$icon3.path/jabbin.png
	icon4.path=$$PREFIX/share/icons/hicolor/64x64/apps
	icon4.extra = cp -f ../iconsets/system/default/logo_64.png $(INSTALL_ROOT)$$icon4.path/jabbin.png
	icon5.path=$$PREFIX/share/icons/hicolor/128x128/apps
	icon5.extra = cp -f ../iconsets/system/default/logo_128.png $(INSTALL_ROOT)$$icon5.path/jabbin.png
	INSTALLS += dt icon1 icon2 icon3 icon4 icon5
}

windows {
	LIBS += -lWSock32 -lUser32 -lShell32 -lGdi32 -lAdvAPI32 -lsecur32 -lWS2_32 -liphlpapi -lwinmm
	DEFINES += QT_STATICPLUGIN
	DEFINES += NOMINMAX # suppress min/max #defines in windows headers
	INCLUDEPATH += . # otherwise MSVC will fail to find "common.h" when compiling options/* stuff
	#QTPLUGIN += qjpeg qgif

	# Expat
        #CONFIG(debug, debug|release)   { LIBS += $$EXPATHDIR\win32\bin\debug\libexpat.lib }
        #CONFIG(release, debug|release) { LIBS += $$EXPATHDIR\win32\bin\release\libexpat.lib }
        LIBS += -L$$EXPATHDIR/Bin -lexpat
	
#	QMAKE_CFLAGS	+= -GR -GX -DWIN32
#	QMAKE_CXXFLAGS	+= -GR -GX -DWIN32
	QMAKE_CFLAGS	+= -DWIN32
	QMAKE_CXXFLAGS	+= -DWIN32
}

unix {
    DEFINES += DEVRAND
    DEFINES += POSIX
}

# Configuration
# IPv6 ?
#DEFINES += NO_NDNS

# Psi sources
include(src.pri)

# don't clash with unittests
SOURCES += main.cpp
HEADERS += main.h

################################################################################
# Translation
################################################################################

LANG_PATH = ../lang

TRANSLATIONS = \
	$$LANG_PATH/psi_ru.ts

OPTIONS_TRANSLATIONS_FILE=$$PWD/option_translations.cpp

QMAKE_EXTRA_TARGETS += translate_options
translate_options.commands = $$PWD/../admin/update_options_ts.py $$PWD/../options/default.xml > $$OPTIONS_TRANSLATIONS_FILE

# In case lupdate doesn't work
QMAKE_EXTRA_TARGETS += translate
translate.commands = lupdate . options widgets tools/grepshortcutkeydlg ../cutestuff/network ../iris/xmpp-im -ts $$TRANSLATIONS


exists($$OPTIONS_TRANSLATIONS_FILE) {
	SOURCES += $$OPTIONS_TRANSLATIONS_FILE
}
QMAKE_CLEAN += $$OPTIONS_TRANSLATIONS_FILE

################################################################################

# Resources
#system(cd ../lang && lrelease psi_ru.ts && lrelease qt_ru.ts && cd ../src)
RESOURCES += ../psi.qrc ../iconsets.qrc

!win32 {
	# Revision number
	system(./jabbinpsi_revision.sh)

	# Qt translations
	system(./jabbinpsi_qt_translations.sh)
}

BREAKPAD_PATH = $$PWD/../../vendor/google-breakpad/src
BREAKPAD_PRI = $$PWD/tools/breakpad/breakpad.pri
include($$BREAKPAD_PRI)

mac {
	CARBONCOCOA_PRI = $$PWD/tools/carboncocoa/carboncocoa.pri
	include($$CARBONCOCOA_PRI)

	SPARKLE_PRI = $$PWD/tools/sparkle/sparkle.pri
	include($$SPARKLE_PRI)
}

# Protection against buffer overruns
unix:debug {
	# QMAKE_CFLAGS   += -fstack-protector-all -Wstack-protector
	# QMAKE_CXXFLAGS += -fstack-protector-all -Wstack-protector
}
win32-msvc.net:debug {
	# QMAKE_CFLAGS   += /GS
	# QMAKE_CXXFLAGS += /GS
}

# Speed up compilation process
win32-msvc.net:debug {
	# /MD (Multithreaded runtime)  http://msdn2.microsoft.com/en-us/library/2kzt1wy3.aspx
	# /Gm (Enable Minimal Rebuild) http://msdn2.microsoft.com/en-us/library/kfz8ad09.aspx
	# /INCREMENTAL                 http://msdn2.microsoft.com/en-us/library/4khtbfyf.aspx
	QMAKE_CFLAGS   += /Gm
	QMAKE_CXXFLAGS += /Gm
	QMAKE_LFLAGS += /INCREMENTAL
}


# Platform specifics
unix:!mac {
	QMAKE_POST_LINK = rm -f ../jabbin ; ln -s src/jabbin ../jabbin
}

win32 {
#	# generate program debug detabase
#	win32-msvc|win32-msvc.net|win32-msvc2005 {
#		QMAKE_CFLAGS += /Zi
#		QMAKE_LFLAGS += /DEBUG
#	}
#
	# buggy MSVC workaround
    # !!! IT IS NOT BUGGY MSVC AND IT IS NOT WORKAROUND !!!
    # this partially solves linker problems when static libraries are linked with mixed
    # Runtime Library mode, mode of RTL must be the same for all
    # So this line must be removed for production release.
	win32-msvc|win32-msvc.net|win32-msvc2005: QMAKE_LFLAGS += /NODEFAULTLIB:libcmt.lib
}

mac {
	# Universal binaries
	qc_universal:contains(QT_CONFIG,x86):contains(QT_CONFIG,ppc) {
		CONFIG += x86 ppc
		QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
		QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
	}

	# Frameworks are specified in src.pri

	QMAKE_INFO_PLIST = ../mac/Info.plist
	RC_FILE = ../mac/application.icns
	QMAKE_POST_LINK = \
		mkdir -p `dirname $(TARGET)`/../Resources/iconsets/emoticons; \
		cp -R tools/yastuff/iconsets/emoticons/* `dirname $(TARGET)`/../Resources/iconsets/emoticons; \
		cp -R ../certs ../sound `dirname $(TARGET)`/../Resources; \
		echo "APPLyach" > `dirname $(TARGET)`/../PkgInfo
}
