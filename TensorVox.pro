QT += core gui
QT += multimedia
QT += winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
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
    attention.cpp \
    batchdenoisedlg.cpp \
    ext/ByteArr.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/DarkStyle.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/framelesswindow.cpp \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/windowdragger.cpp \
    ext/ZCharScanner.cpp \
    ext/ZFile.cpp \
    ext/qcustomplot.cpp \
    main.cpp \
    mainwindow.cpp \
    melgen.cpp \
    modelinfodlg.cpp \
    phddialog.cpp \
    phonemizer.cpp \
    phoneticdict.cpp \
    phonetichighlighter.cpp \
    spectrogram.cpp \
    tacotron2.cpp \
    tfg2p.cpp \
    torchmoji.cpp \
    track.cpp \
    vits.cpp \
    voicemanager.cpp \
    voxer.cpp

HEADERS += \
    EnglishPhoneticProcessor.h \
    FastSpeech2.h \
    MultiBandMelGAN.h \
    TextTokenizer.h \
    Voice.h \
    VoxCommon.hpp \
    attention.h \
    batchdenoisedlg.h \
    ext/AudioFile.hpp \
    ext/ByteArr.h \
    ext/CppFlow/context.h \
    ext/CppFlow/cppflow.h \
    ext/CppFlow/datatype.h \
    ext/CppFlow/defer.h \
    ext/CppFlow/model.h \
    ext/CppFlow/ops.h \
    ext/CppFlow/raw_ops.h \
    ext/CppFlow/tensor.h \
    ext/Qt-Frameless-Window-DarkStyle-master/DarkStyle.h \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/framelesswindow.h \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow/windowdragger.h \
    ext/ZCharScanner.h \
    ext/ZFile.h \
    ext/json.hpp \
    ext/qcustomplot.h \
    mainwindow.h \
    melgen.h \
    modelinfodlg.h \
    phddialog.h \
    phonemizer.h \
    phoneticdict.h \
    phonetichighlighter.h \
    spectrogram.h \
    tacotron2.h \
    tfg2p.h \
    torchmoji.h \
    track.h \
    vits.h \
    voicemanager.h \
    voxer.h

FORMS += \
    batchdenoisedlg.ui \
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
INCLUDEPATH += $$PWD/deps/include/libtorch
INCLUDEPATH += $$PWD/ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow
win32: LIBS += -L$$PWD/deps/lib/ tensorflow.lib r8bsrc64.lib rnnoise64.lib LogitechLEDLib.lib LibNumberText64.lib c10.lib torch.lib torch_cpu.lib
win32: LIBS += Advapi32.lib User32.lib Psapi.lib


RESOURCES += \
    ext/Qt-Frameless-Window-DarkStyle-master/darkstyle.qrc \
    ext/Qt-Frameless-Window-DarkStyle-master/framelesswindow.qrc \
    stdres.qrc

win32:RC_ICONS += winicon.ico

VERSION = 1.0.0.0
CONFIG += force_debug_info

QMAKE_CXXFLAGS += /std:c++17 /utf-8 -DPSAPI_VERSION=1

DISTFILES += \
    res/defaultim.png
