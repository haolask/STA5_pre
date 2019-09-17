#@DESCRIPTION: Car Processor STA1XXX default providers

MULTI_PROVIDER_WHITELIST += "virtual/libgles1 virtual/libgles2 virtual/egl virtual/mesa"

ST_GCNANO_USERLAND_BACKEND = "gcnano-userland-multi-binary"
# virtual/libgles1
PREFERRED_PROVIDER_virtual/libgles1 = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${ST_GCNANO_USERLAND_BACKEND}', 'mesa', d)}"
# virtual/libgles2
PREFERRED_PROVIDER_virtual/libgles2 = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${ST_GCNANO_USERLAND_BACKEND}', 'mesa', d)}"
# virtual/egl
PREFERRED_PROVIDER_virtual/egl = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${ST_GCNANO_USERLAND_BACKEND}', 'mesa', d)}"
# virtual/mesa
#PREFERRED_PROVIDER_virtual/mesa = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${ST_GCNANO_USERLAND_BACKEND}', 'mesa', d)}"
# virtual/mesa
PREFERRED_PROVIDER_virtual/mesa = "${@bb.utils.contains('MACHINE_FEATURES', 'gcnanogpuimpl', '${ST_GCNANO_USERLAND_BACKEND}', 'mesa', d)}"
