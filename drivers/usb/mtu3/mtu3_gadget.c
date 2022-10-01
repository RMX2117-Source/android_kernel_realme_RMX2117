/*
 * mtu3_gadget.c - MediaTek usb3 DRD peripheral support
 *
 * Copyright (C) 2016 MediaTek Inc.
 *
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "mtu3.h"
#include "mtu3_dr.h"
#include <linux/usb/composite.h>
#ifdef CONFIG_USB_MTU3_PLAT_PHONE
#include <mt-plat/mtk_boot.h>
#endif

void mtu3_req_complete(struct mtu3_ep *mep,
		     struct usb_request *req, int status)
__releases(mep->mtu->lock)
__acquires(mep->mtu->lock)
{
	struct mtu3_request *mreq;
	struct mtu3 *mtu;
	int busy = mep->busy;

	mreq = to_mtu3_request(req);
	list_del(&mreq->list);
	if (mreq->request.status == -EINPROGRESS)
		mreq->request.status = status;

	mtu = mreq->mtu;
	mep->busy = 1;

	/* ep0 makes use of PIO, needn't unmap it */
	if (mep->epnum)
		usb_gadget_unmap_request(&mtu->g, req, mep->is_in);

	dev_dbg(mtu->dev, "%s complete req: %p, sts %d, %d/%d\n", mep->name,
		req, req->status, mreq->request.actual, mreq->request.length);

	spin_unlock(&mtu->lock);

	usb_gadget_giveback_request(&mep->ep, &mreq->request);

	spin_lock(&mtu->lock);
	mep->busy = busy;
}

static void nuke(struct mtu3_ep *mep, const int status)
{
	struct mtu3_request *mreq = NULL;

	mep->busy = 1;
	if (list_empty(&mep->req_list))
		return;

	dev_dbg(mep->mtu->dev, "abort %s's req: sts %d\n", mep->name, status);

	/* exclude EP0 */
	if (mep->epnum)
		mtu3_qmu_flush(mep);

	while (!list_empty(&mep->req_list)) {
		mreq = list_first_entry(&mep->req_list,
					struct mtu3_request, list);
		mtu3_req_complete(mep, &mreq->request, status);
	}
}

void mtu3_nuke_all_ep(struct mtu3 *mtu)
{
	int i;

	nuke(mtu->ep0, -ESHUTDOWN);
	for (i = 1; i < mtu->num_eps; i++) {
		nuke(mtu->in_eps + i, -ESHUTDOWN);
		nuke(mtu->out_eps + i, -ESHUTDOWN);
	}
}

