win32 {
# Stolen from Radioconda
LIBS += -L$$PWD/libxml2-win64 -llibxml2
INCLUDEPATH += $$PWD/libxml2-win64
DEPENDPATH += $$PWD/libxml2-win64
}

unix {
LIBS += -llibxml2
}

include($$PWD/iconv.pri)
include($$PWD/zlib.pri)
