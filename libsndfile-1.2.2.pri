win32 {
LIBS += -L$$PWD/libsndfile-1.2.2-win64/lib -lsndfile
INCLUDEPATH += $$PWD/libsndfile-1.2.2-win64/include
DEPENDPATH += $$PWD/libsndfile-1.2.2-win64/bin
}

unix {
 #TODO
}