static int is_db_ok(struct mtu3_ep *mep)
{
	struct mtu3 *mtu = mep->mtu;
	struct usb_composite_dev *cdev = (mtu->g).ep0->driver_data;
	struct usb_configuration *c = cdev->config;
	struct usb_gadget *gadget = &(mtu->g);
	int tmp;
	int ret = 1;

#ifdef OPLUS_FEATURE_CHG_BASIC
	for (tmp = 0; tmp < c->next_interface_id; tmp++) {
#else
	for (tmp = 0; tmp < MAX_CONFIG_INTERFACES; tmp++) {
#endif
		struct usb_function *f = c->interface[tmp];
		struct usb_descriptor_header **descriptors;

		if (!f)
			break;

		pr_info("Ifc name=%s\n", f->name);

		switch (gadget->speed) {
		case USB_SPEED_SUPER:
			descriptors = f->ss_descriptors;
			break;
		case USB_SPEED_HIGH:
			descriptors = f->hs_descriptors;
			break;
		default:
			descriptors = f->fs_descriptors;
		}

		for (; *descriptors; ++descriptors) {
			struct usb_endpoint_descriptor *ep;
			int is_in;
			int epnum;

			if ((*descriptors)->bDescriptorType != USB_DT_ENDPOINT)
				continue;

			ep = (struct usb_endpoint_descriptor *)*descriptors;

			is_in = (ep->bEndpointAddress & 0x80) >> 7;
			epnum = (ep->bEndpointAddress & 0x0f);

			/*
			 * Under saving mode, ALL EPs will be set
			 * as Single Buffer
			 */

			/* ep must be matched */
			if (ep->bEndpointAddress == (mep->ep).address) {

				if (gadget->speed == USB_SPEED_SUPER)
					ret = 0;
				goto end;
			}
		}
	}
end:
	return ret;
}

static int mtu3_ep_enable(struct mtu3_ep *mep)
{
	const struct usb_endpoint_descriptor *desc;
	const struct usb_ss_ep_comp_descriptor *comp_desc;
	struct mtu3 *mtu = mep->mtu;
	u32 interval = 0;
	u32 mult = 0;
	u32 burst = 0;
	int max_packet;
	int ret;

	desc = mep->desc;
	comp_desc = mep->comp_desc;
	mep->type = usb_endpoint_type(desc);
	max_packet = usb_endpoint_maxp(desc);
	mep->maxp = max_packet & GENMASK(10, 0);

	switch (mtu->g.speed) {
	case USB_SPEED_SUPER:
		if (usb_endpoint_xfer_int(desc) ||
				usb_endpoint_xfer_isoc(desc)) {
			interval = desc->bInterval;
			interval = clamp_val(interval, 1, 16) - 1;
			if (usb_endpoint_xfer_isoc(desc) && comp_desc)
				mult = comp_desc->bmAttributes;
		}
		if (comp_desc)
			burst = comp_desc->bMaxBurst;

		break;
	case USB_SPEED_HIGH:
		if (usb_endpoint_xfer_isoc(desc) ||
				usb_endpoint_xfer_int(desc)) {
			interval = desc->bInterval;
			interval = clamp_val(interval, 1, 16) - 1;
			burst = (max_packet & GENMASK(12, 11)) >> 11;
		}
		break;
	default:
		break; /*others are ignored */
	}

	dev_dbg(mtu->dev, "%s maxp:%d, interval:%d, burst:%d, mult:%d\n",
		__func__, mep->maxp, interval, burst, mult);

	mep->ep.maxpacket = mep->maxp;
	mep->ep.desc = desc;
	mep->ep.comp_desc = comp_desc;

	/* slot mainly affects bulk/isoc transfer, so ignore int */
	mep->slot = usb_endpoint_xfer_int(desc) ? 0 : mtu->slot;

	#ifdef CONFIG_USB_MTU3_PLAT_PHONE
	if (is_saving_mode()) {
		if (is_db_ok(mep)) {
			dev_info(mtu->dev, "Saving mode, but EP%d supports DBBUF\n",
				mep->epnum);
		} else {
			dev_info(mtu->dev, "EP%d supports single buffer\n",
				mep->epnum);
			mep->slot = 0;
		}
	}
	#endif

	ret = mtu3_config_ep(mtu, mep, interval, burst, mult);
	if (ret < 0)
		return ret;

	ret = mtu3_gpd_ring_alloc(mep);
	if (ret < 0) {
		mtu3_deconfig_ep(mtu, mep);
		return ret;
	}

	mtu3_qmu_start(mep);

	return 0;
}

static int mtu3_ep_disable(struct mtu3_ep *mep)
{
	struct mtu3 *mtu = mep->mtu;

	mtu3_qmu_stop(mep);

	/* abort all pending requests */
	nuke(mep, -ESHUTDOWN);
	mtu3_deconfig_ep(mtu, mep);
	mtu3_gpd_ring_free(mep);

	mep->desc = NULL;
	mep->ep.desc = NULL;
	mep->comp_desc = NULL;
	mep->type = 0;
	mep->flags = 0;

	return 0;
}

static int mtu3_gadget_ep_enable(struct usb_ep *ep,
		const struct usb_endpoint_descriptor *desc)
{
	struct mtu3_ep *mep;
	struct mtu3 *mtu;
	unsigned long flags;
	int ret = -EINVAL;

	if (!ep || !desc || desc->bDescriptorType != USB_DT_ENDPOINT) {
		pr_debug("%s invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (!desc->wMaxPacketSize) {
		pr_debug("%s missing wMaxPacketSize\n", __func__);
		return -EINVAL;
	}
	mep = to_mtu3_ep(ep);
	mtu = mep->mtu;

	/* check ep number and direction against endpoint */
	if (usb_endpoint_num(desc) != mep->epnum)
		return -EINVAL;

	if (!!usb_endpoint_dir_in(desc) ^ !!mep->is_in)
		return -EINVAL;

	dev_dbg(mtu->dev, "%s %s\n", __func__, ep->name);

	if (mep->flags & MTU3_EP_ENABLED) {
		dev_WARN_ONCE(mtu->dev, true, "%s is already enabled\n",
				mep->name);
		return 0;
	}

	spin_lock_irqsave(&mtu->lock, flags);
	mep->desc = desc;
	mep->comp_desc = ep->comp_desc;

	ret = mtu3_ep_enable(mep);
	if (ret)
		goto error;

	mep->busy = 0;
	mep->wedged = 0;
	mep->flags |= MTU3_EP_ENABLED;
	mtu->active_ep++;

error:
	spin_unlock_irqrestore(&mtu->lock, flags);

	dev_dbg(mtu->dev, "%s active_ep=%d\n", __func__, mtu->active_ep);

	return ret;
}

static int mtu3_gadget_ep_disable(struct usb_ep *ep)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);
	struct mtu3 *mtu = mep->mtu;
	unsigned long flags;

	dev_dbg(mtu->dev, "%s %s\n", __func__, mep->name);

	if (!(mep->flags & MTU3_EP_ENABLED)) {
		dev_warn(mtu->dev, "%s is already disabled\n", mep->name);
		return 0;
	}

	spin_lock_irqsave(&mtu->lock, flags);
	mtu3_ep_disable(mep);
	mep->flags &= ~MTU3_EP_ENABLED;
	mtu->active_ep--;
	spin_unlock_irqrestore(&(mtu->lock), flags);

	dev_dbg(mtu->dev, "%s active_ep=%d, mtu3 is_active=%d\n",
		__func__, mtu->active_ep, mtu->is_active);

	return 0;
}

