EXPATHDIR = c:/expat-2.0.1
OPENSSL_PREFIX = c:/openssl

exists(../../conf.pri) {
	include(../../conf.pri)
}

TEMPLATE = lib
CONFIG += staticlib
TARGET = jingle

#win32:CONFIG += debug
#unix:CONFIG += debug
CONFIG += warn_off

unix:debug: QMAKE_CXXFLAGS += -O0 -g3

#target.extra = true

JINGLE_CPP = .
INCLUDEPATH += $$JINGLE_CPP ../../src/voip
unix:DEFINES += POSIX

win32 {
  DEFINES += WIN32
  DEFINES += _WINDOWS
  DEFINES += _UNICODE
  DEFINES += WINVER=0x0500
  DEFINES += WIN32_WINNT=0x500
  DEFINES += _WIN32_IE=0x0500
  DEFINES += _SCL_SECURE_NO_DEPRECATE
  DEFINES += _CRT_SECURE_NO_DEPRECATE

  #DEFINES += COMPILER_MSVC
#  QMAKE_CFLAGS += -EHsc
#  QMAKE_CXXFLAGS += -EHsc
  INCLUDEPATH += $$EXPATHDIR/Source/lib \
        $$OPENSSL_PREFIX/openssl/include
  LIBS += -L$$EXPATHDIR/Bin -lexpat -loleaut32 -lole32 -lsecur32
  SOURCES += $$JINGLE_CPP/talk/base/winfirewall.cc \
	$$JINGLE_CPP/talk/base/winping.cc \
}

OBJECTS_DIR = $$JINGLE_CPP/.obj

# Base
SOURCES += \
	$$JINGLE_CPP/talk/base/asyncpacketsocket.cc \
	$$JINGLE_CPP/talk/base/asynctcpsocket.cc \
	$$JINGLE_CPP/talk/base/asyncudpsocket.cc \
	$$JINGLE_CPP/talk/base/base64.cc \
	$$JINGLE_CPP/talk/base/bytebuffer.cc \
	$$JINGLE_CPP/talk/base/md5c.c \
	$$JINGLE_CPP/talk/base/messagequeue.cc \
	$$JINGLE_CPP/talk/base/network.cc \
	$$JINGLE_CPP/talk/base/physicalsocketserver.cc \
	$$JINGLE_CPP/talk/base/socketadapters.cc \
	$$JINGLE_CPP/talk/base/socketaddress.cc \
	$$JINGLE_CPP/talk/base/task.cc \
	$$JINGLE_CPP/talk/base/taskrunner.cc \
	$$JINGLE_CPP/talk/base/thread.cc \
	$$JINGLE_CPP/talk/base/logging.cc \
	$$JINGLE_CPP/talk/base/stream.cc \
	$$JINGLE_CPP/talk/base/helpers.cc \
	$$JINGLE_CPP/talk/base/httpcommon.cc \
	$$JINGLE_CPP/talk/base/stringdigest.cc \
	$$JINGLE_CPP/talk/base/stringencode.cc \
	$$JINGLE_CPP/talk/base/asynchttprequest.cc \
	$$JINGLE_CPP/talk/base/signalthread.cc \
	$$JINGLE_CPP/talk/base/ssladapter.cc \
	$$JINGLE_CPP/talk/base/socketpool.cc \
	$$JINGLE_CPP/talk/base/httpbase.cc \
	$$JINGLE_CPP/talk/base/httpclient.cc \
	$$JINGLE_CPP/talk/base/diskcache.cc \
	$$JINGLE_CPP/talk/base/firewallsocketserver.cc \
	$$JINGLE_CPP/talk/base/pathutils.cc \
	$$JINGLE_CPP/talk/base/fileutils.cc \
	$$JINGLE_CPP/talk/base/tarstream.cc \
	$$JINGLE_CPP/talk/base/urlencode.cc \
	$$JINGLE_CPP/talk/base/time.cc

