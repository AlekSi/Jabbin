SOURCES += $$PWD/qca-ossl/qca-ossl.cpp

windows {
	LIBS += -lgdi32 -lwsock32

	debug {
		LIBS += -llibeay32MDd -lssleay32MDd
	}

	!debug {
		LIBS += -llibeay32MD -lssleay32MD
	}
}
