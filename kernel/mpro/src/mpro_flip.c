#include <drm/drm_format_helper.h>
#include "mpro.h"

#ifndef DRM_FORMAT_CONV_STATE_INIT
#include <drm/drm_framebuffer.h>

struct drm_format_conv_state {
	struct {
		void *mem;
		size_t size;
		bool preallocated;
	} tmp;
};

#define __DRM_FORMAT_CONV_STATE_INIT(_mem, _size, _preallocated) { \
		.tmp = { \
			.mem = (_mem), \
			.size = (_size), \
			.preallocated = (_preallocated), \
		} \
	}

#define DRM_FORMAT_CONV_STATE_INIT \
	__DRM_FORMAT_CONV_STATE_INIT(NULL, 0, false)

static void *drm_format_conv_state_reserve(struct drm_format_conv_state *state,
				    size_t new_size, gfp_t flags) {

	void *mem;

	if ( new_size <= state->tmp.size )
		goto out;
	else if ( state -> tmp.preallocated )
		return NULL;

	mem = krealloc(state -> tmp.mem, new_size, flags);
	if ( !mem )
		return NULL;

	state -> tmp.mem = mem;
	state -> tmp.size = new_size;

out:
	return state -> tmp.mem;
}

static unsigned int clip_offset(const struct drm_rect *clip, unsigned int pitch, unsigned int cpp) {
	return clip -> y1 * pitch + clip -> x1 * cpp;
}

/* TODO: Make this function work with multi-plane formats. */
static int __drm_fb_xfrm(void *dst, unsigned long dst_pitch, unsigned long dst_pixsize,
			 const void *vaddr, const struct drm_framebuffer *fb,
			 const struct drm_rect *clip, bool vaddr_cached_hint,
			 struct drm_format_conv_state *state,
			 void (*xfrm_line)(void *dbuf, const void *sbuf, unsigned int npixels)) {

	unsigned long linepixels = drm_rect_width(clip);
	unsigned long lines = drm_rect_height(clip);
	size_t sbuf_len = linepixels * fb -> format -> cpp[0];
	void *stmp = NULL;
	unsigned long i;
	const void *sbuf;

	/*
	 * Some source buffers, such as DMA memory, use write-combine
	 * caching, so reads are uncached. Speed up access by fetching
	 * one line at a time.
	 */
	if ( !vaddr_cached_hint ) {
		stmp = drm_format_conv_state_reserve(state, sbuf_len, GFP_KERNEL);
		if ( !stmp )
			return -ENOMEM;
	}

	if ( !dst_pitch )
		dst_pitch = drm_rect_width(clip) * dst_pixsize;
	vaddr += clip_offset(clip, fb -> pitches[0], fb -> format -> cpp[0]);

	for ( i = 0; i < lines; ++i ) {
		if ( stmp )
			sbuf = memcpy(stmp, vaddr, sbuf_len);
		else
			sbuf = vaddr;
		xfrm_line(dst, sbuf, linepixels);
		vaddr += fb -> pitches[0];
		dst += dst_pitch;
	}

	return 0;
}

/* TODO: Make this function work with multi-plane formats. */
static int __drm_fb_xfrm_toio(void __iomem *dst, unsigned long dst_pitch, unsigned long dst_pixsize,
			      const void *vaddr, const struct drm_framebuffer *fb,
			      const struct drm_rect *clip, bool vaddr_cached_hint,
			      struct drm_format_conv_state *state,
			      void (*xfrm_line)(void *dbuf, const void *sbuf, unsigned int npixels)) {

	unsigned long linepixels = drm_rect_width(clip);
	unsigned long lines = drm_rect_height(clip);
	size_t dbuf_len = linepixels * dst_pixsize;
	size_t stmp_off = round_up(dbuf_len, ARCH_KMALLOC_MINALIGN); /* for sbuf alignment */
	size_t sbuf_len = linepixels * fb -> format -> cpp[0];
	void *stmp = NULL;
	unsigned long i;
	const void *sbuf;
	void *dbuf;

	if ( vaddr_cached_hint ) {
		dbuf = drm_format_conv_state_reserve(state, dbuf_len, GFP_KERNEL);
	} else {
		dbuf = drm_format_conv_state_reserve(state, stmp_off + sbuf_len, GFP_KERNEL);
		stmp = dbuf + stmp_off;
	}
	if ( !dbuf )
		return -ENOMEM;

	if ( !dst_pitch )
		dst_pitch = linepixels * dst_pixsize;
	vaddr += clip_offset(clip, fb -> pitches[0], fb -> format -> cpp[0]);

	for ( i = 0; i < lines; ++i ) {
		if ( stmp )
			sbuf = memcpy(stmp, vaddr, sbuf_len);
		else
			sbuf = vaddr;
		xfrm_line(dbuf, sbuf, linepixels);
		memcpy_toio(dst, dbuf, dbuf_len);
		vaddr += fb -> pitches[0];
		dst += dst_pitch;
	}

	return 0;
}

