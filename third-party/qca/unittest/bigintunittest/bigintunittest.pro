DEPENDPATH += .
include(../unittest.pri)
CONFIG += qtestlib

# test target
QMAKE_EXTRA_TARGETS = test
test.depends = bigintunittest
test.commands = ./bigintunittest

# Input
SOURCES += bigintunittest.cpp
