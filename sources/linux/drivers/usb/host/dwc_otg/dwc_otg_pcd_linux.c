 /* ==========================================================================
  * File: //dwh/usb_iip/dev/software/otg/linux/drivers/dwc_otg_pcd_linux.c
  * Revision: #30
  * Date: 2015/08/06
  * Change: 2913039
  *
  * Synopsys HS OTG Linux Software Driver and documentation (hereinafter,
  * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
  * otherwise expressly agreed to in writing between Synopsys and you.
  *
  * The Software IS NOT an item of Licensed Software or Licensed Product under
  * any End User Software License Agreement or Agreement for Licensed Product
  * with Synopsys or any supplement thereto. You are permitted to use and
  * redistribute this Software in source and binary forms, with or without
  * modification, provided that redistributions of source code must retain this
  * notice. You may not view, use, disclose, copy or distribute this file or
  * any information contained herein except pursuant to this license grant from
  * Synopsys. If you do not agree with this notice, including the disclaimer
  * below, then you are not authorized to use the Software.
  *
  * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
  * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
  * DAMAGE.
  * ========================================================================== */
#ifndef DWC_HOST_ONLY

/** @file
 * This file implements the Peripheral Controller Driver.
 *
 * The Peripheral Controller Driver (PCD) is responsible for
 * translating requests from the Function Driver into the appropriate
 * actions on the DWC_otg controller. It isolates the Function Driver
 * from the specifics of the controller by providing an API to the
 * Function Driver.
 *
 * The Peripheral Controller Driver for Linux will implement the
 * Gadget API, so that the existing Gadget drivers can be used.
 * (Gadget Driver is the Linux terminology for a Function Driver.)
 *
 * The Linux Gadget API is defined in the header file
 * <code><linux/usb_gadget.h></code>.  The USB EP operations API is
 * defined in the structure <code>usb_ep_ops</code> and the USB
 * Controller API is defined in the structure
 * <code>usb_gadget_ops</code>.
 *
 */

#include "dwc_otg_os_dep.h"
#include "dwc_otg_pcd_if.h"
#include "dwc_otg_pcd.h"
#include "dwc_otg_driver.h"
#include "dwc_otg_dbg.h"

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/of_platform.h>

#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>

#define VBUS_MONITORING 2000

struct gadget_wrapper_ep_t {
	struct gadget_wrapper_t *gadget_wrapper;
	struct usb_ep ep;
};


struct gadget_wrapper_t {
	dwc_otg_pcd_t *pcd;

	struct usb_gadget gadget;
	struct usb_gadget_driver *driver;

	struct gadget_wrapper_ep_t ep0;
	struct gadget_wrapper_ep_t in_ep[16];
	struct gadget_wrapper_ep_t out_ep[16];

};

static inline struct gadget_wrapper_t *ep_to_gadget_wrapper(struct usb_ep *ep)
{
	struct gadget_wrapper_ep_t *ptr = container_of(ep,
						struct gadget_wrapper_ep_t,
						ep);

	if (ptr)
		return ptr->gadget_wrapper;

	return NULL;
}

static inline struct gadget_wrapper_t *gadget_to_gadget_wrapper(
						struct usb_gadget *gadget)
{
	return container_of(gadget, struct gadget_wrapper_t, gadget);
}

/* Display the contents of the buffer */
extern void dump_msg(const u8 * buf, unsigned int length);
/**
 * Get the dwc_otg_pcd_ep_t* from usb_ep* pointer - NULL in case
 * if the endpoint is not found
 */
static struct dwc_otg_pcd_ep *ep_from_handle(dwc_otg_pcd_t * pcd, void *handle)
{
	int i;
	if (pcd->ep0.priv == handle) {
		return &pcd->ep0;
	}

	for (i = 0; i < MAX_EPS_CHANNELS - 1; i++) {
		if (pcd->in_ep[i].priv == handle)
			return &pcd->in_ep[i];
		if (pcd->out_ep[i].priv == handle)
			return &pcd->out_ep[i];
	}

	return NULL;
}

/* USB Endpoint Operations */
/*
 * The following sections briefly describe the behavior of the Gadget
 * API endpoint operations implemented in the DWC_otg driver
 * software. Detailed descriptions of the generic behavior of each of
 * these functions can be found in the Linux header file
 * include/linux/usb_gadget.h.
 *
 * The Gadget API provides wrapper functions for each of the function
 * pointers defined in usb_ep_ops. The Gadget Driver calls the wrapper
 * function, which then calls the underlying PCD function. The
 * following sections are named according to the wrapper
 * functions. Within each section, the corresponding DWC_otg PCD
 * function name is specified.
 *
 */

/**
 * This function is called by the Gadget Driver for each EP to be
 * configured for the current configuration (SET_CONFIGURATION).
 *
 * This function initializes the dwc_otg_ep_t data structure, and then
 * calls dwc_otg_ep_activate.
 */
