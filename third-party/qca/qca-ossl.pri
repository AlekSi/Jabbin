SOURCES += $$PWD/qca-ossl/qca-ossl.cpp

windows {
	LIBS += -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32
}
