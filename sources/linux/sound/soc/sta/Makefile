# STA Platform Support

snd-soc-sta-i2s-msp-objs := \
	sta_msp_dai.o \
	sta_pcm.o \
	sta_dmaengine_pcm.o
obj-$(CONFIG_SND_SOC_STA_MSP) += snd-soc-sta-i2s-msp.o

snd-soc-sta-dpmailbox-objs := sta_dpmailbox_dai.o
obj-$(CONFIG_SND_SOC_STA_DPMAILBOX) += snd-soc-sta-dpmailbox.o

snd-soc-sta-codec-aif-objs := sta_codec_aif.o
obj-$(CONFIG_SND_SOC_STA_AIF) += snd-soc-sta-codec-aif.o

snd-soc-sta-codec-msg-objs := sta_audio_msg.o
obj-$(CONFIG_SND_SOC_STA_AIF) += snd-soc-sta-codec-msg.o

snd-soc-sta-sai-objs := sta_sai.o
obj-$(CONFIG_SND_SOC_STA_AIF) += snd-soc-sta-sai.o

snd-soc-sta-card-objs := sta_card.o
obj-$(CONFIG_SND_SOC_STA)  += snd-soc-sta-card.o

snd-soc-sta-ext-dsp-objs := ext_codec_dsp.o
obj-$(CONFIG_SND_SOC_EXT_DSP)  += snd-soc-sta-ext-dsp.o

obj-$(CONFIG_SND_SOC_STA_STAUDIOLIB) += staudiolib/

