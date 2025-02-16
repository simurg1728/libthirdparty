win32 {
# Stolen from Radioconda
LIBS += -L$$PWD/volk-3.1.2-win64 -lvolk
INCLUDEPATH += $$PWD/volk-3.1.2-win64
DEPENDPATH += $$PWD/volk-3.1.2-win64
}

unix {
LIBS += -lvolk
}

DEFINES += HAVE_VOLK
