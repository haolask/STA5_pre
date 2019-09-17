/**
 * @file    CSE_ext_test_globals.h
 * @brief   global variables definition - used by test -  header file.
 */

#ifndef _CSE_EXT_TEST_GLOBALS_H_
#define _CSE_EXT_TEST_GLOBALS_H_

#include "cse_types.h"

 /* @addtogroup SHE-ext_driver
 * @{
 */

extern uint32_t G_asymKeyCounter;
extern uint32_t reduced_test_vector_set;
extern uint32_t WP_test_user_selected;
extern uint32_t WP_test_forced;

/* Used to perform all (RSA, ECC) tests with a reduced nb of vectors */
#define MAX_REDUCED_TV_NB 1


/**
 * @}
 */
#endif /* _CSE_EXT_TEST_GLOBALS_H_ */
