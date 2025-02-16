win32 {
# Stolen from Radioconda
LIBS += -L$$PWD/zlib-win64 -lzlib
INCLUDEPATH += $$PWD/zlib-win64
DEPENDPATH += $$PWD/zlib-win64
}

unix {
LIBS += -lzlib
}
