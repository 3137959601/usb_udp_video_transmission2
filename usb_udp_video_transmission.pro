QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

#QMAKE_CXXFLAGS_RELEASE = -O2 -MD -GL
#QMAKE_CXXFLAGS_DEBUG = -Zi -MDd
QMAKE_LFLAGS += "/STACK:655360000,40960000" #设置栈保留大小6553600k，提交大小409600k
CONFIG += resources_big

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    child_window/image_procss_window.cpp \
    child_window/logwindow.cpp \
    drawthread.cpp \
    main.cpp \
    mainwindow.cpp \
    child_window/system_settings_window.cpp \
    child_window/toolwindow.cpp \
    serialworker.cpp \
    usbthread.cpp \
    widget_image.cpp

HEADERS += \
    child_window/image_procss_window.h \
    child_window/logwindow.h \
    cy_cpp/inc/CyAPI.h \
    drawthread.h \
    mainwindow.h \
    child_window/system_settings_window.h \
    child_window/toolwindow.h \
    serialworker.h \
    usbthread.h \
    widget_image.h

FORMS += \
    child_window/image_procss_window.ui \
    child_window/logwindow.ui \
    mainwindow.ui \
    child_window/system_settings_window.ui \
    child_window/toolwindow.ui

CONFIG += lrelease             //用于多语言转换
#CONFIG += embed_translations   //用于多语言转换
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/cy_cpp/lib/x64/ -lCyAPI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/cy_cpp/lib/x64/ -lCyAPI
else:unix: LIBS += -L$$PWD/cy_cpp/lib/x64/ -lCyAPI

INCLUDEPATH += $$PWD/cy_cpp/lib/x64
DEPENDPATH += $$PWD/cy_cpp/lib/x64

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/cy_cpp/lib/x64/libCyAPI.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/cy_cpp/lib/x64/libCyAPI.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/cy_cpp/lib/x64/CyAPI.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/cy_cpp/lib/x64/CyAPI.lib
else:unix: PRE_TARGETDEPS += $$PWD/cy_cpp/lib/x64/libCyAPI.a

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lUser32
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lUser32
else:unix: LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lUser32

INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64'
DEPENDPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64'

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libUser32.a'
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libUser32.a'
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/User32.lib'
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/User32.lib'
else:unix: PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libUser32.a'

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lSetupAPI
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lSetupAPI
else:unix: LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/' -lSetupAPI

INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64'
DEPENDPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64'

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libSetupAPI.a'
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libSetupAPI.a'
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/SetupAPI.lib'
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/SetupAPI.lib'
else:unix: PRE_TARGETDEPS += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22000.0/um/x64/libSetupAPI.a'


#unix|win32: LIBS += -L'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/lib/x64/' -llegacy_stdio_definitions

#INCLUDEPATH += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/lib/x64'
#DEPENDPATH += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/lib/x64'

#win32:!win32-g++: PRE_TARGETDEPS += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/lib/x64/legacy_stdio_definitions.lib'
#else:unix|win32-g++: PRE_TARGETDEPS += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/lib/x64/liblegacy_stdio_definitions.a'

unix|win32: LIBS += -L'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/lib/x64/' -llegacy_stdio_definitions

INCLUDEPATH += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/lib/x64'
DEPENDPATH += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/lib/x64'

win32:!win32-g++: PRE_TARGETDEPS += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/lib/x64/legacy_stdio_definitions.lib'
else:unix|win32-g++: PRE_TARGETDEPS += 'D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/lib/x64/liblegacy_stdio_definitions.a'

win32:CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world470
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world470d
else:unix: LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world470

INCLUDEPATH += D:/opencv/build/include
DEPENDPATH += D:/opencv/build/include

RESOURCES += \
    picture.qrc