static int ep_enable(struct usb_ep *usb_ep,
		     const struct usb_endpoint_descriptor *ep_desc)
{
	int retval;
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);

	DWC_DEBUGPL(DBG_PCD, "%s(%p,%p)\n", __func__, usb_ep, ep_desc);

	if (!usb_ep || !gadget_wrapper || !ep_desc ||
	    (ep_desc->bDescriptorType != USB_DT_ENDPOINT)) {
		DWC_WARN("%s, bad ep or descriptor\n", __func__);
		return -EINVAL;
	}

	/* Check FIFO size? */
	if (!ep_desc->wMaxPacketSize) {
		DWC_WARN("%s, bad %s maxpacket\n", __func__, usb_ep->name);
		return -ERANGE;
	}

	if (!gadget_wrapper->driver ||
	    gadget_wrapper->gadget.speed == USB_SPEED_UNKNOWN) {
		DWC_WARN("%s, bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

	/* Delete after check - MAS */
#if 0
	nat = (uint32_t) ep_desc->wMaxPacketSize;
	printk(KERN_ALERT "%s: nat (before) =%d\n", __func__, nat);
	nat = (nat >> 11) & 0x03;
	printk(KERN_ALERT "%s: nat (after) =%d\n", __func__, nat);
#endif
	retval = dwc_otg_pcd_ep_enable(gadget_wrapper->pcd,
				       (const uint8_t *)ep_desc,
				       (void *)usb_ep);
	if (retval) {
		DWC_WARN("dwc_otg_pcd_ep_enable failed\n");
		return -EINVAL;
	}

	usb_ep->maxpacket = le16_to_cpu(ep_desc->wMaxPacketSize);

	return 0;
}

/**
 * This function is called when an EP is disabled due to disconnect or
 * change in configuration. Any pending requests will terminate with a
 * status of -ESHUTDOWN.
 *
 * This function modifies the dwc_otg_ep_t data structure for this EP,
 * and then calls dwc_otg_ep_deactivate.
 */
static int ep_disable(struct usb_ep *usb_ep)
{
	int retval;
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);

	if (!usb_ep || !gadget_wrapper) {
		DWC_WARN("bad ep\n");
		return -EINVAL;
	}

	DWC_DEBUGPL(DBG_PCD, "%s(%p)\n", __func__, usb_ep);
	if (!usb_ep) {
		DWC_DEBUGPL(DBG_PCD, "%s, %s not enabled\n", __func__,
			    usb_ep ? usb_ep->name : NULL);
		return -EINVAL;
	}

	retval = dwc_otg_pcd_ep_disable(gadget_wrapper->pcd, usb_ep);
	if (retval) {
		retval = -EINVAL;
	}

	return retval;
}

/**
 * This function allocates a request object to use with the specified
 * endpoint.
 *
 * @param ep The endpoint to be used with with the request
 * @param gfp_flags the GFP_* flags to use.
 */
static struct usb_request *dwc_otg_pcd_alloc_request(struct usb_ep *ep,
						     gfp_t gfp_flags)
{
	struct usb_request *usb_req;

	DWC_DEBUGPL(DBG_PCDV, "%s(%p,%d)\n", __func__, ep, gfp_flags);
	if (0 == ep) {
		DWC_WARN("%s() %s\n", __func__, "Invalid EP!\n");
		return 0;
	}
	usb_req = kmalloc(sizeof(*usb_req), gfp_flags);
	if (0 == usb_req) {
		DWC_WARN("%s() %s\n", __func__, "request allocation failed!\n");
		return 0;
	}
	memset(usb_req, 0, sizeof(*usb_req));
	usb_req->dma = DWC_DMA_ADDR_INVALID;

	return usb_req;
}

/**
 * This function frees a request object.
 *
 * @param ep The endpoint associated with the request
 * @param req The request being freed
 */
static void dwc_otg_pcd_free_request(struct usb_ep *ep, struct usb_request *req)
{
	DWC_DEBUGPL(DBG_PCDV, "%s(%p,%p)\n", __func__, ep, req);

	if (0 == ep || 0 == req) {
		DWC_WARN("%s() %s\n", __func__,
			 "Invalid ep or req argument!\n");
		return;
	}

	kfree(req);
}

/**
 * This function is used to submit an I/O Request to an EP.
 *
 *	- When the request completes the request's completion callback
 *	  is called to return the request to the driver.
 *	- An EP, except control EPs, may have multiple requests
 *	  pending.
 *	- Once submitted the request cannot be examined or modified.
 *	- Each request is turned into one or more packets.
 *	- A BULK EP can queue any amount of data; the transfer is
 *	  packetized.
 *	- Zero length Packets are specified with the request 'zero'
 *	  flag.
 */
static int ep_queue(struct usb_ep *usb_ep, struct usb_request *usb_req,
		    gfp_t gfp_flags)
{
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);
	struct dwc_otg_pcd_ep *ep;
	int retval, is_isoc_ep, is_in_ep;
	dma_addr_t dma_addr;

	DWC_DEBUGPL(DBG_PCDV, "%s(%p,%p,%d)\n",
		    __func__, usb_ep, usb_req, gfp_flags);

	if (!usb_req || !usb_req->complete || !usb_req->buf) {
		DWC_WARN("bad params\n");
		return -EINVAL;
	}

	if (!usb_ep || !gadget_wrapper) {
		DWC_WARN("bad ep\n");
		return -EINVAL;
	}

	if (!gadget_wrapper->driver ||
	    gadget_wrapper->gadget.speed == USB_SPEED_UNKNOWN) {
		DWC_DEBUGPL(DBG_PCD, "gadget.speed=%d\n",
			    gadget_wrapper->gadget.speed);
		DWC_WARN("bogus device state\n");
		return -ESHUTDOWN;
	}

	DWC_DEBUGPL(DBG_PCDV, "%s queue req %p, len %d buf %p\n",
		    usb_ep->name, usb_req, usb_req->length, usb_req->buf);

	usb_req->status = -EINPROGRESS;
	usb_req->actual = 0;

	ep = ep_from_handle(gadget_wrapper->pcd, usb_ep);
	if (ep == NULL) {
		is_isoc_ep = 0;
		is_in_ep = 0;
	} else {
		is_isoc_ep = (ep->dwc_ep.type == DWC_OTG_EP_TYPE_ISOC) ? 1 : 0;
		is_in_ep = ep->dwc_ep.is_in;
	}

	dma_addr = usb_req->dma;
	if (GET_CORE_IF(gadget_wrapper->pcd)->dma_enable) {
#ifdef PCI_INTERFACE
		struct pci_dev *dev =
		    gadget_wrapper->pcd->otg_dev->os_dep.pcidev;
		if (dma_addr == DWC_DMA_ADDR_INVALID) {
			if (usb_req->length != 0) {
				dma_addr = pci_map_single(dev, usb_req->buf,
					usb_req->length, is_in_ep ?
					PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
				usb_req->dma = dma_addr;
			} else {
				dma_addr = 0;
			}
		}
#elif defined(PLATFORM_INTERFACE)
		dwc_otg_device_t *otg_dev = gadget_wrapper->pcd->otg_dev;
		struct device *dev = NULL;

		if (otg_dev != NULL)
			dev = DWC_OTG_OS_GETDEV(otg_dev->os_dep);

		if (dma_addr == DWC_DMA_ADDR_INVALID) {
			if (usb_req->length != 0) {
				dma_addr =
					dma_map_single(dev, usb_req->buf,
						       usb_req->length,
						       is_in_ep ?
						       DMA_TO_DEVICE
						       : DMA_FROM_DEVICE);
			}
			usb_req->dma = dma_addr;
		}
#endif
	}

#ifdef DWC_UTE_PER_IO
	if (is_isoc_ep == 1) {
		retval =
		    dwc_otg_pcd_xiso_ep_queue(gadget_wrapper->pcd, usb_ep,
					      usb_req->buf,
					      dma_addr, usb_req->length,
					      usb_req->zero, usb_req,
					      gfp_flags,
					      &usb_req->ext_req);
		if (retval)
			return -EINVAL;

		return 0;
	}
#endif
	retval = dwc_otg_pcd_ep_queue(gadget_wrapper->pcd, usb_ep,
				      usb_req->buf, dma_addr,
				      usb_req->length, usb_req->zero, usb_req,
				      gfp_flags);
	if (retval) {
		return -EINVAL;
	}

	return 0;
}

