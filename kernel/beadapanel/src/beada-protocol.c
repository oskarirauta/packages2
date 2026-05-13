// SPDX-License-Identifier: GPL-2.0-only
/*
 * beada-protocol.c – Panel-Link and Status-Link message construction
 *
 * This file is responsible for BUILDING protocol messages (filling structs,
 * computing checksums).  It has no knowledge of USB – all actual transfers
 * happen in beada-usb.c and beada-pipeline.c.
 *
 * Two protocols share a common RFC 1071 checksum algorithm:
 *
 *   Panel-Link  – pixel stream framing tags
 *   Status-Link – device control commands and responses
 *
 * See beada-priv.h for the wire-format struct definitions.
 */

#include <linux/string.h>

#include "beada-priv.h"

/* ------------------------------------------------------------------ */
/* Shared checksum                                                      */
/* ------------------------------------------------------------------ */

/**
 * beada_checksum16() - RFC 1071 one's-complement 16-bit checksum
 * @data: Buffer start
 * @len:  Number of bytes (must be even)
 *
 * Identical to checksum16() in both panelLinkProtocol.c and
 * statusLinkProtocol.c (NXElec reference implementation).
 */
__sum16 beada_checksum16(const void *data, size_t len)
{
	const __le16 *words = (const __le16 *)data;
	unsigned long sum = 0;
	size_t i;

	for (i = 0; i < len / 2; i++)
		sum += le16_to_cpu(words[i]);

	/* Fold 32-bit carry into 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return (__sum16)(~sum);
}

/* ------------------------------------------------------------------ */
/* Panel-Link message builder                                           */
/* ------------------------------------------------------------------ */

/**
 * pl_fill_tag() - Populate a Panel-Link tag in-place
 * @tag:    Buffer to fill; must be sizeof(struct panellink_tag) = 270 bytes
 * @type:   PL_TYPE_START, PL_TYPE_END, or PL_TYPE_RESET
 * @fmtstr: GStreamer caps string to embed in @tag->fmtstr (TYPE_START only).
 *          Pass NULL to zero-fill the fmtstr field (TYPE_END, TYPE_RESET).
 *
 * The checksum covers the first 268 bytes of the struct (everything before
 * the checksum16 field), matching the NXElec implementation:
 *   checksum16(data, (sizeof(PANELLINK_STREAM_TAG) - 2) / 2)
 */
void pl_fill_tag(struct panellink_tag *tag, u8 type, const char *fmtstr)
{
	memset(tag, 0, sizeof(*tag));
	memcpy(tag->protocol_name, "PANEL-LINK", sizeof(tag->protocol_name));
	tag->version = PL_VERSION;
	tag->type    = type;

	if (fmtstr)
		strscpy(tag->fmtstr, fmtstr, sizeof(tag->fmtstr));

	tag->checksum16 = beada_checksum16(tag,
			      offsetof(struct panellink_tag, checksum16));
}

/* ------------------------------------------------------------------ */
/* Status-Link message builder                                          */
/* ------------------------------------------------------------------ */

/**
 * sl_fill_tag() - Populate a Status-Link header in-place
 * @tag:        Header to fill; must be sizeof(struct statuslink_tag) = 20 bytes
 * @type:       SL_TYPE_* command code to embed
 * @packet_len: Total packet size (header + payload) written into tag->length
 *              so the device knows how many bytes to expect or return.
 *
 * Checksum scope:
 *   The NXElec implementation (statusLinkProtocol.c, packageDummySL()) always
 *   checksums exactly sizeof(STATUSLINK_TAG)-2 = 18 bytes – i.e. the header
 *   bytes that precede the checksum16 field – regardless of payload length.
 *   The payload bytes are NEVER included in the checksum.
 *
 *   Concretely: beada_checksum16(tag, 18) covers bytes 0..17, which is
 *   protocol_name[11] + version + type + reserved1 + sequence_number(2) +
 *   length(2) = 18 bytes, stopping just before checksum16 itself.
 *
 * Caller must write any payload into the packet BEFORE calling this function
 * if the payload affects tag->length (it does not affect the checksum, but
 * packet_len must already be correct when this function is called).
 */
void sl_fill_tag(struct statuslink_tag *tag, u8 type, u16 packet_len)
{
	memset(tag, 0, sizeof(*tag));
	memcpy(tag->protocol_name, "STATUS-LINK", sizeof(tag->protocol_name));
	tag->version = SL_VERSION;
	tag->type    = type;
	/* reserved1 = 0 (set by memset) */
	/* sequence_number = 0 (device accepts any value) */
	tag->length  = cpu_to_le16(packet_len);

	tag->checksum16 = beada_checksum16(tag,
			      offsetof(struct statuslink_tag, checksum16));
}