struct usb_request *mtu3_alloc_request(struct usb_ep *ep, gfp_t gfp_flags)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);
	struct mtu3_request *mreq;

	mreq = kzalloc(sizeof(*mreq), gfp_flags);
	if (!mreq)
		return NULL;

	mreq->request.dma = DMA_ADDR_INVALID;
	mreq->epnum = mep->epnum;
	mreq->mep = mep;

	return &mreq->request;
}

void mtu3_free_request(struct usb_ep *ep, struct usb_request *req)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);
	struct mtu3_request *mreq = to_mtu3_request(req);
        struct mtu3_request *r;
        struct mtu3 *mtu = mep->mtu;
	unsigned long flags;

	spin_lock_irqsave(&mtu->lock, flags);
	list_for_each_entry(r, &mep->req_list, list) {
                if (r == mreq) {
                        list_del(&mreq->list);
                break;
                }
        }
        kfree(mreq);
        spin_unlock_irqrestore(&mtu->lock, flags);
}

static int mtu3_gadget_queue(struct usb_ep *ep,
		struct usb_request *req, gfp_t gfp_flags)
{
	struct mtu3_ep *mep;
	struct mtu3_request *mreq;
	struct mtu3 *mtu;
	unsigned long flags;
	int ret = 0;

	if (!ep || !req)
		return -EINVAL;

	if (!req->buf)
		return -ENODATA;

	mep = to_mtu3_ep(ep);
	mtu = mep->mtu;
	mreq = to_mtu3_request(req);
	mreq->mtu = mtu;

	if (mreq->mep != mep)
		return -EINVAL;

	dev_dbg(mtu->dev, "%s %s EP%d(%s), req=%p, maxp=%d, len#%d\n",
		__func__, mep->is_in ? "TX" : "RX", mreq->epnum, ep->name,
		mreq, ep->maxpacket, mreq->request.length);

