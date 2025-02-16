win32 {
# Stolen from Radioconda

LIBS += -L$$PWD/SoapySDR-win64 -lSoapySDR
INCLUDEPATH += $$PWD/SoapySDR-win64
DEPENDPATH += $$PWD/SoapySDR-win64
}

unix {
LIBS += -lSoapySDR
}
