#-------------------------------------------------
#
# Project created by QtCreator 2018-01-18T13:20:57
#
#-------------------------------------------------
QT       -= gui

TARGET = jasmin_lib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += JASMIN_LIB_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH +=  ../src/Radio_AMFM_APP/inc \
                ../src/Radio_AMFM_HAL/inc \
                ../src/Radio_AMFM_TUNER_CTRL/inc \
                ../src/Radio_DAB_APP/inc \
                ../src/Radio_DAB_HAL/inc \
                ../src/Radio_DAB_TUNER_CTRL/inc \
                ../src/Radio_ETAL/inc \
                ../src/Radio_Framework/inc/cfg \
                ../src/Radio_Framework/inc/debug \
                ../src/Radio_Framework/inc/etal \
                ../src/Radio_Framework/inc/hsm \
                ../src/Radio_Framework/inc/nvm \
                ../src/Radio_Framework/inc/rtos \
                ../src/Radio_Framework/inc/sys \
                ../src/Radio_Framework/inc/uti_lib \
                ../src/Radio_Framework/inc/win32 \
                ../src/Radio_HMI_IF/inc \
                ../src/Radio_Manager/inc


SOURCES += \
    ../src/Radio_AMFM_APP/src/amfm_app_application.c \
    ../src/Radio_AMFM_APP/src/amfm_app_hsm.c \
    ../src/Radio_AMFM_APP/src/amfm_app_inst_hsm.c \
    ../src/Radio_AMFM_APP/src/amfm_app_notify.c \
    ../src/Radio_AMFM_APP/src/amfm_app_request.c \
    ../src/Radio_AMFM_APP/src/amfm_app_response.c \
    ../src/Radio_AMFM_APP/src/interpolation.c \
    ../src/Radio_AMFM_HAL/src/amfm_hal_interface.c \
    ../src/Radio_AMFM_HAL/src/AMFM_HAL_RDS_Collector.c \
    ../src/Radio_AMFM_HAL/src/AMFM_HAL_RDS_decoder.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_App.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_Instance_hsm.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_Main_hsm.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_Notify.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_Request.c \
    ../src/Radio_AMFM_TUNER_CTRL/src/AMFM_Tuner_Ctrl_Response.c \
    ../src/Radio_DAB_APP/src/dab_app.c \
    ../src/Radio_DAB_APP/src/dab_app_freq_band.c \
    ../src/Radio_DAB_APP/src/dab_app_hsm.c \
    ../src/Radio_DAB_APP/src/dab_app_inst_hsm.c \
    ../src/Radio_DAB_APP/src/dab_app_notify.c \
    ../src/Radio_DAB_APP/src/dab_app_request.c \
    ../src/Radio_DAB_APP/src/dab_app_response.c \
    ../src/Radio_DAB_APP/src/dab_app_stationlist.c \
    ../src/Radio_DAB_APP/src/msg_cmn.c \
    ../src/Radio_DAB_HAL/src/DAB_HAL_FIC_Parsing.c \
    ../src/Radio_DAB_HAL/src/DAB_HAL_Fig_Decoder.c \
    ../src/Radio_DAB_HAL/src/DAB_HAL_Interface.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_Announcement.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_app.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_inst_hsm.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_Linking.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_main_hsm.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_Notify.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_Request.c \
    ../src/Radio_DAB_TUNER_CTRL/src/DAB_Tuner_Ctrl_Response.c \
    ../src/Radio_Framework/src/cfg/cfg_variant_market.c \
    ../src/Radio_Framework/src/debug/debug_log.c \
    ../src/Radio_Framework/src/etal/etal.c \
    ../src/Radio_Framework/src/hsm/hsm_implementation.c \
    ../src/Radio_Framework/src/nvm/sys_nvm.c \
    ../src/Radio_Framework/src/rtos/osal.c \
    ../src/Radio_Framework/src/sys/sys_message.c \
    ../src/Radio_Framework/src/sys/sys_task.c \
    ../src/Radio_Framework/src/sys/sys_timer.c \
    ../src/Radio_Framework/src/sys/Tuner_core_sys_main.c \
    ../src/Radio_Framework/src/uti_lib/lib_string.c \
    ../src/Radio_Framework/src/win32/win32_os.c \
    ../src/Radio_Framework/src/win32/win32_os_timer.c \
    ../src/Radio_HMI_IF/src/hmi_if_app.c \
    ../src/Radio_HMI_IF/src/hmi_if_app_notify.c \
    ../src/Radio_HMI_IF/src/hmi_if_app_request.c \
    ../src/Radio_HMI_IF/src/hmi_if_utf8_conversion.c \
    ../src/Radio_Manager/src/radio_mngr_app.c \
    ../src/Radio_Manager/src/radio_mngr_app_hsm.c \
    ../src/Radio_Manager/src/radio_mngr_app_inst_hsm.c \
    ../src/Radio_Manager/src/radio_mngr_app_notify.c \
    ../src/Radio_Manager/src/radio_mngr_app_request.c \
    ../src/Radio_Manager/src/radio_mngr_app_response.c


