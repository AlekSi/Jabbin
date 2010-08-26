include(../../conf.pri)

VOIP_CPP      = voip

tins:{
    DEFINES += TINS
    HEADERS += $$VOIP_CPP/tins.h
    SOURCES += $$VOIP_CPP/tins.cpp
}

INCLUDEPATH += $$VOIP_CPP ../third-party/jrtplib

# unix:LIBS += -L../third-party/jrtplib -ljrtp
unix:LIBS   += ../third-party/jrtplib/libjrtp.a
# -L../third-party/iLBC -liLBC
unix:LIBS   += -lspeex
unix:LIBS   += -lssl

#win32:LIBS += -ljthread

win32 {
	DEFINES += WIN32

	INCLUDEPATH  += $$SPEEXDIR/include
	LIBS         += $$SPEEXDIR/libspeex/.libs/libspeex.a

	#CONFIG(debug, debug|release)   { LIBS += ..\third-party\jrtplib\debug\jrtp.lib }
	#CONFIG(release, debug|release) { LIBS += ..\third-party\jrtplib\release\jrtp.lib }
	CONFIG(debug, debug|release)   { LIBS += -L..\third-party\jrtplib\debug\ -ljrtp }
	CONFIG(release, debug|release) { LIBS += -L..\third-party\jrtplib\release\ -ljrtp }

    #LIBS += ..\third-party\iLBC\iLBC.lib

# PortAudio
	INCLUDEPATH += $$PORTAUDIODIR/include

	#CONFIG(debug, debug|release)   { LIBS += $$PORTAUDIODIR\lib\debug\portaudio_x86.lib   }
	#CONFIG(release, debug|release) { LIBS += $$PORTAUDIODIR\lib\release\portaudio_x86.lib }
	LIBS += -L$$PORTAUDIODIR/lib/.libs -lportaudio
	LIBS += -liphlpapi -lwinmm
}
unix {
    LIBS += -lportaudio
}
mac {
    LIBS += -lportaudio
    # If portaudio is used as static
	QMAKE_LFLAGS += -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework CoreFoundation -framework Carbon
}

HEADERS += \
#    $$VOIP_CPP/callslog.h \
#    $$VOIP_CPP/calldlg.h \
    $$VOIP_CPP/dtmfsender.h \
    $$VOIP_CPP/voicecodec.h \
    $$VOIP_CPP/codecs/pcmucodec.h \
    $$VOIP_CPP/codecs/speexcodec.h \
#    $$VOIP_CPP/codecs/ilbccodec.h \
    $$VOIP_CPP/sdp.h \
    $$VOIP_CPP/udp.h \
    $$VOIP_CPP/stun.h \
    $$VOIP_CPP/ringbuffer.h \
    $$VOIP_CPP/mediastream.h \
    $$VOIP_CPP/jabbinmediaengine.h \
#    $$VOIP_CPP/callslogdialogbase.h

SOURCES += \
#    $$VOIP_CPP/callslog.cpp \
#    $$VOIP_CPP/calldlg.cpp \
    $$VOIP_CPP/dtmfsender.cpp \
    $$VOIP_CPP/voicecodec.cpp \
    $$VOIP_CPP/codecs/pcmucodec.cpp \
    $$VOIP_CPP/codecs/speexcodec.cpp \
#    $$VOIP_CPP/codecs/ilbccodec.cpp \
    $$VOIP_CPP/sdp.cpp \
    $$VOIP_CPP/udp.cpp \
    $$VOIP_CPP/stun.cpp \
    $$VOIP_CPP/ringbuffer.cpp \
    $$VOIP_CPP/mediastream.cpp \
    $$VOIP_CPP/jabbinmediaengine.cpp \
#    $$VOIP_CPP/callslogdialogbase.cpp

#INTERFACES += $$VOIP_CPP/callslogdialogbase.ui
