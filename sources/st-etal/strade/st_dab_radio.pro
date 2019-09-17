# Include extra-commands from external makefile
Standalone {
include(./targets/standalone/target_config.mk)
HEADERS += ./targets/standalone/target_config.h
INCLUDEPATH += ./targets/standalone
DISTFILES +=  ./standalone/target_config.mk
}

Etal {
include(./targets/etal/target_config.mk)
HEADERS += ./targets/etal/target_config.h
INCLUDEPATH += ./targets/etal
DISTFILES +=  ./etal/target_config.mk
}

Jasmin {
include(./targets/jasmin/target_config.mk)
HEADERS += ./targets/jasmin/target_config.h
INCLUDEPATH += ./targets/jasmin
DISTFILES +=  ./jasmin/target_config.mk
}

Release:DESTDIR = release
Release:OBJECTS_DIR = release/obj
Release:MOC_DIR = release/moc
Release:RCC_DIR = release/rcc
Release:UI_DIR = release/ui
Release:DEFINES += RELEASE_TARGET

Release:CONFIG += use_libs

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/obj
Debug:MOC_DIR = debug/moc
Debug:RCC_DIR = debug/rcc
Debug:UI_DIR = debug/ui
Debug:DEFINES += DEBUG_TARGET
Debug:CONFIG += use_debug_libs
#Debug:CONFIG += console
Debug:CONFIG -= console

#ifdef QT_DEBUG
#  // do something (debug mode)
#endif

# Dist files
DISTFILES += \
    tools_config/uncrustify.cfg

# Configuration flags
CONFIG += c++11

# C++ flags
QMAKE_CXXFLAGS += -Wno-missing-field-initializers

# Include paths
INCLUDEPATH += .

# QT modules
QT += core
QT += network
QT += multimedia

contains(QT_CONFIG, opengl): QT += opengl

# Source files
SOURCES += main.cpp \
    tcptransportlayer.cpp \
    utilities.cpp \
    launchwindow.cpp \
    filemanager.cpp \
    dabradio.cpp \
    station_list_dab.cpp \
    station_list_fm.cpp \
    station_list_global.cpp \
    worker.cpp \
    observer.cpp \
    storage.cpp \
    shellwindow.cpp \
    radiomanagerbase.cpp \
    protocollayer.cpp \
    event_manager.cpp \
    postaloffice.cpp \
    state_machine.cpp \
    state_machine_radiomanager.cpp \
    rds.cpp \
    stationlistmanager.cpp \
    presets.cpp \
    sm.cpp

CONFIG_USE_STANDALONE {
SOURCES += cmd_mngr_mw.cpp \
           cmd_mngr_cis.cpp \
           radiomanagermw.cpp \
           radiomanager.cpp \
           globallist_mngr_mw.cpp
}

CONFIG_USE_ETAL {
SOURCES += cmd_mngr_etal.cpp \
           radiomanageretal.cpp \
           radiomanager.cpp \
           globallist_mngr_etal.cpp
}

CONFIG_USE_JASMIN {
SOURCES += radiomanager_jasmin.cpp
}

# Header files
HEADERS += common.h \
    tcptransportlayer.h \
    utilities.h \
    launchwindow.h \
    filemanager.h \
    dabradio.h \
    station_list.h \
    station_list_dab.h \
    station_list_fm.h \
    station_list_global.h \
    stm_types.h \
    worker.h \
    observer.h \
    storage.h \
    radio_storage_types.h \
    shellwindow.h \
    radiomanagerbase.h \
    protocollayer.h \
    event_manager.h \
    defines.h \
    postaloffice.h \
    postal_types.h \
    state_machine.h \
    cmd_mngr_base.h \
    state_machine_radiomanager.h \
    rds.h \
    stationlistmanager.h \
    presets.h \
    globallist_mngr_base.h \
    sm.h \
    elistwidget.h \
    pty.h \
    echeckbox.h \
    dab.h

CONFIG_USE_STANDALONE {
HEADERS += cmd_mngr_mw.h \
           cmd_mngr_cis.h \
           radiomanager.h \
           radiomanagermw.h \
           globallist_mngr_mw.h
}

CONFIG_USE_ETAL {
HEADERS += cmd_mngr_etal.h \
    globallist_mngr_etal.h \
    radiomanager.h \
    radiomanageretal.h \
    etal/etal_api.h \
    etal/etal_types.h \
    etal/etaltml_api.h \
    etal/etaltml_types.h \
    etal/etalversion.h
}

CONFIG_USE_JASMIN {
    HEADERS += radiomanager_jasmin.h \
    etal/etal_api.h \
    etal/etal_types.h \
    etal/etaltml_api.h \
    etal/etaltml_types.h \
    etal/etalversion.h
}

# Forms
FORMS += \
    launchwindow.ui \
    dabradio.ui \
    shellwindow.ui

# Internal resources
RESOURCES += \
    resources.qrc

OTHER_FILES += \
    settings/protocol_layer_config.cfg

