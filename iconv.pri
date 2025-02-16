win32 {
# Stolen from Radioconda
LIBS += -L$$PWD/iconv-win64 -liconv
INCLUDEPATH += $$PWD/iconv-win64
DEPENDPATH += $$PWD/iconv-win64
}

unix {
LIBS += -liconv
}

include($$PWD/charset.pri)