	if (req->length > GPD_BUF_SIZE) {
		dev_warn(mtu->dev,
			"req length > supported MAX:%d requested:%d\n",
			GPD_BUF_SIZE, req->length);
		return -EOPNOTSUPP;
	}

	/* don't queue if the ep is down */
	if (!mep->desc) {
		dev_dbg(mtu->dev, "req=%p queued to %s while it's disabled\n",
			req, ep->name);
		return -ESHUTDOWN;
	}

	mreq->request.actual = 0;
	mreq->request.status = -EINPROGRESS;

	ret = usb_gadget_map_request(&mtu->g, req, mep->is_in);
	if (ret) {
		dev_err(mtu->dev, "dma mapping failed\n");
		return ret;
	}

	spin_lock_irqsave(&mtu->lock, flags);

	if (mtu3_prepare_transfer(mep)) {
		ret = -EAGAIN;
		goto error;
	}

	list_add_tail(&mreq->list, &mep->req_list);
	mtu3_insert_gpd(mep, mreq);
	mtu3_qmu_resume(mep);

error:
	spin_unlock_irqrestore(&mtu->lock, flags);

	return ret;
}

static int mtu3_gadget_dequeue(struct usb_ep *ep, struct usb_request *req)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);
	struct mtu3_request *mreq = to_mtu3_request(req);
	struct mtu3_request *r;
	unsigned long flags;
	int ret = 0;
	struct mtu3 *mtu = mep->mtu;

	if (!ep || !req || mreq->mep != mep)
		return -EINVAL;

	dev_dbg(mtu->dev, "%s : req=%p\n", __func__, req);

	spin_lock_irqsave(&mtu->lock, flags);

	list_for_each_entry(r, &mep->req_list, list) {
		if (r == mreq)
			break;
	}
	if (r != mreq) {
		dev_dbg(mtu->dev, "req=%p not queued to %s\n", req, ep->name);
		ret = -EINVAL;
		goto done;
	}

	mtu3_qmu_flush(mep);  /* REVISIT: set BPS ?? */
	mtu3_req_complete(mep, req, -ECONNRESET);
	mtu3_qmu_start(mep);

done:
	spin_unlock_irqrestore(&mtu->lock, flags);

	return ret;
}

/*
 * Set or clear the halt bit of an EP.
 * A halted EP won't TX/RX any data but will queue requests.
 */
static int mtu3_gadget_ep_set_halt(struct usb_ep *ep, int value)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);
	struct mtu3 *mtu = mep->mtu;
	struct mtu3_request *mreq;
	unsigned long flags;
	int ret = 0;

	if (!ep)
		return -EINVAL;

	dev_dbg(mtu->dev, "%s : %s...", __func__, ep->name);

	spin_lock_irqsave(&mtu->lock, flags);

	if (mep->type == USB_ENDPOINT_XFER_ISOC) {
		ret = -EINVAL;
		goto done;
	}

	mreq = next_request(mep);
	if (value) {
		/*
		 * If there is not request for TX-EP, QMU will not transfer
		 * data to TX-FIFO, so no need check whether TX-FIFO
		 * holds bytes or not here
		 */
		if (mreq) {
			dev_dbg(mtu->dev, "req in progress, cannot halt %s\n",
				ep->name);
			ret = -EAGAIN;
			goto done;
		}
	} else {
		mep->wedged = 0;
	}

	dev_dbg(mtu->dev, "%s %s stall\n", ep->name, value ? "set" : "clear");

	mtu3_ep_stall_set(mep, value);

done:
	spin_unlock_irqrestore(&mtu->lock, flags);

	return ret;
}

/* Sets the halt feature with the clear requests ignored */
static int mtu3_gadget_ep_set_wedge(struct usb_ep *ep)
{
	struct mtu3_ep *mep = to_mtu3_ep(ep);

	if (!ep)
		return -EINVAL;

	mep->wedged = 1;

	return usb_ep_set_halt(ep);
}

