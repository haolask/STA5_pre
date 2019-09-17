obj-$(CONFIG_ARCH_STA) += sta.o ramdump.o
ifneq ($(CONFIG_ARM_PSCI),y)
obj-$(CONFIG_SMP) += platsmp.o headsmp.o
obj-$(CONFIG_SUSPEND) += pm.o sleep.o
endif
