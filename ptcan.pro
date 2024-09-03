#-------------------------------------------------
#
# Project created by QtCreator 2017-04-24T11:33:21
#
#-------------------------------------------------

QT       += core xml network
QT       -= gui
CONFIG += release

TARGET = ptcan
CONFIG   += console
CONFIG   -= app_bundle

QMAKE_CXXFLAGS += -std=c++0x

TEMPLATE = app

target.path = /opt/work_zc/work
INSTALLS += target

if(contains(TEMPLATE,"lib")){
   DESTDIR = $$OUT_PWD/lib #lib output pwd
}else{
    DESTDIR = $$OUT_PWD/bin #app output pwd
}

MOC_DIR = $$OUT_PWD/moc_tmp
OBJECTS_DIR = $$OUT_PWD/o_tmp

DEFINES += ARM_PLATFORM

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
INCLUDEPATH = .\
        mutex\
        epoll\
        timer\
        zprint\
        canbus\
        canpro\
        tkcommon\
        reflect\
        prodata\
        prodata/sem\
        ptxml\
        msg\
        candata\
        modbus\
        1030app\
        tk200


SOURCES += main.cpp \
    timer/timers.cpp\
    canbus/can_bus.cpp \
    zprint/zprint.cpp \
    canpro/can_protocol.cpp \
    prodata/sem/syssem.cpp \
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
    tkcommon/bsdev.cpp \
    zprint/netprint.cpp

HEADERS += \
    epoll/e_poll.h \
    timer/timers.h \
    canbus/can_bus.h \
    canbus/can_relate.h \
    zprint/version.h \
    zprint/zprint.h \
    canpro/can_protocol.h \
    prodata/pro_data.h \
    prodata/clist.h \
    prodata/sem/syssem.h \
    ptxml/ptxml.h \
    reflect/reflect.h \
    reflect/xmlprocess.h \
    msg/define.h \
    msg/driver.h \
    msg/msg.h \
    msg/MsgMng.h \
    msg/MsgObject.h \
    msg/msgtype.h \
    candata/candata.h \
    candata/candatainfo.h \
    prodata/semshare.h \
    prodata/sharemem.h \
    prodata/ptdataapp.h \
    1030app/1030common.h \
    1030app/1030pro.h \
    1030app/run_mode.h \
    modbus/modbuscrc.h \
    prodata/zmap.h \
    candata/ptrwdatainfo.h \
    tk200/tk200pro.h \
    tk200/tk200cs.h \
    tkcommon/tkcommon.h \
    mutex/mutex.h \
    tkcommon/bsdev.h \
    zprint/netprint.h

