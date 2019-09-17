#ifndef __STA_REGMAP_H__
#define __STA_REGMAP_H__

#define STA_REGMAP_TYPE_MSK	0xFF
#define STA_REGMAP_READ		0x0
#define STA_REGMAP_WRITE	0x1
#define STA_REGMAP_UPDATE_BITS	0x2

#define STA_REGMAP_VALBITS_8	8
#define STA_REGMAP_VALBITS_16	16
#define STA_REGMAP_VALBITS_32	32
#define STA_REGMAP_VALBITS_64	64

/* REGMAP error codes */
#define STA_REGMAP_OK			0x0
#define STA_REGMAP_NOT_SUPPORTED	-1
#define STA_REGMAP_FAILED		-2
#define STA_REGMAP_INVALID_PARAMS	-3

struct regmap_request {
	uint8_t type;
	uint8_t status;
	uint8_t val_bits;
	uint8_t padding;
	uint32_t reg;
	uint32_t mask;
	uint32_t val;
};

int regmap_init(void);

#endif /* __STA_REGMAP_H__ */
