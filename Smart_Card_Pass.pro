#-------------------------------------------------
#
# Project created by QtCreator 2026-07-20T14:33:35
#
#-------------------------------------------------

QT       += core gui sql serialport svg multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NexCard
TEMPLATE = app

# ---------- 人脸识别（dlib）----------
# 第三方 dlib 源码（头文件 + 编译单元）
INCLUDEPATH += thirdparty/dlib
SOURCES += thirdparty/dlib/dlib/all/source.cpp
# 禁用 dlib GUI 支持（本工程用 Qt 提供界面），减少编译依赖
DEFINES += DLIB_NO_GUI_SUPPORT
# Windows 下 dlib networking/定时依赖 Winsock 与 winmm
win32: LIBS += -lws2_32 -lwinmm

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# dlib 19.24 需要 C++14 编译（C++11 下 rvalue 引用成员等特性报错）
CONFIG += c++14

SOURCES += \
        main.cpp \
        widget.cpp \
    loginadminwidget.cpp \
    logincardwidget.cpp \
    mysql.cpp \
    adminpagewidget.cpp \
    adminregisterwidget.cpp \
    serialportwidget.cpp \
    myserial.cpp \
    employeeregisterwidget.cpp \
    employeepagewidget.cpp \
    employeedatawidget.cpp \
    employeeattendancewidget.cpp \
    adminofemployee.cpp \
    adminofsetemp.cpp \
    adminofattendance.cpp \
    adminofaddatt.cpp \
    adminofsetadd.cpp \
    adminofshop.cpp \
    adminofaddshop.cpp \
    adminofsetshop.cpp \
    employeeofshop.cpp \
    employeerefill.cpp \
    adminofadmin.cpp \
    attendanceruledialog.cpp \
    adminfeedbackwidget.cpp \
    employeefeedbackwidget.cpp \
    employeepunchwidget.cpp \
    facedetector.cpp \
    cameraframesurface.cpp \
    facerecognitionworker.cpp \
    faceregisterdialog.cpp

HEADERS += \
        widget.h \
    loginadminwidget.h \
    logincardwidget.h \
    mysql.h \
    adminpagewidget.h \
    adminregisterwidget.h \
    serialportwidget.h \
    myserial.h \
    employeeregisterwidget.h \
    employeepagewidget.h \
    employeedatawidget.h \
    employeeattendancewidget.h \
    adminofemployee.h \
    adminofsetemp.h \
    adminofattendance.h \
    adminofaddatt.h \
    adminofsetadd.h \
    adminofshop.h \
    adminofaddshop.h \
    adminofsetshop.h \
    employeeofshop.h \
    employeerefill.h \
    adminofadmin.h \
    attendanceruledialog.h \
    adminfeedbackwidget.h \
    employeefeedbackwidget.h \
    employeepunchwidget.h \
    facedetector.h \
    cameraframesurface.h \
    facerecognitionworker.h \
    faceregisterdialog.h

FORMS += \
        widget.ui \
    loginadminwidget.ui \
    logincardwidget.ui \
    adminpagewidget.ui \
    adminregisterwidget.ui \
    serialportwidget.ui \
    employeeregisterwidget.ui \
    employeepagewidget.ui \
    employeedatawidget.ui \
    employeeattendancewidget.ui \
    adminofemployee.ui \
    adminofsetemp.ui \
    adminofattendance.ui \
    adminofaddatt.ui \
    adminofsetadd.ui \
    adminofshop.ui \
    adminofaddshop.ui \
    adminofsetshop.ui \
    employeeofshop.ui \
    employeerefill.ui \
    adminofadmin.ui \
    attendanceruledialog.ui \
    adminfeedbackwidget.ui \
    employeefeedbackwidget.ui \
    employeepunchwidget.ui \
    faceregisterdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
