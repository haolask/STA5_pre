#if !defined PP_CTRL_H
#define PP_CTRL_H
#include "sta_ltdc.h"
#include "string.h"
#include "trace.h"
#include "ppapi.h"

struct user_ppconfig {
	bool deinterlace_needed;
	enum ltdc_pix_fmt input_fmt;
	uint32_t input_wdt;
	uint32_t input_hgt;
	uint32_t input_paddr;
	enum ltdc_pix_fmt output_fmt;
	uint32_t output_wdt;
	uint32_t output_hgt;
	uint32_t output_paddr;
};

void* g1_pp_open(void * ctx);
int g1_pp_close(void * inst);
int g1_pp_set_config(void * inst, struct user_ppconfig * usr_ppconfig);
int g1_pp_get_frame(void * inst, u32 ** bufferBusAddr);

#endif