/**
 * This function cancels an I/O request from an EP.
 */
static int ep_dequeue(struct usb_ep *usb_ep, struct usb_request *usb_req)
{
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);

	DWC_DEBUGPL(DBG_PCDV, "%s(%p,%p)\n", __func__, usb_ep, usb_req);

	if (!usb_ep || !gadget_wrapper) {
			DWC_WARN("bad argument\n");
			return -EINVAL;
		}

	if (!gadget_wrapper->driver ||
	    gadget_wrapper->gadget.speed == USB_SPEED_UNKNOWN) {
		DWC_WARN("bogus device state\n");
		return -ESHUTDOWN;
	}
	if (dwc_otg_pcd_ep_dequeue(gadget_wrapper->pcd, usb_ep, usb_req)) {
		return -EINVAL;
	}

	return 0;
}

/**
 * usb_ep_set_halt stalls an endpoint.
 *
 * usb_ep_clear_halt clears an endpoint halt and resets its data
 * toggle.
 *
 * Both of these functions are implemented with the same underlying
 * function. The behavior depends on the value argument.
 *
 * @param[in] usb_ep the Endpoint to halt or clear halt.
 * @param[in] value
 *	- 0 means clear_halt.
 *	- 1 means set_halt,
 *	- 2 means clear stall lock flag.
 *	- 3 means set  stall lock flag.
 */
static int ep_halt(struct usb_ep *usb_ep, int value)
{
	int retval = 0;
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);


	DWC_DEBUGPL(DBG_PCD, "HALT %s %d\n", usb_ep->name, value);

	if (!usb_ep || !gadget_wrapper) {
		DWC_WARN("bad ep\n");
		return -EINVAL;
	}

	retval = dwc_otg_pcd_ep_halt(gadget_wrapper->pcd, usb_ep, value);
	if (retval == -DWC_E_AGAIN) {
		return -EAGAIN;
	} else if (retval) {
		retval = -EINVAL;
	}

	return retval;
}

static int ep_wedge(struct usb_ep *usb_ep)
{
	DWC_DEBUGPL(DBG_PCD, "WEDGE %s\n", usb_ep->name);

	return ep_halt(usb_ep, 3);
}

#ifdef DWC_EN_ISOC
/**
 * This function is used to submit an ISOC Transfer Request to an EP.
 *
 *	- Every time a sync period completes the request's completion callback
 *	  is called to provide data to the gadget driver.
 *	- Once submitted the request cannot be modified.
 *	- Each request is turned into periodic data packets untill ISO
 *	  Transfer is stopped..
 */
static int iso_ep_start(struct usb_ep *usb_ep, struct usb_iso_request *req,
			gfp_t gfp_flags)
{
	int retval = 0;
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);


	if (!req || !req->process_buffer || !req->buf0 || !req->buf1) {
		DWC_WARN("bad params\n");
		return -EINVAL;
	}

	if (!usb_ep || !gadget_wrapper)
		DWC_WARN("bad ep\n");

	req->status = -EINPROGRESS;

	retval =
	    dwc_otg_pcd_iso_ep_start(gadget_wrapper->pcd, usb_ep, req->buf0,
				     req->buf1, req->dma0, req->dma1,
				     req->sync_frame, req->data_pattern_frame,
				     req->data_per_frame,
				     req->
				     flags & USB_REQ_ISO_ASAP ? -1 :
				     req->start_frame, req->buf_proc_intrvl,
				     req, gfp_flags == GFP_ATOMIC ? 1 : 0);

	if (retval)
		return -EINVAL;

	return retval;
}

/**
 * This function stops ISO EP Periodic Data Transfer.
 */
static int iso_ep_stop(struct usb_ep *usb_ep, struct usb_iso_request *req)
{
	int retval = 0;
	struct gadget_wrapper_t *gadget_wrapper = ep_to_gadget_wrapper(usb_ep);

	if (!usb_ep || !gadget_wrapper)
		DWC_WARN("bad ep\n");

	if (!gadget_wrapper->driver ||
	    gadget_wrapper->gadget.speed == USB_SPEED_UNKNOWN) {
		DWC_DEBUGPL(DBG_PCD, "gadget.speed=%d\n",
			    gadget_wrapper->gadget.speed);
		DWC_WARN("bogus device state\n");
	}

	dwc_otg_pcd_iso_ep_stop(gadget_wrapper->pcd, usb_ep, req);
	if (retval) {
		retval = -EINVAL;
	}

	return retval;
}

static struct usb_iso_request *alloc_iso_request(struct usb_ep *ep,
						 int packets, gfp_t gfp_flags)
{
	struct usb_iso_request *pReq = NULL;
	uint32_t req_size;

	req_size = sizeof(struct usb_iso_request);
	req_size +=
	    (2 * packets * (sizeof(struct usb_gadget_iso_packet_descriptor)));

	pReq = kmalloc(req_size, gfp_flags);
	if (!pReq) {
		DWC_WARN("Can't allocate Iso Request\n");
		return 0;
	}
	pReq->iso_packet_desc0 = (void *)(pReq + 1);

	pReq->iso_packet_desc1 = pReq->iso_packet_desc0 + packets;

	return pReq;
}

static void free_iso_request(struct usb_ep *ep, struct usb_iso_request *req)
{
	kfree(req);
}

