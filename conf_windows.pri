# Windows build settings
CONFIG += release
#CONFIG  += debug
CONFIG  += jingle
CONFIG += qca-static

# tell iris to use our internal libz
CONFIG += psi-zip

# Console window
CONFIG(debug, debug|release) { CONFIG += console }

EXPATHDIR = C:\expat-2.0.1
PORTAUDIODIR = C:\portaudio
SPEEXDIR = C:\speex-1.2rc1

# OpenSSL
qca-static {
	DEFINES += HAVE_OPENSSL
	DEFINES += OSSL_097
	DEFINES += QCA_NO_PLUGINS
	OPENSSL_PREFIX = $$OPENSSL_PREFIX/openssl
	INCLUDEPATH += $$OPENSSL_PREFIX/include
	LIBS += -L$$OPENSSL_PREFIX/lib
}

# SASL
#qca-static {
#	CYRUSSASL_PREFIX = /local
#	INCLUDEPATH += $$CYRUSSASL_PREFIX/include
#	LIBS += $$CYRUSSASL_PREFIX/lib/libsasl.lib
#}

# ASpell
#DEFINES += HAVE_ASPELL
contains(DEFINES, HAVE_ASPELL) {
	ASPELL_PREFIX = ../../../aspell
	INCLUDEPATH += "$$ASPELL_PREFIX/include"
	LIBS += -L"$$ASPELL_PREFIX/lib"
	LIBS += -laspell-15
}
# qconf

PREFIX = /usr
BINDIR = /usr/bin
DATADIR = /usr/share

DEFINES += HAVE_XSS HAVE_DNOTIFY HAVE_GETHOSTBYNAME_R HAVE_CONFIG
#LIBS += -lXss
#LIBS += -lz -lXss -laspell
#CONFIG += dbus
#CONFIG += release
#PSI_DATADIR=/usr/share/yachat

