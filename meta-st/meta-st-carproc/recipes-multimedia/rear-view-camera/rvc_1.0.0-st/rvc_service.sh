#!/bin/bash -
#===============================================================================
#
#          FILE: rvc_service.sh
#
#         USAGE: ./rvc_service.sh
#
#   DESCRIPTION: Use to probe RVC Daemon Driver and RVC Handler
#
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Pierre-Yves MORDRET
#  ORGANIZATION: STMicroelectronics
#     COPYRIGHT: Copyright (C) 2016, STMicroelectronics - All Rights Reserved
#       CREATED: 11/16/2016 16:55
#      REVISION: 1.1.0
#===============================================================================

set -o nounset                              # Treat unset variables as an error

GST_REGISTRY_UPDATE="no" rvc_handler --libpath=/lib/modules/`uname -r` --iomode=4 -d -r -W 800 -H 480

exit 0


