// SPDX-License-Identifier: GPL-2.0
/*
 * mpro_screen.c — screen state notification mechanism.
 *
 * Children (DRM, backlight, touch) can register callbacks to be
 * notified when DRM's display pipe enters or leaves the active state.
 *
 * Use cases:
 *   - turn the backlight off when the DRM pipe is blanked
 *   - stop the touch URB pipeline while the screen is not visible
 *
 * DRM calls mpro_screen_notify_{off,on}() when the pipe state changes.
 */

#include <linux/module.h>
#include <linux/list.h>
#include <linux/mutex.h>

#include "mpro.h"

int mpro_screen_listener_register(struct mpro_device *mpro,
				  struct mpro_screen_listener *l)
{
	mutex_lock(&mpro->listeners_lock);
	list_add_tail(&l->node, &mpro->screen_listeners);
	mutex_unlock(&mpro->listeners_lock);
	return 0;
}
EXPORT_SYMBOL_GPL(mpro_screen_listener_register);

void mpro_screen_listener_unregister(struct mpro_device *mpro,
				     struct mpro_screen_listener *l)
{
	if (!mpro || !l)
		return;

	mutex_lock(&mpro->listeners_lock);
	list_del(&l->node);
	mutex_unlock(&mpro->listeners_lock);
}
EXPORT_SYMBOL_GPL(mpro_screen_listener_unregister);

void mpro_screen_notify_off(struct mpro_device *mpro)
{
	struct mpro_screen_listener *l;

	if (!mpro)
		return;

	dev_dbg(&mpro->intf->dev, "screen_notify_off\n");

	mutex_lock(&mpro->listeners_lock);
	list_for_each_entry(l, &mpro->screen_listeners, node)
		if (l->screen_off)
			l->screen_off(l->priv);
	mutex_unlock(&mpro->listeners_lock);
}
EXPORT_SYMBOL_GPL(mpro_screen_notify_off);

void mpro_screen_notify_on(struct mpro_device *mpro)
{
	struct mpro_screen_listener *l;

	if (!mpro)
		return;

	dev_dbg(&mpro->intf->dev, "screen_notify_on\n");

	mutex_lock(&mpro->listeners_lock);
	list_for_each_entry(l, &mpro->screen_listeners, node)
		if (l->screen_on)
			l->screen_on(l->priv);
	mutex_unlock(&mpro->listeners_lock);
}
EXPORT_SYMBOL_GPL(mpro_screen_notify_on);

void mpro_screen_notify_wake(struct mpro_device *mpro)
{
	struct mpro_screen_listener *l;

	if (!mpro)
		return;

	dev_dbg(&mpro->intf->dev, "screen_notify_wake\n");

	mutex_lock(&mpro->listeners_lock);
	list_for_each_entry(l, &mpro->screen_listeners, node)
		if (l->screen_wake)
			l->screen_wake(l->priv);
	mutex_unlock(&mpro->listeners_lock);
}
EXPORT_SYMBOL_GPL(mpro_screen_notify_wake);
