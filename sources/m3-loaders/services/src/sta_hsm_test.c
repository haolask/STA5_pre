/**
 * @file sta_hsm_test.c
 * @brief HSM tests file
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

/* Standard includes. */
#include <string.h>
#include <errno.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"

/* HSM framework includes */
#include "config.h"
#include "menu.h"
#include "menu_ext.h"

#ifdef ST_TEST_MODE
#include "menu_st.h"
#endif

#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif

/**
 * Function Name  : interactive_test
 * Description    : Run the tests in interactive mode with a dedicated menu.
 * Input          : None
 * Return         : None
 */
static void interactive_test(void)
{
	uint32_t verbose = 1;
	uint32_t ret = 0;

	TRACE_INFO("\nHSM tests %s %s \r\n", __DATE__, __TIME__);

	/* Display menu options via the UART */
	display_menu();
	display_menu_ext();
#ifdef ST_TEST_MODE
	display_menu_st();
#endif

	while (1) {
		int operation;

		/* Read user selection */
		operation = select_operation();

		/*
		 * Check it is a supported option and call the associated code
		 */
		ret = menu_entry(operation, &verbose);
		if (ret != 0)
			/* was not an option managed by 1st menu, try ext one */
			ret = menu_ext_entry(operation, &verbose);

#ifdef ST_TEST_MODE
		if (ret != 0)
			/* was not an option managed by 2nd menu, try rom one */
			ret = menu_st_entry(operation, &verbose);
#endif

		if (ret != 0) {
			display_menu();
			display_menu_ext();
#ifdef ST_TEST_MODE
			display_menu_st();
#endif
		}
	}
}

/**
 * Function Name  : hsm_test_menu_task
 * Description    : Main Test function
 * Input          : None
 * Return         : None
 */
void hsm_test_menu_task(void)
{
	/*
	 * In case no event is triggered, let's proceed with booting
	 * This entity is in charge of boting directly
	 */
	TRACE_INFO("Running interactive test ...\n");

	interactive_test();

	vTaskDelete(NULL);
}