static const struct usb_ep_ops mtu3_ep_ops = {
	.enable = mtu3_gadget_ep_enable,
	.disable = mtu3_gadget_ep_disable,
	.alloc_request = mtu3_alloc_request,
	.free_request = mtu3_free_request,
	.queue = mtu3_gadget_queue,
	.dequeue = mtu3_gadget_dequeue,
	.set_halt = mtu3_gadget_ep_set_halt,
	.set_wedge = mtu3_gadget_ep_set_wedge,
};

static int mtu3_gadget_get_frame(struct usb_gadget *gadget)
{
	struct mtu3 *mtu = gadget_to_mtu3(gadget);

	return (int)mtu3_readl(mtu->mac_base, U3D_USB20_FRAME_NUM);
}

static int mtu3_gadget_wakeup(struct usb_gadget *gadget)
{
	struct mtu3 *mtu = gadget_to_mtu3(gadget);
	unsigned long flags;

	dev_dbg(mtu->dev, "%s\n", __func__);

	/* remote wakeup feature is not enabled by host */
	if (!mtu->may_wakeup)
		return  -EOPNOTSUPP;

	spin_lock_irqsave(&mtu->lock, flags);
	if (mtu->g.speed == USB_SPEED_SUPER) {
		mtu3_setbits(mtu->mac_base, U3D_LINK_POWER_CONTROL, UX_EXIT);
	} else {
		mtu3_setbits(mtu->mac_base, U3D_POWER_MANAGEMENT, RESUME);
		spin_unlock_irqrestore(&mtu->lock, flags);
		usleep_range(10000, 11000);
		spin_lock_irqsave(&mtu->lock, flags);
		mtu3_clrbits(mtu->mac_base, U3D_POWER_MANAGEMENT, RESUME);
	}
	spin_unlock_irqrestore(&mtu->lock, flags);
	return 0;
}

static int mtu3_gadget_set_self_powered(struct usb_gadget *gadget,
		int is_selfpowered)
{
	struct mtu3 *mtu = gadget_to_mtu3(gadget);

	mtu->is_self_powered = !!is_selfpowered;
	return 0;
}

static int usb_rdy;		/* default value 0 */

void set_usb_rdy(void)
{
	pr_info("set usb_rdy, wake up bat\n");
	usb_rdy = 1;
}

bool is_usb_rdy(void)
{
	if (usb_rdy)
		return true;
	else
		return false;
}

static int mtu3_gadget_pullup(struct usb_gadget *gadget, int is_on)
{
	struct mtu3 *mtu = gadget_to_mtu3(gadget);
	unsigned long flags;

	dev_info(mtu->dev, "%s (%s) for %sactive device\n", __func__,
		is_on ? "on" : "off", mtu->is_active ? "" : "in");

	/* we'd rather not pullup unless the device is active. */
	spin_lock_irqsave(&mtu->lock, flags);

	is_on = !!is_on;
	if (!mtu->is_active) {
		/* save it for mtu3_start() to process the request */
		mtu->softconnect = is_on;
	} else if (is_on != mtu->softconnect) {
		mtu->softconnect = is_on;
		mtu3_dev_on_off(mtu, is_on);

		if (!is_on)
			mtu3_nuke_all_ep(mtu);
	}

	if (is_usb_rdy() == false && is_on)
		set_usb_rdy();

	spin_unlock_irqrestore(&mtu->lock, flags);
	#ifdef CONFIG_USB_MTU3_PLAT_PHONE
	/* Trigger connection when force on*/
	if ((mtu3_cable_mode == CABLE_MODE_FORCEON) ||
		(get_boot_mode() == META_BOOT) ||
		(get_boot_mode() == ADVMETA_BOOT)) {
		dev_info(mtu->dev, "%s CABLE_MODE_FORCEON or META_MODE\n",
			__func__);
		ssusb_set_mailbox(&mtu->ssusb->otg_switch,
			MTU3_VBUS_VALID);
	}
	#endif

	return 0;
}

