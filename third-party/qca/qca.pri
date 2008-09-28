INCLUDEPATH += $$PWD/qca/include/QtCrypto
LIBS += ../third-party/qca/qca_psi.lib
windows:LIBS += -lcrypt32 
mac:LIBS += -framework Security
