INCLUDEPATH += $$PWD/qca/include/QtCrypto
LIBS += -L$$PWD -lqca
windows:LIBS += -lcrypt32 
mac:LIBS += -framework Security