static struct usb_isoc_ep_ops dwc_otg_pcd_ep_ops = {
	.ep_ops = {
		.enable = ep_enable,
		.disable = ep_disable,
		.alloc_request = dwc_otg_pcd_alloc_request,
		.free_request = dwc_otg_pcd_free_request,
		.queue = ep_queue,
		.dequeue = ep_dequeue,
		.set_halt = ep_halt,
		.set_wedge = ep_wedge,
		.fifo_status = 0,
		.fifo_flush = 0,
	},

	.iso_ep_start = iso_ep_start,
	.iso_ep_stop = iso_ep_stop,
	.alloc_iso_request = alloc_iso_request,
	.free_iso_request = free_iso_request,
};

#else

static struct usb_ep_ops dwc_otg_pcd_ep_ops = {
	.enable = ep_enable,
	.disable = ep_disable,

	.alloc_request = dwc_otg_pcd_alloc_request,
	.free_request = dwc_otg_pcd_free_request,

	.queue = ep_queue,
	.dequeue = ep_dequeue,

	.set_halt = ep_halt,

	.set_wedge = ep_wedge,

	.fifo_status = 0,
	.fifo_flush = 0,

};

#endif /* _EN_ISOC_ */
/*	Gadget Operations */
/**
 * The following gadget operations will be implemented in the DWC_otg
 * PCD. Functions in the API that are not described below are not
 * implemented.
 *
 * The Gadget API provides wrapper functions for each of the function
 * pointers defined in usb_gadget_ops. The Gadget Driver calls the
 * wrapper function, which then calls the underlying PCD function. The
 * following sections are named according to the wrapper functions
 * (except for ioctl, which doesn't have a wrapper function). Within
 * each section, the corresponding DWC_otg PCD function name is
 * specified.
 *
 */

/**
 *Gets the USB Frame number of the last SOF.
 */
static int get_frame_number(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	DWC_DEBUGPL(DBG_PCDV, "%s(%p)\n", __func__, gadget);

	if (gadget == 0) {
		return -ENODEV;
	}

	d = container_of(gadget, struct gadget_wrapper_t, gadget);
	return dwc_otg_pcd_get_frame_number(d->pcd);
}

#ifdef CONFIG_USB_DWC_OTG_LPM
static int test_lpm_enabled(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	d = container_of(gadget, struct gadget_wrapper_t, gadget);

	return dwc_otg_pcd_is_lpm_enabled(d->pcd);
}
static int test_besl_enabled(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	d = container_of(gadget, struct gadget_wrapper_t, gadget);

	return dwc_otg_pcd_is_besl_enabled(d->pcd);
}
static int get_param_baseline_besl(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	d = container_of(gadget, struct gadget_wrapper_t, gadget);

	return dwc_otg_pcd_get_param_baseline_besl(d->pcd);
}
static int get_param_deep_besl(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	d = container_of(gadget, struct gadget_wrapper_t, gadget);

	return dwc_otg_pcd_get_param_deep_besl(d->pcd);
}
#endif

/**
 * Initiates Session Request Protocol (SRP) to wakeup the host if no
 * session is in progress. If a session is already in progress, but
 * the device is suspended, remote wakeup signaling is started.
 *
 */
static int wakeup(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *d;

	DWC_DEBUGPL(DBG_PCD, "%s(%p)\n", __func__, gadget);

	if (gadget == 0) {
		return -ENODEV;
	} else {
		d = container_of(gadget, struct gadget_wrapper_t, gadget);
	}
	dwc_otg_pcd_wakeup(d->pcd);
	return 0;
}

static const struct usb_gadget_ops dwc_otg_pcd_ops = {
	.get_frame = get_frame_number,
	.wakeup = wakeup,
#ifdef CONFIG_USB_DWC_OTG_LPM
	.lpm_support = test_lpm_enabled,
	.besl_support = test_besl_enabled,
	.get_baseline_besl = get_param_baseline_besl,
	.get_deep_besl = get_param_deep_besl,
#endif
	// current versions must always be self-powered
};

static int _setup(dwc_otg_pcd_t * pcd, uint8_t * bytes)
{
	struct usb_gadget_driver *driver = pcd->gadget_wrapper->driver;
	int retval = -DWC_E_NOT_SUPPORTED;

	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	if (driver && driver->setup) {
		retval = driver->setup(&pcd->gadget_wrapper->gadget,
				       (struct usb_ctrlrequest *)bytes);
	}

	if (retval == -ENOTSUPP) {
		retval = -DWC_E_NOT_SUPPORTED;
	} else if (retval < 0) {
		retval = -DWC_E_INVALID;
	}

	return retval;
}

#ifdef DWC_EN_ISOC
static int _isoc_complete(dwc_otg_pcd_t * pcd, void *ep_handle,
			  void *req_handle, int proc_buf_num)
{
	int i, packet_count;
	struct usb_gadget_iso_packet_descriptor *iso_packet = 0;
	struct usb_iso_request *iso_req = req_handle;

	if (proc_buf_num) {
		iso_packet = iso_req->iso_packet_desc1;
	} else {
		iso_packet = iso_req->iso_packet_desc0;
	}
	packet_count =
	    dwc_otg_pcd_get_iso_packet_count(pcd, ep_handle, req_handle);
	for (i = 0; i < packet_count; ++i) {
		int status;
		int actual;
		int offset;
		dwc_otg_pcd_get_iso_packet_params(pcd, ep_handle, req_handle,
						  i, &status, &actual, &offset);
		switch (status) {
		case -DWC_E_NO_DATA:
			status = -ENODATA;
			break;
		default:
			if (status) {
				DWC_PRINTF("unknown status in isoc packet\n");
			}

		}
		iso_packet[i].status = status;
		iso_packet[i].offset = offset;
		iso_packet[i].actual_length = actual;
	}

	iso_req->status = 0;
	iso_req->process_buffer(ep_handle, iso_req);

	return 0;
}
#endif /* DWC_EN_ISOC */

