QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ..

HEADERS += \
    ../structures.h \
    ../hierarchy.h \
    ../validator.h

SOURCES +=  tst_test.cpp \
    ../validator.cpp \
    ../hierarchy.cpp