HEADERS += \
    ../src/Radio_AMFM_APP/inc/amfm_app_application.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_hsm.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_inst_hsm.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_market.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_msg.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_msg_id.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_notify.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_request.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_response.h \
    ../src/Radio_AMFM_APP/inc/amfm_app_types.h \
    ../src/Radio_AMFM_APP/inc/interpolation.h \
    ../src/Radio_AMFM_HAL/inc/AMFM_HAL_Interface.h \
    ../src/Radio_AMFM_HAL/inc/AMFM_HAL_RDS_Collector.h \
    ../src/Radio_AMFM_HAL/inc/AMFM_HAL_RDS_decoder.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_App.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Instance_hsm.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Main_hsm.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Notify.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Request.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Response.h \
    ../src/Radio_AMFM_TUNER_CTRL/inc/AMFM_Tuner_Ctrl_Types.h \
    ../src/Radio_DAB_APP/inc/dab_app.h \
    ../src/Radio_DAB_APP/inc/dab_app_extern.h \
    ../src/Radio_DAB_APP/inc/dab_app_freq_band.h \
    ../src/Radio_DAB_APP/inc/dab_app_hsm.h \
    ../src/Radio_DAB_APP/inc/dab_app_inst_hsm.h \
    ../src/Radio_DAB_APP/inc/dab_app_msg_id.h \
    ../src/Radio_DAB_APP/inc/dab_app_notify.h \
    ../src/Radio_DAB_APP/inc/dab_app_request.h \
    ../src/Radio_DAB_APP/inc/dab_app_response.h \
    ../src/Radio_DAB_APP/inc/dab_app_stationlist.h \
    ../src/Radio_DAB_APP/inc/dab_app_types.h \
    ../src/Radio_DAB_APP/inc/msg_cmn.h \
    ../src/Radio_DAB_HAL/inc/DAB_HAL_FIC_Parsing.h \
    ../src/Radio_DAB_HAL/inc/DAB_HAL_Fig_Decoder.h \
    ../src/Radio_DAB_HAL/inc/DAB_HAL_Interface.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_Announcement.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_app.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_inst_hsm.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_main_hsm.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_Notify.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_Request.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_Response.h \
    ../src/Radio_DAB_TUNER_CTRL/inc/DAB_Tuner_Ctrl_Types.h \
    ../src/Radio_ETAL/inc/etal_api.h \
    ../src/Radio_ETAL/inc/etal_types.h \
    ../src/Radio_ETAL/inc/etalversion.h \
    ../src/Radio_Framework/inc/cfg/cfg_types.h \
    ../src/Radio_Framework/inc/cfg/cfg_variant_market.h \
    ../src/Radio_Framework/inc/debug/debug_log.h \
    ../src/Radio_Framework/inc/etal/etal.h \
    ../src/Radio_Framework/inc/hsm/hsm_api.h \
    ../src/Radio_Framework/inc/nvm/sys_nvm.h \
    ../src/Radio_Framework/inc/rtos/osal_api.h \
    ../src/Radio_Framework/inc/rtos/osal_private.h \
    ../src/Radio_Framework/inc/sys/sys_message.h \
    ../src/Radio_Framework/inc/sys/sys_task.h \
    ../src/Radio_Framework/inc/sys/sys_timer.h \
    ../src/Radio_Framework/inc/sys/Tuner_core_sys_main.h \
    ../src/Radio_Framework/inc/uti_lib/lib_bitmanip.h \
    ../src/Radio_Framework/inc/uti_lib/lib_string.h \
    ../src/Radio_Framework/inc/win32/win32_os_api.h \
    ../src/Radio_Framework/inc/win32/win32_os_private.h \
    ../src/Radio_Framework/inc/win32/win32_os_timer.h \
    ../src/Radio_HMI_IF/inc/hmi_if_app.h \
    ../src/Radio_HMI_IF/inc/hmi_if_app_notify.h \
    ../src/Radio_HMI_IF/inc/hmi_if_app_request.h \
    ../src/Radio_HMI_IF/inc/hmi_if_common.h \
    ../src/Radio_HMI_IF/inc/hmi_if_extern.h \
    ../src/Radio_HMI_IF/inc/hmi_if_utf8_conversion.h \
    ../src/Radio_HMI_IF/inc/IRadio.h \
    ../src/Radio_Manager/inc/radio_mngr_app.h \
    ../src/Radio_Manager/inc/radio_mngr_app_hsm.h \
    ../src/Radio_Manager/inc/radio_mngr_app_inst_hsm.h \
    ../src/Radio_Manager/inc/radio_mngr_app_notify.h \
    ../src/Radio_Manager/inc/radio_mngr_app_request.h \
    ../src/Radio_Manager/inc/radio_mngr_app_response.h \
    ../src/Radio_Manager/inc/radio_mngr_app_types.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
