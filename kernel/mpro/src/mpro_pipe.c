#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_format_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include "mpro.h"

static char cmd_quit_sleep[6] = {
	0x00, 0x29, 0x00, 0x00, 0x00, 0x00
};

static void mpro_pipe_enable(struct drm_simple_display_pipe *pipe,
				 struct drm_crtc_state *crtc_state,
				 struct drm_plane_state *plane_state) {

	struct mpro_device *mpro = to_mpro(pipe -> crtc.dev);
	struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(plane_state);
	struct drm_framebuffer *fb = plane_state -> fb;
	struct drm_rect rect = {
		.x1 = 0,
		.x2 = fb -> width,
		.y1 = 0,
		.y2 = fb -> height,
	};

	mpro_send_command(mpro, cmd_quit_sleep, sizeof(cmd_quit_sleep));
	mpro_fb_mark_dirty(&shadow_plane_state -> data[0], fb, &rect);
}

static void mpro_pipe_disable(struct drm_simple_display_pipe *pipe) {

	//struct mpro_device *mpro = to_mpro(pipe -> crtc.dev);
}

static void mpro_pipe_update(struct drm_simple_display_pipe *pipe,
				 struct drm_plane_state *old_state) {

	struct drm_plane_state *state = pipe -> plane.state;
	struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(state);
	struct drm_framebuffer *fb = state -> fb;
	struct drm_rect rect;

	if ( !pipe -> crtc.state -> active )
		return;

	if ( drm_atomic_helper_damage_merged(old_state, state, &rect))
		mpro_fb_mark_dirty(&shadow_plane_state -> data[0], fb, &rect);
}

static const struct drm_simple_display_pipe_funcs mpro_pipe_funcs = {
	.enable	    = mpro_pipe_enable,
	.disable    = mpro_pipe_disable,
	.update	    = mpro_pipe_update,
	DRM_GEM_SIMPLE_DISPLAY_PIPE_SHADOW_PLANE_FUNCS,
};

static const uint32_t mpro_pipe_formats[] = {
	DRM_FORMAT_XRGB8888,
};

static const uint64_t mpro_pipe_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

int mpro_pipe_init(struct mpro_device *mpro) {

	int ret;

	ret = drm_simple_display_pipe_init(&mpro->dev, &mpro->pipe,
					   &mpro_pipe_funcs, mpro_pipe_formats,
					   ARRAY_SIZE(mpro_pipe_formats),
					   mpro_pipe_modifiers, &mpro->conn);
	if ( ret )
		return ret;

	drm_plane_enable_fb_damage_clips(&mpro -> pipe.plane);

	return 0;
}
