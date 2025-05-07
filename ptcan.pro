#-------------------------------------------------
#
# Project created by QtCreator 2017-04-24T11:33:21
#
#-------------------------------------------------

QT       += core xml network
QT       -= gui

TARGET = ptcan
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

#target.path = /opt/work_zc/work
target.path = /opt/bin
INSTALLS += target

if(contains(TEMPLATE,"lib")){
   DESTDIR = $$OUT_PWD/lib #lib output pwd
}else{
    DESTDIR = $$OUT_PWD/bin #app output pwd
}

MOC_DIR = $$OUT_PWD/moc_tmp
OBJECTS_DIR = $$OUT_PWD/o_tmp

linux-arm-gnueabi-g++{
    CONFIG(debug, debug|release){
        QBUILD = $$PWD/../bin/debug/arm/exe
    }else{
        QBUILD = $$PWD/../bin/release/arm/exe
    }
    QLIB = $$PWD/Libs/arm/lib
    QINCLUDE = $$PWD/Libs/arm/include
    message("arm")
    DEFINES += ARM
    DEFINES += ARM_PLATFORM
    DEFINES += SUPPORT_DBUS

}
linux-gnueabi-oe-g++ {
    CONFIG(debug, debug|release){
        message("debug")
        QBUILD = $$PWD/../bin/debug/arm/exe
    }else{
        message("release")
        QBUILD = $$PWD/../bin/release/arm/exe
    }
    QLIB = $$PWD/Libs/arm/lib
    QINCLUDE = $$PWD/Libs/arm/include
    message("arm")
    DEFINES += ARM
    DEFINES += ARM_PLATFORM
    DEFINES += SUPPORT_DBUS

}

COMMLIB_PATH = $$PWD/../public/zcommonlib
LIBS +=  -L$$QBUILD -lzcommonlib

# DEFINES += ARM_PLATFORM

#linux-arm-gnueabi-g++{
#           DEFINES += ARM_PLATFORM
#}
linux-g++-64{
           DEFINES += PC_PLATFORM
}


contains(QT_ARCH, aarch64){
message("arm64")
linux-g++{
           DEFINES += AARCH64_PLATFORM
}
}else{
message("x86")
linux-g++{
           DEFINES += PC_PLATFORM
}
}

DEFINES += NO_LOCK_ERROR
#DEFINES += END_NO_RESET

INCLUDEPATH += $${COMMLIB_PATH}/.\
            $${COMMLIB_PATH}/bufmodel\
            $${COMMLIB_PATH}/date\
            $${COMMLIB_PATH}/epoll\
            $${COMMLIB_PATH}/mutex\
            $${COMMLIB_PATH}/prodata\
            $${COMMLIB_PATH}/prodata/sem\
            $${COMMLIB_PATH}/reflect\
            $${COMMLIB_PATH}/sempro\
            $${COMMLIB_PATH}/sigslot\
            $${COMMLIB_PATH}/socket\
            $${COMMLIB_PATH}/tcp\
            $${COMMLIB_PATH}/timer\
            $${COMMLIB_PATH}/udp\
            $${COMMLIB_PATH}/file\
            $${COMMLIB_PATH}/zprint



INCLUDEPATH += .\
        canbus\
        canpro\
        tkcommon\
        ptxml\
        msg\
        candata\
        modbus\
        1030app\
        tk200




SOURCES += main.cpp \
    canbus/can_bus.cpp \
    canpro/can_protocol.cpp \
    msg/driver.cpp \
    msg/msg.cpp \
    msg/MsgMng.cpp \
    msg/MsgObject.cpp \
    candata/candata.cpp \
    1030app/1030pro.cpp \
    1030app/1030common.cpp \
    modbus/modbuscrc.cpp \
    candata/ptrwdatainfo.cpp \
    tk200/tk200pro.cpp \
    tk200/tk200cs.cpp \
    tkcommon/tkcommon.cpp \
    tkcommon/bsdev.cpp


HEADERS += \
    canbus/can_bus.h \
    canbus/can_relate.h \
    canpro/can_protocol.h \
    ptxml/ptxml.h \
    msg/define.h \
    msg/driver.h \
    msg/msg.h \
    msg/MsgMng.h \
    msg/MsgObject.h \
    msg/msgtype.h \
    candata/candata.h \
    candata/candatainfo.h \
    1030app/1030common.h \
    1030app/1030pro.h \
    1030app/run_mode.h \
    modbus/modbuscrc.h \
    candata/ptrwdatainfo.h \
    tk200/tk200pro.h \
    tk200/tk200cs.h \
    tkcommon/tkcommon.h \
    tkcommon/bsdev.h

