#@DESCRIPTION: Car Processor STA1XXX machine features definitions

#------------------
#
# GPU
#
# to support gpu on a machine, please add "gcnanogpuimpl" on machine features
#MACHINE_FEATURES += "gcnanogpuimpl"

GPU_LIST          = "kernel-module-galcore ${ST_GCNANO_USERLAND_BACKEND}"
GPU_IMAGE_INSTALL = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${GPU_LIST}', '', d)} "
