/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ax206-priv.h - AX206 MFD parent private API
 *
 * Not to be included by child drivers.
 *
 * Copyright (C) 2025 Oskari Rauta <oskari.rauta@gmail.com>
 */

#ifndef _AX206_PRIV_H
#define _AX206_PRIV_H

#include "ax206.h"

/*
 * ax206_usb_init() - discover bulk endpoints from the interface descriptor
 *
 * Must be called during probe before any USB transfer.
 * Returns 0 on success, -ENODEV if endpoints are missing.
 */
int ax206_usb_init(struct ax206_dev *dpf);

/*
 * ax206_usb_query_dims() - read physical LCD dimensions from device
 *
 * Called only during probe, with usb_mutex held.
 * Returns 0 on success, negative errno on failure.
 */
int ax206_usb_query_dims(struct ax206_dev *dpf);

#endif /* _AX206_PRIV_H */
