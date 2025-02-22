win32 {
# Stolen from Radioconda
# ATTENTION: lib/SoapySDR/modules0.8 folder, which consists the device dlls, must be copy in the deployment folder

LIBS += -L$$PWD/SoapySDR-win64 -lSoapySDR
INCLUDEPATH += $$PWD/SoapySDR-win64
DEPENDPATH += $$PWD/SoapySDR-win64
}

unix {
LIBS += -lSoapySDR
}