static int mtu3_gadget_start(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver)
{
	struct mtu3 *mtu = gadget_to_mtu3(gadget);
	unsigned long flags;

	if (mtu->gadget_driver) {
		dev_err(mtu->dev, "%s is already bound to %s\n",
			mtu->g.name, mtu->gadget_driver->driver.name);
		return -EBUSY;
	}

	dev_dbg(mtu->dev, "bind driver %s\n", driver->function);

	spin_lock_irqsave(&mtu->lock, flags);

	mtu->softconnect = 0;
	mtu->gadget_driver = driver;

	if (mtu->ssusb->dr_mode == USB_DR_MODE_PERIPHERAL)
		mtu3_start(mtu);

	spin_unlock_irqrestore(&mtu->lock, flags);

	return 0;
}

static void stop_activity(struct mtu3 *mtu)
{
	struct usb_gadget_driver *driver = mtu->gadget_driver;
	int i;

	dev_info(mtu->dev, "%s\n", __func__);

	/* don't disconnect if it's not connected */
	if (mtu->g.speed == USB_SPEED_UNKNOWN)
		driver = NULL;
	else
		mtu->g.speed = USB_SPEED_UNKNOWN;

	/* deactivate the hardware */
	if (mtu->softconnect) {
		mtu->softconnect = 0;
		mtu3_dev_on_off(mtu, 0);
	}

	/*
	 * killing any outstanding requests will quiesce the driver;
	 * then report disconnect
	 */
	mtu3_nuke_all_ep(mtu);
	if (driver) {
		spin_unlock(&mtu->lock);
		driver->disconnect(&mtu->g);
		spin_lock(&mtu->lock);
	}
}

static int mtu3_gadget_stop(struct usb_gadget *g)
{
	struct mtu3 *mtu = gadget_to_mtu3(g);
	unsigned long flags;

	dev_dbg(mtu->dev, "%s\n", __func__);

	spin_lock_irqsave(&mtu->lock, flags);

	stop_activity(mtu);
	mtu->gadget_driver = NULL;

	if (mtu->ssusb->dr_mode == USB_DR_MODE_PERIPHERAL)
		mtu3_stop(mtu);

	spin_unlock_irqrestore(&mtu->lock, flags);

	return 0;
}

static const struct usb_gadget_ops mtu3_gadget_ops = {
	.get_frame = mtu3_gadget_get_frame,
	.wakeup = mtu3_gadget_wakeup,
	.set_selfpowered = mtu3_gadget_set_self_powered,
	.pullup = mtu3_gadget_pullup,
	.udc_start = mtu3_gadget_start,
	.udc_stop = mtu3_gadget_stop,
};

static void mtu3_state_reset(struct mtu3 *mtu)
{
	struct mtu3_ep *mep;

	mtu->address = 0;
	mtu->ep0_state = MU3D_EP0_STATE_SETUP;
	mtu->may_wakeup = 0;

	mep = mtu->ep0;
	if (!list_empty(&mep->req_list)) {
		pr_info("%s reinit EP[0] req_list\n", __func__);
		INIT_LIST_HEAD(&mep->req_list);
	}
}

static void init_hw_ep(struct mtu3 *mtu, struct mtu3_ep *mep,
		u32 epnum, u32 is_in)
{
	mep->epnum = epnum;
	mep->mtu = mtu;
	mep->is_in = is_in;

	INIT_LIST_HEAD(&mep->req_list);

	sprintf(mep->name, "ep%d%s", epnum,
		!epnum ? "" : (is_in ? "in" : "out"));

	mep->ep.name = mep->name;
	INIT_LIST_HEAD(&mep->ep.ep_list);

	/* initialize maxpacket as SS */
	if (!epnum) {
		usb_ep_set_maxpacket_limit(&mep->ep, 512);
		mep->ep.caps.type_control = true;
		mep->ep.ops = &mtu3_ep0_ops;
		mtu->g.ep0 = &mep->ep;
	} else {
		usb_ep_set_maxpacket_limit(&mep->ep, 1024);
		mep->ep.caps.type_iso = true;
		mep->ep.caps.type_bulk = true;
		mep->ep.caps.type_int = true;
		mep->ep.ops = &mtu3_ep_ops;
		list_add_tail(&mep->ep.ep_list, &mtu->g.ep_list);
	}

	dev_dbg(mtu->dev, "%s, name=%s, maxp=%d\n", __func__, mep->ep.name,
		 mep->ep.maxpacket);

	if (!epnum) {
		mep->ep.caps.dir_in = true;
		mep->ep.caps.dir_out = true;
	} else if (is_in) {
		mep->ep.caps.dir_in = true;
	} else {
		mep->ep.caps.dir_out = true;
	}
}