#ifdef DWC_UTE_PER_IO
/**
 * Copy the contents of the extended request to the Linux usb_request's
 * extended part and call the gadget's completion.
 *
 * @param pcd			Pointer to the pcd structure
 * @param ep_handle		Void pointer to the usb_ep structure
 * @param req_handle	Void pointer to the usb_request structure
 * @param status		Request status returned from the portable logic
 * @param ereq_port		Void pointer to the extended request structure
 *						created in the the portable part that contains the
 *						results of the processed iso packets.
 */
static int _xisoc_complete(dwc_otg_pcd_t * pcd, void *ep_handle,
			   void *req_handle, int32_t status, void *ereq_port)
{
	struct dwc_ute_iso_req_ext *ereqorg = NULL;
	struct dwc_iso_xreq_port *ereqport = NULL;
	struct dwc_ute_iso_packet_descriptor *desc_org = NULL;
	int i;
	struct usb_request *req;
	//struct dwc_ute_iso_packet_descriptor *
	//int status = 0;

	req = (struct usb_request *)req_handle;
	ereqorg = &req->ext_req;
	ereqport = (struct dwc_iso_xreq_port *)ereq_port;
	desc_org = ereqorg->per_io_frame_descs;

	if (req && req->complete) {
		/* Copy the request data from the portable logic to our request */
		for (i = 0; i < ereqport->pio_pkt_count; i++) {
			desc_org[i].actual_length =
			    ereqport->per_io_frame_descs[i].actual_length;
			desc_org[i].status =
			    ereqport->per_io_frame_descs[i].status;
		}

		switch (status) {
		case -DWC_E_SHUTDOWN:
			req->status = -ESHUTDOWN;
			break;
		case -DWC_E_RESTART:
			req->status = -ECONNRESET;
			break;
		case -DWC_E_INVALID:
			req->status = -EINVAL;
			break;
		case -DWC_E_TIMEOUT:
			req->status = -ETIMEDOUT;
			break;
		default:
			req->status = status;
		}

		/* And call the gadget's completion */
		req->complete(ep_handle, req);
	}

	return 0;
}
#endif /* DWC_UTE_PER_IO */

static int _complete(dwc_otg_pcd_t *pcd, void *ep_handle,
		     void *req_handle, int32_t status, uint32_t actual)
{
	struct usb_request *req = (struct usb_request *)req_handle;
	if (GET_CORE_IF(pcd)->dma_enable && req->length != 0) {
#ifdef PCI_INTERFACE
		struct pci_dev *dev =
			pcd->gadget_wrapper->pcd->otg_dev->os_dep.pcidev;
		struct dwc_otg_pcd_ep *ep = ep_from_handle(pcd, ep_handle);
		int is_in_ep = 0;

		if (ep)
			is_in_ep = ep->dwc_ep.is_in;

		pci_unmap_single(dev, req->dma, req->length, is_in_ep ?
				 PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		req->dma = DWC_DMA_ADDR_INVALID;
#endif

#ifdef PLATFORM_INTERFACE
	struct device *dev = NULL;
	dwc_otg_device_t *otg_dev = pcd->gadget_wrapper->pcd->otg_dev;
	struct dwc_otg_pcd_ep *ep =  ep_from_handle(pcd, ep_handle);

	if (req->dma && req->dma != DWC_DMA_ADDR_INVALID) {
		WARN_ON(!otg_dev);

		if (otg_dev) {
			dev = DWC_OTG_OS_GETDEV(otg_dev->os_dep);

			dma_unmap_single(dev, req->dma, req->length,
					 ep->dwc_ep.is_in ?
					 DMA_TO_DEVICE : DMA_FROM_DEVICE);
		}
	}
	req->dma = DWC_DMA_ADDR_INVALID;
#endif

	};

	if (req && req->complete) {
		switch (status) {
		case -DWC_E_SHUTDOWN:
			req->status = -ESHUTDOWN;
			break;
		case -DWC_E_RESTART:
			req->status = -ECONNRESET;
			break;
		case -DWC_E_INVALID:
			req->status = -EINVAL;
			break;
		case -DWC_E_TIMEOUT:
			req->status = -ETIMEDOUT;
			break;
		default:
			req->status = status;

		}

		req->actual = actual;
		DWC_SPINUNLOCK(pcd->core_if->lock);
		req->complete(ep_handle, req);
		DWC_SPINLOCK(pcd->core_if->lock);
	}

	return 0;
}

static int _connect(dwc_otg_pcd_t * pcd, int speed)
{
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	pcd->gadget_wrapper->gadget.speed = speed;
	return 0;
}

static int _disconnect(dwc_otg_pcd_t * pcd)
{
	struct usb_gadget_driver *driver = pcd->gadget_wrapper->driver;
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);


	if (driver && driver->disconnect)
		driver->disconnect(&pcd->gadget_wrapper->gadget);
	return 0;
}

static int _resume(dwc_otg_pcd_t * pcd)
{
	struct usb_gadget_driver *driver = pcd->gadget_wrapper->driver;
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	if (driver && driver->resume)
		driver->resume(&pcd->gadget_wrapper->gadget);
	return 0;
}

static int _suspend(dwc_otg_pcd_t * pcd)
{
	struct usb_gadget_driver *driver = pcd->gadget_wrapper->driver;
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	if (driver && driver->suspend)
		driver->suspend(&pcd->gadget_wrapper->gadget);
	return 0;
}

/**
 * This function updates the otg values in the gadget structure.
 */
static int _hnp_changed(dwc_otg_pcd_t * pcd)
{

	if (!pcd->gadget_wrapper->gadget.is_otg)
		return 0;

	pcd->gadget_wrapper->gadget.b_hnp_enable = get_b_hnp_enable(pcd);
	pcd->gadget_wrapper->gadget.a_hnp_support = get_a_hnp_support(pcd);
	pcd->gadget_wrapper->gadget.a_alt_hnp_support =
						get_a_alt_hnp_support(pcd);
	return 0;
}

static int _reset(dwc_otg_pcd_t * pcd)
{
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);
	return 0;
}

#ifdef DWC_UTE_CFI
static int _cfi_setup(dwc_otg_pcd_t * pcd, void *cfi_req)
{
	int retval = -DWC_E_INVALID;
	if (pcd->gadget_wrapper->driver->cfi_feature_setup) {
		retval =
		    pcd->gadget_wrapper->driver->
		    cfi_feature_setup(&pcd->gadget_wrapper->gadget,
				      (struct cfi_usb_ctrlrequest *)cfi_req);
	}

	return retval;
}
#endif

