/*
 * Copyright (c) 2015, STMicroelectronics - All Rights Reserved.
 * Author(s): Gerald Lejeune (gerald.lejeune@st.com) for STMicroelectronics.
 *
 * License terms: BSD 3 Clauses.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holder(s) nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MAILBOXES_H__
#define __MAILBOXES_H__

#define WAKE_UP_CONTROL_WORD            0x656b6157
#define SET_WAKE_UP_ADDRESS             0
#define CLEAR_MAILBOX                   1

#define MAILBOX_SIZE                    8
#define MAILBOX_CONTROL_WORD            0
#define MAILBOX_SEC_ENTRYPOINT          4

#ifndef ASM

struct mailbox_t {
	unsigned int control_word;
	unsigned int sec_entrypoint;
} mailbox;

/*******************************************************************************
 * Macro to flag a compile time assertion. It uses the preprocessor to generate
 * an invalid C construct if 'cond' evaluates to false.
 * The following compilation error is triggered if the assertion fails:
 * "error: size of array 'msg' is negative"
 * The 'unused' attribute ensures that the unused typedef does not emit a
 * compiler warning.
 ******************************************************************************/
#define CASSERT(cond, msg)	\
	typedef char msg[(cond) ? 1 : -1] __unused

/*
 * Check that defines for assembly are consistent with C structure size and
 * offsets.
 */
CASSERT(sizeof(struct mailbox_t) == MAILBOX_SIZE, mailbox_size_mismatch);

CASSERT(offsetof(struct mailbox_t, control_word) == MAILBOX_CONTROL_WORD,
	mailbox_control_word_offset_mismatch);

CASSERT(offsetof(struct mailbox_t, sec_entrypoint) == MAILBOX_SEC_ENTRYPOINT,
	mailbox_sec_entrypoint_offset_mismatch);

#endif /* ASM */

#endif /* __MAILBOXES_H__ */