#
# Configure Etal target
#
CONFIG_USE_ETAL {

CONFIG_BOARD_SIGLE_TUNER {
CONFIG( debug, debug|release ) {
    # debug
    win32: LIBS += -L$$PWD/etal/lib_std/ -letald
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/etald.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/libetald.a
    INCLUDEPATH += $$PWD/etal/lib_std
    DEPENDPATH += $$PWD/etal/lib_std
} else {
    # release
    win32: LIBS += -L$$PWD/etal/lib_std/ -letal
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/etal.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/libetal.a
    INCLUDEPATH += $$PWD/etal/lib_std
    DEPENDPATH += $$PWD/etal/lib_std
}
}

CONFIG_BOARD_DUAL_TUNER {
CONFIG( debug, debug|release ) {
    # debug
    win32: LIBS += -L$$PWD/etal/lib_mtd/ -letald
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/etald.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/libetald.a
    INCLUDEPATH += $$PWD/etal/lib_mtd
    DEPENDPATH += $$PWD/etal/lib_mtd
} else {
    # release
    win32: LIBS += -L$$PWD/etal/lib_mtd/ -letal
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/etal.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/libetal.a
    INCLUDEPATH += $$PWD/etal/lib_mtd
    DEPENDPATH += $$PWD/etal/lib_mtd
}
}

INCLUDEPATH += $$PWD/etal
DEPENDPATH += $$PWD/etal

win32: LIBS += -L$$PWD/etal/ -lws2_32

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/ws2_32.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/libws2_32.a
}

#
# Configure Jasmin target
#
CONFIG_USE_JASMIN {

win32: LIBS += -L$$PWD/jasmin/bin/debug/ -ljasmin_lib

INCLUDEPATH +=  $$PWD/jasmin/src/Radio_AMFM_APP/inc \
                $$PWD/jasmin/src/Radio_AMFM_HAL/inc \
                $$PWD/jasmin/src/Radio_AMFM_TUNER_CTRL/inc \
                $$PWD/jasmin/src/Radio_DAB_APP/inc \
                $$PWD/jasmin/src/Radio_DAB_HAL/inc \
                $$PWD/jasmin/src/Radio_DAB_TUNER_CTRL/inc \
                $$PWD/jasmin/src/Radio_ETAL/inc \
                $$PWD/jasmin/src/Radio_Framework/inc/cfg \
                $$PWD/jasmin/src/Radio_Framework/inc/debug \
                $$PWD/jasmin/src/Radio_Framework/inc/etal \
                $$PWD/jasmin/src/Radio_Framework/inc/hsm \
                $$PWD/jasmin/src/Radio_Framework/inc/nvm \
                $$PWD/jasmin/src/Radio_Framework/inc/rtos \
                $$PWD/jasmin/src/Radio_Framework/inc/sys \
                $$PWD/jasmin/src/Radio_Framework/inc/uti_lib \
                $$PWD/jasmin/src/Radio_Framework/inc/win32 \
                $$PWD/jasmin/src/Radio_HMI_IF/inc \
                $$PWD/jasmin/src/Radio_Manager/inc \
                $$PWD/jasmin/src/Radio_Framework/inc/cfg

DEPENDPATH += $$PWD/jasmin/src/Radio_HMI_IF/inc

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/jasmin/bin/debug/jasmin_lib.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/jasmin/bin/debug/libjasmin_lib.a

HEADERS += \
    $$PWD/jasmin/src/Radio_HMI_IF/inc/hmi_if_app_request.h

CONFIG_BOARD_SIGLE_TUNER {
CONFIG( debug, debug|release ) {
    # debug
    win32: LIBS += -L$$PWD/etal/lib_std/ -letald
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/etald.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/libetald.a
    INCLUDEPATH += $$PWD/etal/lib_std
    DEPENDPATH += $$PWD/etal/lib_std
} else {
    # release
    win32: LIBS += -L$$PWD/etal/lib_std/ -letal
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/etal.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_std/libetal.a
    INCLUDEPATH += $$PWD/etal/lib_std
    DEPENDPATH += $$PWD/etal/lib_std
}
}

CONFIG_BOARD_DUAL_TUNER {
CONFIG( debug, debug|release ) {
    # debug
    win32: LIBS += -L$$PWD/etal/lib_mtd/ -letald
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/etald.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/libetald.a
    INCLUDEPATH += $$PWD/etal/lib_mtd
    DEPENDPATH += $$PWD/etal/lib_mtd
} else {
    # release
    win32: LIBS += -L$$PWD/etal/lib_mtd/ -letal
    win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/etal.lib
    else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/lib_mtd/libetal.a
    INCLUDEPATH += $$PWD/etal/lib_mtd
    DEPENDPATH += $$PWD/etal/lib_mtd
}
}

INCLUDEPATH += $$PWD/etal
DEPENDPATH += $$PWD/etal

win32: LIBS += -L$$PWD/etal/ -lws2_32

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/etal/ws2_32.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/etal/libws2_32.a
}

# End of file
