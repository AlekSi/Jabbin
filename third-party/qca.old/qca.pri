INCLUDEPATH += $$PWD/qca/include/QtCrypto

windows {
    CONFIG(debug, debug|release) {
        LIBS += ../third-party/qca/debug/qca_psi.lib
    }
    CONFIG(release, debug|release) {
        LIBS += ../third-party/qca/release/qca_psi.lib
    }
    LIBS         += -lcrypt32
}
unix:LIBS        += ../third-party/qca/libqca_psi.a
mac:LIBS         += -framework Security
