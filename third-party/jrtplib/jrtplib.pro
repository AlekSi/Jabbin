TEMPLATE = lib
CONFIG += staticlib
TARGET = jrtp
INCLUDEPATH += . ../libjingle-0.4.0

win32:DEFINES += WIN32
win32:QMAKE_CFLAGS	+= -GR -GX -DWIN32
win32:QMAKE_CXXFLAGS	+= -GR -GX -DWIN32

unix:{
        MOC_DIR        = .moc
        OBJECTS_DIR    = .obj
        UI_DIR         = .ui
}

win32:{
        MOC_DIR        = .obj
        OBJECTS_DIR    = .obj
        UI_DIR         = .obj
}

# Input
HEADERS += rtcpapppacket.h \
           rtcpbyepacket.h \
           rtcpcompoundpacket.h \
           rtcpcompoundpacketbuilder.h \
           rtcppacket.h \
           rtcppacketbuilder.h \
           rtcprrpacket.h \
           rtcpscheduler.h \
           rtcpsdesinfo.h \
           rtcpsdespacket.h \
           rtcpsrpacket.h \
           rtcpunknownpacket.h \
           rtpaddress.h \
           rtpcollisionlist.h \
           rtpconfig.h \
           rtpdebug.h \
           rtpdefines.h \
           rtperrors.h \
           rtphashtable.h \
           rtpinternalsourcedata.h \
           rtpipv4address.h \
           rtpipv4destination.h \
           rtpipv6address.h \
           rtpipv6destination.h \
           rtpkeyhashtable.h \
           rtplibraryversion.h \
           rtppacket.h \
           rtppacketbuilder.h \
           rtppollthread.h \
           rtprandom.h \
           rtprawpacket.h \
           rtpsession.h \
           rtpsessionparams.h \
           rtpsessionsources.h \
           rtpsourcedata.h \
           rtpsources.h \
           rtpstructs.h \
           rtptimeutilities.h \
           rtptransmitter.h \
           rtptypes.h \
           rtptypes_unix.h \
           rtptypes_win.h \
           rtpudpv4transmitter.h \
           rtpudpv6transmitter.h \
           jabbintransmitter.h

unix:HEADERS += rtpconfig_unix.h
win32:HEADERS += rtpconfig_win.h
           
SOURCES += rtcpapppacket.cpp \
           rtcpbyepacket.cpp \
           rtcpcompoundpacket.cpp \
           rtcpcompoundpacketbuilder.cpp \
           rtcppacket.cpp \
           rtcppacketbuilder.cpp \
           rtcprrpacket.cpp \
           rtcpscheduler.cpp \
           rtcpsdesinfo.cpp \
           rtcpsdespacket.cpp \
           rtcpsrpacket.cpp \
           rtpcollisionlist.cpp \
           rtpdebug.cpp \
           rtperrors.cpp \
           rtpinternalsourcedata.cpp \
           rtpipv4address.cpp \
           rtpipv6address.cpp \
           rtplibraryversion.cpp \
           rtppacket.cpp \
           rtppacketbuilder.cpp \
           rtppollthread.cpp \
           rtprandom.cpp \
           rtpsession.cpp \
           rtpsessionparams.cpp \
           rtpsessionsources.cpp \
           rtpsourcedata.cpp \
           rtpsources.cpp \
           rtpudpv4transmitter.cpp \
           rtpudpv6transmitter.cpp \
           jabbintransmitter.cpp 
