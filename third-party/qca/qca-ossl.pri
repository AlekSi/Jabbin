SOURCES += $$PWD/qca-ossl/qca-ossl.cpp

windows {
	LIBS += -lgdi32 -lwsock32

	release {
		LIBS += -llibeay32 -lssleay32
	}

	!release {
		LIBS += -llibeay32d -lssleay32d
	}
}
