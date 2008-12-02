INCLUDEPATH += $$PWD/qca/include/QtCrypto
win32 {
    LIBS += ../third-party/qca/release/qca_psi.lib
}
unix {
    LIBS += ../third-party/qca/libqca_psi.a
}
windows:LIBS += -lcrypt32
mac:LIBS += -framework Security
