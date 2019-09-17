/*
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef CSE_HSM
    #define CSE_HSM
#endif

/*
 * Test mode dedicated for ST company
 */
//#define ST_TEST_MODE

/*
 * Add memory alias area alignment
 */
//#define MEM_ALIAS_BUF

/*
 * Add Verbose mode for debugging
 */
//#define VERBOSE
//#define VERBOSE_UNALIGNED_BUF

/*
 * Add performance measurement data
 */
#ifdef ST_TEST_MODE
#define PERF_MEASURMENT
#endif

/*
 * HSM Services useful for Runtime
 */
#define HSM_RELEASE

#define INCLUDE_SHA1
#define INCLUDE_SHA224
#define INCLUDE_SHA256

/*
 * Elliptic Curve configuration
 * NIST P256 is the default configuration
 * NIST P384 and P521 can be added by defining the two following constants
 */
#define INCLUDE_NIST_P384
#define INCLUDE_NIST_P521

#define INCLUDE_BRAINPOOL_P256R1
#define INCLUDE_BRAINPOOL_P384R1

/*
 * SHA algorithms configuration
 * can be used to support ECDSA signature with associated SHA algorithms
 * NIST P384: at least one among SHA384 and SHA512 can be selected
 * NIST P521: at least SHA512 must be selected
 */
#define INCLUDE_SHA384
#define INCLUDE_SHA512

#define ALIGN __attribute__((aligned(4)))
