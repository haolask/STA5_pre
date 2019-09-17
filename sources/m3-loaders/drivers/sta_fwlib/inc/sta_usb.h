/**
 * @file sta_usb.h
 * @brief USB utilities header
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _USB_H_
#define _USB_H_

/**
 * @brief	initializes USB physical layer (basically remove PLL power down)
 * @return	0 if no error, not 0 otherwise
 */
int usb_phy_init(void);

#endif /* _USB_H_ */

