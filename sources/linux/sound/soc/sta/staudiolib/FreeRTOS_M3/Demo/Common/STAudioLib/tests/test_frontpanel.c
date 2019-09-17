/*
 * STAudioLib - test_frontpanel.c
 *
 * Created on 2013/10/23 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 test application
 */


#if 1
#include "common.h"
#include "A2evb_frontpanel.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

static void rtxCallback(int steps)
{
	PRINTF("ROTARY = %d\n", steps);

	//TMP
	uprintf(3, "UART3 %d\n", steps);
//	uprintf(0, "UART0 %d\n", steps);
}

static void keyCallback(int key, bool down)
{
	PRINTF("KEY = %x\n", key);
}

//---------------------------------------------------------

void TestFrontPanel(void)
{
	PRINTF("TestFrontPanel...\n");

	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	while (1)
		SLEEP(5);
}

//S_GPIO->gpio_mis
//S_GPIO->gpio_ic
//R4_GPIO0->gpio_mis
//R4_GPIO0->gpio_ic
//R4_GPIO1->gpio_mis
//R4_GPIO1->gpio_ic

//S_GPIO->gpio_dat
//R4_GPIO0->gpio_dat
//R4_GPIO1->gpio_dat

#endif //0

