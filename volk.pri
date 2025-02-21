win32 {
# https://packages.msys2.org/packages/mingw-w64-x86_64-volk, 3.1.2-5
LIBS += -L$$PWD/volk-3.1.2-win64 -llibvolk
INCLUDEPATH += $$PWD/volk-3.1.2-win64
DEPENDPATH += $$PWD/volk-3.1.2-win64
}

unix {
LIBS += -lvolk
}

DEFINES += HAVE_VOLK