unix {
	SOURCES += $$JINGLE_CPP/talk/base/unixfilesystem.cc \
				$$JINGLE_CPP/talk/base/openssladapter.cc
}
win32:SOURCES += $$JINGLE_CPP/talk/base/win32filesystem.cc
win32:SOURCES += $$JINGLE_CPP/talk/base/schanneladapter.cc


# Not needed ?
#$$JINGLE_CPP/talk/base/socketaddresspair.cc \
#$$JINGLE_CPP/talk/base/host.cc \

# P2P Base
SOURCES += \
	$$JINGLE_CPP/talk/p2p/base/port.cc \
	$$JINGLE_CPP/talk/p2p/base/relayport.cc \
	$$JINGLE_CPP/talk/p2p/base/base_constants.cc \
	$$JINGLE_CPP/talk/p2p/base/session.cc \
	$$JINGLE_CPP/talk/p2p/base/sessionmanager.cc \
	$$JINGLE_CPP/talk/p2p/base/transport.cc \
	$$JINGLE_CPP/talk/p2p/base/transportchannel.cc \
	$$JINGLE_CPP/talk/p2p/base/transportchannelproxy.cc \
	$$JINGLE_CPP/talk/p2p/base/p2ptransportchannel.cc \
	$$JINGLE_CPP/talk/p2p/base/stun.cc \
	$$JINGLE_CPP/talk/p2p/base/stunport.cc \
	$$JINGLE_CPP/talk/p2p/base/stunrequest.cc \
	$$JINGLE_CPP/talk/p2p/base/tcpport.cc \
	$$JINGLE_CPP/talk/p2p/base/udpport.cc \
	$$JINGLE_CPP/talk/p2p/base/p2ptransport.cc
	
# P2P Client
SOURCES += \
	$$JINGLE_CPP/talk/p2p/client/basicportallocator.cc \
	$$JINGLE_CPP/talk/p2p/client/httpportallocator.cc \
	$$JINGLE_CPP/talk/p2p/client/socketmonitor.cc


# XMLLite
SOURCES += \
	$$JINGLE_CPP/talk/xmllite/qname.cc \
	$$JINGLE_CPP/talk/xmllite/xmlbuilder.cc \
	$$JINGLE_CPP/talk/xmllite/xmlconstants.cc \
	$$JINGLE_CPP/talk/xmllite/xmlelement.cc \
	$$JINGLE_CPP/talk/xmllite/xmlnsstack.cc \
	$$JINGLE_CPP/talk/xmllite/xmlparser.cc \
	$$JINGLE_CPP/talk/xmllite/xmlprinter.cc

# XMPP
SOURCES += \
	$$JINGLE_CPP/talk/xmpp/constants.cc \
	$$JINGLE_CPP/talk/xmpp/jid.cc \
	$$JINGLE_CPP/talk/xmpp/saslmechanism.cc \
	$$JINGLE_CPP/talk/xmpp/xmppclient.cc \
	$$JINGLE_CPP/talk/xmpp/xmppengineimpl.cc \
	$$JINGLE_CPP/talk/xmpp/xmppengineimpl_iq.cc \
	$$JINGLE_CPP/talk/xmpp/xmpplogintask.cc \
	$$JINGLE_CPP/talk/xmpp/xmppstanzaparser.cc \
	$$JINGLE_CPP/talk/xmpp/xmpptask.cc

# Session
SOURCES += \
		$$JINGLE_CPP/talk/session/phone/call.cc \
		$$JINGLE_CPP/talk/session/phone/audiomonitor.cc \
		$$JINGLE_CPP/talk/session/phone/phonesessionclient.cc \
		$$JINGLE_CPP/talk/session/phone/channelmanager.cc \
		$$JINGLE_CPP/talk/session/phone/voicechannel.cc

        #$$JINGLE_CPP/talk/session/phone/linphonemediaengine.cc \
	
#contains(DEFINES, HAVE_PORTAUDIO) {
#	SOURCES += \
#		$$JINGLE_CPP/talk/session/phone/portaudiomediaengine.cc
#}
