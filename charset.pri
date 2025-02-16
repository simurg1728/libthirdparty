win32 {
# Stolen from Radioconda
LIBS += -L$$PWD/charset-win64 -lcharset
INCLUDEPATH += $$PWD/charset-win64
DEPENDPATH += $$PWD/charset-win64
}

unix {
LIBS += -lcharset
}