/* TODO: Make this function work with multi-plane formats. */
static int drm_fb_xfrm(struct iosys_map *dst,
		       const unsigned int *dst_pitch, const u8 *dst_pixsize,
		       const struct iosys_map *src, const struct drm_framebuffer *fb,
		       const struct drm_rect *clip, bool vaddr_cached_hint,
		       struct drm_format_conv_state *state,
		       void (*xfrm_line)(void *dbuf, const void *sbuf, unsigned int npixels)) {

	static const unsigned int default_dst_pitch[DRM_FORMAT_MAX_PLANES] = { 0, 0, 0, 0 };

	if ( !dst_pitch )
		dst_pitch = default_dst_pitch;

	/* TODO: handle src in I/O memory here */
	if ( dst[0].is_iomem )
		return __drm_fb_xfrm_toio(dst[0].vaddr_iomem, dst_pitch[0], dst_pixsize[0],
					  src[0].vaddr, fb, clip, vaddr_cached_hint, state,
					  xfrm_line);
	else
		return __drm_fb_xfrm(dst[0].vaddr, dst_pitch[0], dst_pixsize[0],
				     src[0].vaddr, fb, clip, vaddr_cached_hint, state,
				     xfrm_line);
}

static void drm_fb_xrgb8888_to_rgb565_line_flipped(void *dbuf, const void *sbuf, unsigned int pixels) {

	__le16 *dbuf16 = dbuf;
	const __le32 *sbuf32 = sbuf;
	unsigned int x;
	u16 val16;
	u32 pix;

	for ( x = 0; x < pixels; x++ ) {
		pix = le32_to_cpu(sbuf32[x]);
		val16 = ((pix & 0x00F80000) >> 8) |
			((pix & 0x0000FC00) >> 5) |
			((pix & 0x000000F8) >> 3);
		dbuf16[pixels - 1 - x] = cpu_to_le16(val16);
	}
}

# else

void drm_fb_xrgb8888_to_rgb565(struct iosys_map *dst, const unsigned int *dst_pitch,
                               const struct iosys_map *src, const struct drm_framebuffer *fb,
                               const struct drm_rect *clip, bool swab) {

	struct drm_format_conv_state fmtcnv_state = DRM_FORMAT_CONV_STATE_INIT;
	drm_fb_xrgb8888_to_rgb565(dst, dst_pitch, src, fb, clip, &fmtcnv_state, swab);
}
#endif

void drm_fb_xrgb8888_to_rgb565_flipped(struct iosys_map *dst, const unsigned int *dst_pitch,
			       const struct iosys_map *src, const struct drm_framebuffer *fb,
			       const struct drm_rect *clip, bool swab) {

	static const u8 dst_pixsize[DRM_FORMAT_MAX_PLANES] = { 2, };
	struct drm_format_conv_state fmtcnv_state = DRM_FORMAT_CONV_STATE_INIT;
	void (*xfrm_line)(void *dbuf, const void *sbuf, unsigned int npixels);

	xfrm_line = drm_fb_xrgb8888_to_rgb565_line_flipped;
	drm_fb_xfrm(dst, dst_pitch, dst_pixsize, src, fb, clip, false, &fmtcnv_state, xfrm_line);
}
