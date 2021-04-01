QT += core gui
QT += multimedia
QT += winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    EnglishPhoneticProcessor.cpp \
    FastSpeech2.cpp \
    MultiBandMelGAN.cpp \
    TextTokenizer.cpp \
    Voice.cpp \
    VoxCommon.cpp \
    ext/ByteArr.cpp \
    ext/CppFlow/src/Model.cpp \
    ext/CppFlow/src/Tensor.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/DarkStyle.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/framelesswindow.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/windowdragger.cpp \
    ext/ZCharScanner.cpp \
    ext/ZFile.cpp \
    main.cpp \
    mainwindow.cpp \
    melgen.cpp \
    modelinfodlg.cpp \
    phddialog.cpp \
    phonemizer.cpp \
    phoneticdict.cpp \
    phonetichighlighter.cpp \
    tacotron2.cpp \
    tfg2p.cpp \
    voicemanager.cpp \
    voxer.cpp

HEADERS += \
    EnglishPhoneticProcessor.h \
    FastSpeech2.h \
    MultiBandMelGAN.h \
    TextTokenizer.h \
    Voice.h \
    VoxCommon.hpp \
    ext/AudioFile.hpp \
    ext/ByteArr.h \
    ext/CppFlow/include/Model.h \
    ext/CppFlow/include/Tensor.h \
    ext/Qt-Frameless-Window-DarkStyle-master/DarkStyle.h \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/framelesswindow.h \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/windowdragger.h \
    ext/ZCharScanner.h \
    ext/ZFile.h \
    ext/json.hpp \
    mainwindow.h \
    melgen.h \
    modelinfodlg.h \
    phddialog.h \
    phonemizer.h \
    phoneticdict.h \
    phonetichighlighter.h \
    tacotron2.h \
    tfg2p.h \
    voicemanager.h \
    voxer.h

FORMS += \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/framelesswindow.ui \
    mainwindow.ui \
    modelinfodlg.ui \
    phddialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


DEFINES += _CRT_SECURE_NO_WARNINGS

INCLUDEPATH += $$PWD/deps/include
INCLUDEPATH += $$PWD/ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow
win32: LIBS += -L$$PWD/deps/lib/ tensorflow.lib r8bsrc64.lib rnnoise64.lib LogitechLEDLib.lib
win32: LIBS += Advapi32.lib User32.lib


RESOURCES += \
    ext/Qt-Frameless-Window-DarkStyle-master/darkstyle.qrc \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow.qrc \
    stdres.qrc

win32:RC_ICONS += winicon.ico

VERSION = 0.8.2.0
CONFIG += force_debug_info
