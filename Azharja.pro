QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    data.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    data.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../scite511/scintilla/bin/release/ -lScintillaEdit5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../scite511/scintilla/bin/debug/ -lScintillaEdit5

INCLUDEPATH += $$PWD/../scite511/scintilla/include
INCLUDEPATH += $$PWD/../scite511/scintilla/src
INCLUDEPATH += $$PWD/../scite511/scintilla/qt/ScintillaEditBase
INCLUDEPATH += $$PWD/../scite511/scintilla/qt/ScintillaEdit

win32:CONFIG(release, debug|release): DEPENDPATH += $$PWD/../scite511/scintilla/bin/release/
else:win32:CONFIG(debug, debug|release): DEPENDPATH += $$PWD/../scite511/scintilla/bin/debug/