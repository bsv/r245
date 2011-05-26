TEMPLATE = app
TARGET = akey
QT += core gui
CONFIG += static release
HEADERS += R245.h \
    keygen.h \
    r245_types.h
SOURCES += R245.cpp \
    keygen.cpp \
    main.cpp
FORMS += keygen.ui
RESOURCES += 
