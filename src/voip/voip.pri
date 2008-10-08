VOIP_CPP = voip
SPEEXPATH = C:\speex-1.2rc1

tins:{
    DEFINES += TINS
    HEADERS += $$VOIP_CPP/tins.h
    SOURCES += $$VOIP_CPP/tins.cpp
}

INCLUDEPATH += $$VOIP_CPP ../third-party/jrtplib $$VOIP_CPP/portaudio

# unix:LIBS += -L../third-party/jrtplib -ljrtp
unix:LIBS += ../third-party/jrtplib/libjrtp.a
# -L../third-party/iLBC -liLBC
unix:LIBS += -lspeex
unix:LIBS += -lssl

#win32:LIBS += -ljthread

win32 {
  INCLUDEPATH += $$SPEEXPATH\include
  LIBS += $$SPEEXPATH\lib\libspeex.lib
  LIBS += ..\third-party\jrtplib\release\jrtp.lib
#  LIBS += ..\third-party\iLBC\iLBC.lib
}

HEADERS += \
    $$VOIP_CPP/callslog.h \
    $$VOIP_CPP/calldlg.h \
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
    $$VOIP_CPP/joimmediaengine.h \
    $$VOIP_CPP/callslogdialogbase.h

SOURCES += \
    $$VOIP_CPP/callslog.cpp \
    $$VOIP_CPP/calldlg.cpp \
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
    $$VOIP_CPP/joimmediaengine.cpp \
    $$VOIP_CPP/callslogdialogbase.cpp

#portaudio

HEADERS += \
    $$VOIP_CPP/portaudio/pa_host.h \
    $$VOIP_CPP/portaudio/pa_trace.h \
    $$VOIP_CPP/portaudio/portaudio.h

unix: HEADERS += $$VOIP_CPP/portaudio/pa_unix.h

SOURCES += \
    $$VOIP_CPP/portaudio/pa_convert.c \
    $$VOIP_CPP/portaudio/pa_lib.c \
    $$VOIP_CPP/portaudio/pa_trace.c

unix: SOURCES += $$VOIP_CPP/portaudio/pa_unix.c $$VOIP_CPP/portaudio/pa_unix_oss.c
win32:SOURCES += $$VOIP_CPP/portaudio/pa_win_wmme.c

INTERFACES += $$VOIP_CPP/callslogdialogbase.ui