static void dwc_otg_vbus_sensing(dwc_otg_pcd_t *pcd, bool state)
{
	if (!pcd->vbus_chan)
		return;

	if (state)
		mod_timer(&pcd->vbus_timer,
			  jiffies + msecs_to_jiffies(VBUS_MONITORING));
	else
		del_timer(&pcd->vbus_timer);
}

static const struct dwc_otg_pcd_function_ops fops = {
	.complete = _complete,
#ifdef DWC_EN_ISOC
	.isoc_complete = _isoc_complete,
#endif
	.setup = _setup,
	.disconnect = _disconnect,
	.connect = _connect,
	.resume = _resume,
	.suspend = _suspend,
	.hnp_changed = _hnp_changed,
	.reset = _reset,
#ifdef DWC_UTE_CFI
	.cfi_setup = _cfi_setup,
#endif
#ifdef DWC_UTE_PER_IO
	.xisoc_complete = _xisoc_complete,
#endif
	.vbus_sensing =  dwc_otg_vbus_sensing,
};

/**
 * This function is the top level PCD interrupt handler.
 */
static irqreturn_t dwc_otg_pcd_irq(int irq, void *dev)
{
	dwc_otg_pcd_t *pcd = dev;
	int32_t retval = IRQ_NONE;

	retval = dwc_otg_pcd_handle_intr(pcd);
	if (retval != 0) {
		S3C2410X_CLEAR_EINTPEND();
	}
	return IRQ_RETVAL(retval);
}

/**
 * This function initialized the usb_ep structures to there default
 * state.
 *
 * @param d Pointer on gadget_wrapper.
 */
void gadget_add_eps(struct gadget_wrapper_t *d)
{
	static const char *names[] = {

		"ep0",
		"ep1in",
		"ep2in",
		"ep3in",
		"ep4in",
		"ep5in",
		"ep6in",
		"ep7in",
		"ep8in",
		"ep9in",
		"ep10in",
		"ep11in",
		"ep12in",
		"ep13in",
		"ep14in",
		"ep15in",
		"ep1out",
		"ep2out",
		"ep3out",
		"ep4out",
		"ep5out",
		"ep6out",
		"ep7out",
		"ep8out",
		"ep9out",
		"ep10out",
		"ep11out",
		"ep12out",
		"ep13out",
		"ep14out",
		"ep15out"
	};

	int i;
	struct usb_ep *ep;
	int8_t dev_endpoints;

	DWC_DEBUGPL(DBG_PCD, "%s\n", __func__);

	INIT_LIST_HEAD(&d->gadget.ep_list);
	d->gadget.ep0 = &d->ep0.ep;
	d->gadget.speed = USB_SPEED_UNKNOWN;
	d->gadget.max_speed = USB_SPEED_HIGH;

	INIT_LIST_HEAD(&d->gadget.ep0->ep_list);

	/**
	 * Initialize the EP0 structure.
	 */
	ep = &d->ep0.ep;
	d->ep0.gadget_wrapper = d;

	/* Init the usb_ep structure. */
	ep->name = names[0];
	ep->ops = (struct usb_ep_ops *)&dwc_otg_pcd_ep_ops;
	ep->caps.type_control = true;

	/**
	 * @todo NGS: What should the max packet size be set to
	 * here?  Before EP type is set?
	 */
	ep->maxpacket = MAX_PACKET_SIZE;
	ep->maxpacket_limit = MAX_PACKET_SIZE;
	dwc_otg_pcd_ep_enable(d->pcd, NULL, ep);

	list_add_tail(&ep->ep_list, &d->gadget.ep_list);

	/**
	 * Initialize the EP structures.
	 */
	dev_endpoints = d->pcd->core_if->dev_if->num_in_eps;

	for (i = 0; i < dev_endpoints; i++) {
		ep = &d->in_ep[i].ep;
		d->in_ep[i].gadget_wrapper = d;

		/* Init the usb_ep structure. */
		ep->name = names[d->pcd->in_ep[i].dwc_ep.num];
		ep->ops = (struct usb_ep_ops *)&dwc_otg_pcd_ep_ops;
		ep->caps.type_iso = true;
		ep->caps.type_bulk = true;
		ep->caps.type_int = true;
		ep->caps.dir_in = true;
		ep->caps.dir_out = false;

		/**
		 * @todo NGS: What should the max packet size be set to
		 * here?  Before EP type is set?
		 */
		ep->maxpacket = MAX_PACKET_SIZE;
		ep->maxpacket_limit = MAX_PACKET_SIZE;
		list_add_tail(&ep->ep_list, &d->gadget.ep_list);
	}

	dev_endpoints = d->pcd->core_if->dev_if->num_out_eps;

	for (i = 0; i < dev_endpoints; i++) {
		ep = &d->out_ep[i].ep;
		d->out_ep[i].gadget_wrapper = d;

		/* Init the usb_ep structure. */
		ep->name = names[15 + d->pcd->out_ep[i].dwc_ep.num];
		ep->ops = (struct usb_ep_ops *)&dwc_otg_pcd_ep_ops;
		ep->caps.type_iso = true;
		ep->caps.type_bulk = true;
		ep->caps.type_int = true;
		ep->caps.dir_in = false;
		ep->caps.dir_out = true;

		/**
		 * @todo NGS: What should the max packet size be set to
		 * here?  Before EP type is set?
		 */
		ep->maxpacket = MAX_PACKET_SIZE;
		ep->maxpacket_limit = MAX_PACKET_SIZE;

		list_add_tail(&ep->ep_list, &d->gadget.ep_list);
	}

	/* remove ep0 from the list.  There is a ep0 pointer. */
	list_del_init(&d->ep0.ep.ep_list);

	d->ep0.ep.maxpacket = MAX_EP0_SIZE;
}

/**
 * This function releases the Gadget device.
 * required by device_unregister().
 *
 * @todo Should this do something?	Should it free the PCD?
 */
static void dwc_otg_pcd_gadget_release(struct device *dev)
{
	DWC_DEBUGPL(DBG_PCD, "%s(%p)\n", __func__, dev);
}

