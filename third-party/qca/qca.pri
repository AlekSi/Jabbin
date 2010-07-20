INCLUDEPATH += $$PWD/qca/include/QtCrypto
LIBS += -L$$PWD -lqca2
windows:LIBS += -lcrypt32 
mac:LIBS += -framework Security