static void mtu3_gadget_init_eps(struct mtu3 *mtu)
{
	u8 epnum;

	/* initialize endpoint list just once */
	INIT_LIST_HEAD(&(mtu->g.ep_list));

	dev_dbg(mtu->dev, "%s num_eps(1 for a pair of tx&rx ep)=%d\n",
		__func__, mtu->num_eps);

	init_hw_ep(mtu, mtu->ep0, 0, 0);
	for (epnum = 1; epnum < mtu->num_eps; epnum++) {
		init_hw_ep(mtu, mtu->in_eps + epnum, epnum, 1);
		init_hw_ep(mtu, mtu->out_eps + epnum, epnum, 0);
	}
}

int mtu3_gadget_setup(struct mtu3 *mtu)
{
	int ret;

	mtu->g.ops = &mtu3_gadget_ops;
	mtu->g.max_speed = mtu->max_speed;
	mtu->g.speed = USB_SPEED_UNKNOWN;
	mtu->g.sg_supported = 0;
	mtu->g.name = MTU3_DRIVER_NAME;
	mtu->is_active = 0;

	mtu3_gadget_init_eps(mtu);

	mtu->g.quirk_ep_out_aligned_size = true;

	ret = usb_add_gadget_udc(mtu->dev, &mtu->g);
	if (ret) {
		dev_err(mtu->dev, "failed to register udc\n");
		return ret;
	}

	usb_gadget_set_state(&mtu->g, USB_STATE_NOTATTACHED);

	return 0;
}

void mtu3_gadget_cleanup(struct mtu3 *mtu)
{
	usb_del_gadget_udc(&mtu->g);
}

void mtu3_gadget_resume(struct mtu3 *mtu)
{
	dev_dbg(mtu->dev, "gadget RESUME\n");
	if (mtu->gadget_driver && mtu->gadget_driver->resume) {
		spin_unlock(&mtu->lock);
		mtu->gadget_driver->resume(&mtu->g);
		spin_lock(&mtu->lock);
	}
}

/* called when SOF packets stop for 3+ msec or enters U3 */
void mtu3_gadget_suspend(struct mtu3 *mtu)
{
	dev_info(mtu->dev, "gadget SUSPEND\n");
	if (mtu->gadget_driver && mtu->gadget_driver->suspend) {
		spin_unlock(&mtu->lock);
		mtu->gadget_driver->suspend(&mtu->g);
		spin_lock(&mtu->lock);
	}
}

/* called when VBUS drops below session threshold, and in other cases */
void mtu3_gadget_disconnect(struct mtu3 *mtu)
{
	struct usb_gadget_driver *driver;

	dev_info(mtu->dev, "gadget DISCONNECT\n");
	if (mtu->gadget_driver && mtu->gadget_driver->disconnect) {
		driver = mtu->gadget_driver;
		spin_unlock(&mtu->lock);
		/*
		 * avoid kernel panic because mtu3_gadget_stop() assigned NULL
		 * to mtu->gadget_driver.
		 */
		driver->disconnect(&mtu->g);
		spin_lock(&mtu->lock);
	}

	mtu3_state_reset(mtu);
	usb_gadget_set_state(&mtu->g, USB_STATE_NOTATTACHED);
}

void mtu3_gadget_reset(struct mtu3 *mtu)
{
	dev_info(mtu->dev, "gadget RESET\n");

	/* report disconnect, if we didn't flush EP state */
	if (mtu->g.speed != USB_SPEED_UNKNOWN)
		mtu3_gadget_disconnect(mtu);
	else
		mtu3_state_reset(mtu);
}
