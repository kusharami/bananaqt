BIN_DIR = $$OUT_PWD/../bin
DESTDIR = $$BIN_DIR

CONFIG += c++11 warn_off

unix|win32-g++ {
    QMAKE_CXXFLAGS_WARN_OFF -= -w
    QMAKE_CXXFLAGS += -Wall
} else {
    win32 {
        QMAKE_CXXFLAGS_WARN_OFF -= -W0
        QMAKE_CXXFLAGS += -W3 /wd4244 /wd4573
        DEFINES += _CRT_SECURE_NO_WARNINGS

        DEFINES += "or=\"||\""
        DEFINES += "and=\"&&\""
        DEFINES += "not=\"!\""
    }
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
} else {
    DEFINES += NDEBUG
}

LIBS += -L$$BIN_DIR
INCLUDEPATH += $$PWD/QtnProperty

for(tr, TRANSLATIONS):system($$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_PWD_/$${tr})