static struct gadget_wrapper_t *alloc_wrapper(
#ifdef LM_INTERFACE
	struct lm_device *_dev
#elif  defined(PCI_INTERFACE)
	struct pci_dev *_dev
#elif defined(PLATFORM_INTERFACE)
	dwc_bus_dev_t *_dev
#endif
    )
{
	static char pcd_name[] = "dwc_otg_pcd";
#ifdef LM_INTERFACE
	dwc_otg_device_t *otg_dev = lm_get_drvdata(_dev);
#elif defined(PCI_INTERFACE)
	dwc_otg_device_t *otg_dev = pci_get_drvdata(_dev);
#elif defined(PLATFORM_INTERFACE)
	dwc_otg_device_t *otg_dev = DWC_OTG_BUSDRVDATA(_dev);
#endif

	struct gadget_wrapper_t *d;

	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	d = DWC_ALLOC(sizeof(*d), GFP_KERNEL);
	if (d == NULL) {
		return NULL;
	}

	memset(d, 0, sizeof(*d));

	d->gadget.name = pcd_name;
	d->pcd = otg_dev->pcd;

	dev_set_name(&d->gadget.dev, "%s", "gadget");

	d->gadget.dev.parent = &_dev->dev;
	d->gadget.dev.release = dwc_otg_pcd_gadget_release;
	d->gadget.ops = &dwc_otg_pcd_ops;
	d->gadget.is_otg = dwc_otg_pcd_is_otg(otg_dev->pcd);

	d->driver = 0;

	return d;
}



/**
 * This function registers a gadget driver with the PCD.
 *
 * When a driver is successfully registered, it will receive control
 * requests including set_configuration(), which enables non-control
 * requests.  then usb traffic follows until a disconnect is reported.
 * then a host may connect again, or the driver might get unbound.
 *
 * @param driver The driver being registered
 * @param bind The bind function of gadget driver
 */
static int dwc_otg_start(struct usb_gadget *gadget,
			   struct usb_gadget_driver *driver)
{
	struct gadget_wrapper_t *gadget_wrapper =
			gadget_to_gadget_wrapper(gadget);

	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	DWC_DEBUGPL(DBG_PCD, "registering gadget driver '%s'\n",
		    driver->driver.name);

	if (!driver ||
	    driver->max_speed == USB_SPEED_UNKNOWN ||
	    !driver->bind ||
	    !driver->unbind || !driver->disconnect || !driver->setup) {
		DWC_DEBUGPL(DBG_PCD, "EINVAL\n");
		return -EINVAL;
	}

	if (!gadget_wrapper) {
		DWC_DEBUGPL(DBG_PCD, "ENODEV\n");
		return -ENODEV;
	}

	if (gadget_wrapper->driver != 0) {
		DWC_DEBUGPL(DBG_PCD, "EBUSY (%p)\n", gadget_wrapper->driver);
		return -EBUSY;
	}

	/* hook up the driver */
	gadget_wrapper->driver = driver;
	gadget_wrapper->gadget.dev.driver = &driver->driver;

	DWC_DEBUGPL(DBG_ANY, "registered gadget driver '%s'\n",
		    driver->driver.name);

	dwc_otg_pcd_disconnect_us(gadget_wrapper->pcd, 1000);

	/* Start VBUS monitoring if initialised*/
	dwc_otg_vbus_sensing(gadget_wrapper->pcd, 1);

	return 0;
}

/**
 * This function unregisters a gadget driver
 *
 * @param driver The driver being unregistered
 */
static int dwc_otg_stop(struct usb_gadget *gadget)
{
	struct gadget_wrapper_t *gadget_wrapper =
				gadget_to_gadget_wrapper(gadget);

	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	if (!gadget_wrapper) {
		DWC_DEBUGPL(DBG_ANY, "%s Return(%d): s_pcd==0\n", __func__,
			    -ENODEV);
		return -ENODEV;
	}

	/* Stop VBUS monitoring if initialised*/
	dwc_otg_vbus_sensing(gadget_wrapper->pcd, 0);

	DWC_DEBUGPL(DBG_ANY, "unregistered driver '%s'\n",
		    gadget_wrapper->driver->driver.name);

	dwc_otg_pcd_disconnect_us(gadget_wrapper->pcd, 0);

	gadget_wrapper->driver = 0;

	return 0;
}

static void dwc_otg_vbus_timer(unsigned long data)
{
	dwc_otg_pcd_t *pcd = (dwc_otg_pcd_t *)data;
	int raw;
	int ret;

	ret = iio_read_channel_raw(pcd->vbus_chan, &raw);
	if (ret < 0) {
		DWC_ERROR("%s read vbus_channel() error: %d\n", __func__, ret);
		return;
	}

	switch (pcd->ep0state) {
	case EP0_DISCONNECT:
		if (raw >= pcd->vbus_limit) {
			DWC_DEBUGPL(DBG_PCD,
				    "vbus detected, set D+ pull-up\n");
			dwc_otg_pcd_connect(pcd);
		}
		break;

	default:
		if (raw < pcd->vbus_limit) {
			DWC_DEBUGPL(DBG_PCD,
				    "vbus drop, SoftDisconnect\n");
			dwc_otg_pcd_disconnect_us(pcd, 0);
			_disconnect(pcd);
			pcd->ep0state = EP0_DISCONNECT;
		}
		break;
	}

	mod_timer(&pcd->vbus_timer,
		  jiffies + msecs_to_jiffies(VBUS_MONITORING));
}

static void free_wrapper(struct gadget_wrapper_t *d)
{
	DWC_DEBUGPL(DBG_PCD, "%s'\n", __func__);

	if (d->driver) {
		/* should have been done already by driver model core */
		usb_gadget_unregister_driver(d->driver);
	}

	DWC_FREE(d);
}

static const struct usb_gadget_ops dwc_otg_gadget_ops = {
	.udc_start		= dwc_otg_start,
	.udc_stop		= dwc_otg_stop,
};

/**
 * This function initialized the PCD portion of the driver.
 *
 */
