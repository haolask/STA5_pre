PLATFORM_FLAVOR ?= 1385

include core/arch/arm/cpu/cortex-a7.mk

core_arm32-platform-aflags	+= -mfpu=neon

$(call force,CFG_ARM32_core,y)
$(call force,CFG_SECURE_TIME_SOURCE_REE,y)
$(call force,CFG_CACHE_API,y)
$(call force,CFG_PM_STUBS,y)
$(call force,CFG_GENERIC_BOOT,y)
$(call force,CFG_GIC,y)
$(call force,CFG_PL011,y)
$(call force,CFG_PM_ARM32,y)
$(call force,CFG_PSCI_ARM32,y)
$(call force,CFG_WITH_LPAE,y)

ta-targets = ta_arm32

CFG_MMAP_REGIONS ?= 18
CFG_CORE_HEAP_SIZE ?= 49152

CFG_WITH_PAGER ?= n

# SMP ?
CFG_BOOT_SYNC_CPU ?= y
CFG_BOOT_SECONDARY_REQUEST ?= y

CFG_TEE_CORE_EMBED_INTERNAL_TESTS ?= y
CFG_WITH_STACK_CANARIES ?= y
CFG_WITH_STATS ?= y

CFG_WITH_SOFTWARE_PRNG ?= y

ifeq ($(PLATFORM_FLAVOR),1385)
# In case of security and safety isolation some peripherals can only be
# accessed by a TEE, so the related register access requests from REE go
# through secure monitor (CFG_SM_PLATFORM_HANDLER).
# If CFG_STA_REMOTEPROC_CTRL is set, then read, write and update register
# requests are forwarded to the remote processor, relying on "regmap" mailbox
# channel.
# Otherwise, register requests are directly handled by the TEE.
CFG_SM_PLATFORM_HANDLER ?= y
CFG_STA_REMOTEPROC_CTRL ?= y

CFG_CRYPTO_HASH_FROM_CRYPTOLIB ?= y
CFG_CRYPTO_ECC_FROM_CRYPTOLIB ?= n
CFG_CRYPTO_RNG_FROM_CRYPTOLIB ?= n

# Returns 'y' if at least one variable is 'n', empty otherwise
cfg-one-disabled = $(if $(filter n, $(foreach var,$(1),$($(var)))),y,)
hsm-one-disabled = $(call cfg-one-disabled,$(foreach v,$(1),CFG_CRYPTO_$(v)_FROM_CRYPTOLIB))

CFG_CRYPTO_WITH_HSM := $(call hsm-one-disabled, HASH ECC RNG)
else
$(call force,CFG_CRYPTO_WITH_HSM,n)
$(call force,CFG_SM_PLATFORM_HANDLER,n)
$(call force,CFG_STA_REMOTEPROC_CTRL,n)
endif
