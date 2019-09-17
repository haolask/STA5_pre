DISABLE_GPLv3_PACKAGES ?= ""
LICENSE_FLAGS_WHITELIST += "${@bb.utils.contains('DISABLE_GPLv3_PACKAGES', 'true', '', ' commercial', d)}"
