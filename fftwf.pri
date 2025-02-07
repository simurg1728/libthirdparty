win32 {
LIBS += -L$$PWD/fftw -llibfftw3f-3
INCLUDEPATH += $$PWD/fftw
DEPENDPATH += $$PWD/fftw
}

unix {
LIBS += -lfftw3
}