int pcd_init(
#ifdef LM_INTERFACE
	struct lm_device *_dev
#elif  defined(PCI_INTERFACE)
	struct pci_dev *_dev
#elif defined(PLATFORM_INTERFACE)
	dwc_bus_dev_t *_dev
#endif
    )
{
#ifdef LM_INTERFACE
	dwc_otg_device_t *otg_dev = lm_get_drvdata(_dev);
#elif  defined(PCI_INTERFACE)
	dwc_otg_device_t *otg_dev = pci_get_drvdata(_dev);
#elif defined(PLATFORM_INTERFACE)
	dwc_otg_device_t *otg_dev = DWC_OTG_BUSDRVDATA(_dev);
#endif
	struct device_node *np = _dev->dev.of_node;
	int retval = 0;

	DWC_DEBUGPL(DBG_PCD, "%s(%p)\n", __func__, _dev);

	otg_dev->pcd = dwc_otg_pcd_init(otg_dev->core_if);

	if (!otg_dev->pcd) {
		DWC_ERROR("dwc_otg_pcd_init failed\n");
		return -ENOMEM;
	}

	otg_dev->pcd->otg_dev = otg_dev;
	otg_dev->pcd->gadget_wrapper = alloc_wrapper(_dev);

	/*
	 * Initialize EP structures
	 */
	gadget_add_eps(otg_dev->pcd->gadget_wrapper);
	/*
	 * Setup interupt handler
	 */
#ifdef PLATFORM_INTERFACE
	DWC_DEBUGPL(DBG_ANY, "registering handler for irq%d\n",
			platform_get_irq(_dev, 0));
	retval = request_irq(platform_get_irq(_dev, 0), dwc_otg_pcd_irq,
			     IRQF_SHARED,
			     otg_dev->pcd->gadget_wrapper->gadget.name,
			     otg_dev->pcd);
	if (retval != 0) {
		DWC_ERROR("request of irq%d failed\n",
				platform_get_irq(_dev, 0));
		free_wrapper(otg_dev->pcd->gadget_wrapper);
		return -EBUSY;
	}
#else
	DWC_DEBUGPL(DBG_ANY, "registering handler for irq%d\n", _dev->irq);
	retval = request_irq(_dev->irq, dwc_otg_pcd_irq,
			     IRQF_SHARED | IRQF_DISABLED,
			     otg_dev->pcd->gadget_wrapper->gadget.name,
			     otg_dev->pcd);
	if (retval != 0) {
		DWC_ERROR("request of irq%d failed\n", _dev->irq);
		free_wrapper(otg_dev->pcd->gadget_wrapper);
		return -EBUSY;
	}
#endif

	otg_dev->pcd->gadget_wrapper->gadget.ops = &dwc_otg_gadget_ops;
	retval = usb_add_gadget_udc(&_dev->dev,
				    &otg_dev->pcd->gadget_wrapper->gadget);
	if (retval)
		DWC_ERROR("%s: error while adding gadget to udc core\n",
				__func__);

	dwc_otg_pcd_start(otg_dev->pcd, &fops);

	/* Check from DT if vbus sensing for peripheral mode is requested */
	otg_dev->pcd->vbus_chan = NULL;
	otg_dev->pcd->vbus_limit = 0;
	if (of_property_read_bool(np, "st,vbus_sensing_gadget")) {
		int vbus_limit;
		struct iio_channel *vbus_chan;
		enum iio_chan_type type;

		vbus_chan = devm_iio_channel_get(&_dev->dev, NULL);
		if (IS_ERR(vbus_chan)) {
			dev_err(&_dev->dev, "vbus adc channel failure\n");
			if (PTR_ERR(vbus_chan) == -ENODEV)
				return -EPROBE_DEFER;
			return -EINVAL;
		}

		retval = iio_get_channel_type(vbus_chan, &type);
		if (retval < 0 || type != IIO_VOLTAGE) {
			dev_err(&_dev->dev, "vbus adc channel failure"
				"to get type or not voltage type\n");
			iio_channel_release_all(otg_dev->pcd->vbus_chan);
			return retval;
		}

		if (of_property_read_u32(np,
					 "st,vbus_raw_limit",
					 &vbus_limit) < 0) {
			dev_err(&_dev->dev,
				"failed to get property st,vbus_raw_limit\n");
			iio_channel_release_all(otg_dev->pcd->vbus_chan);
			return -EINVAL;
		}
		otg_dev->pcd->vbus_limit = vbus_limit;
		otg_dev->pcd->vbus_chan = vbus_chan;
		setup_timer(&otg_dev->pcd->vbus_timer,
			    dwc_otg_vbus_timer,
			    (unsigned long)otg_dev->pcd);
	}

	return retval;
}

/**
 * Cleanup the PCD.
 */
void pcd_remove(
#ifdef LM_INTERFACE
	struct lm_device *_dev
#elif  defined(PCI_INTERFACE)
	struct pci_dev *_dev
#elif defined(PLATFORM_INTERFACE)
	dwc_bus_dev_t *_dev
#endif
    )
{
#ifdef LM_INTERFACE
	dwc_otg_device_t *otg_dev = lm_get_drvdata(_dev);
#elif  defined(PCI_INTERFACE)
	dwc_otg_device_t *otg_dev = pci_get_drvdata(_dev);
#elif defined(PLATFORM_INTERFACE)
	dwc_otg_device_t *otg_dev = DWC_OTG_BUSDRVDATA(_dev);
#endif
	dwc_otg_pcd_t *pcd = otg_dev->pcd;

	DWC_DEBUGPL(DBG_PCD, "%s(%p)\n", __func__, _dev);

	/* Stop VBUS monitoring if initialised*/
	dwc_otg_vbus_sensing(pcd, 0);

	if (pcd->vbus_chan)
		iio_channel_release_all(pcd->vbus_chan);

	/*
	 * Free the IRQ
	 */
#ifdef PLATFORM_INTERFACE
	free_irq(platform_get_irq(_dev, 0), pcd);
#else
	free_irq(_dev->irq, pcd);
#endif

	usb_del_gadget_udc(&pcd->gadget_wrapper->gadget);

	free_wrapper(pcd->gadget_wrapper);
	dwc_otg_pcd_remove(otg_dev->pcd);

	otg_dev->pcd = 0;
}

#endif /* DWC_HOST_ONLY */
