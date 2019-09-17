/**
 * @file sta_rtc.h
 * @brief Provide all the sta RTC driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

int rtc_get_time(uint32_t *time);
void rtc_set_time(uint32_t time);
int rtc_set_timeout(uint32_t timeout);
int rtc_init(void);
int rtc_deinit(void);

