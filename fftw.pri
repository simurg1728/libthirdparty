win32 {
LIBS += -L$$PWD/fftw-win64 -llibfftw3-3
INCLUDEPATH += $$PWD/fftw-win64
DEPENDPATH += $$PWD/fftw-win64
}

unix {
LIBS += -lfftw3
}