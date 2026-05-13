/*
 * dpf-test.c - Simple DRM/KMS draw test for the AX206 photo frame
 *
 * Draws a sequence of test patterns to verify the display is working:
 *   1. Solid color fills (red, green, blue)
 *   2. Vertical color bars
 *   3. Checkerboard
 *   4. Gradient
 *
 * Build:
 *   gcc -o dpf-test dpf-test.c $(pkg-config --cflags --libs libdrm)
 *
 * Run:
 *   ./dpf-test              (auto-detects first dpf DRM device)
 *   ./dpf-test /dev/dri/cardN
 *
 * Press Enter to advance to the next pattern.
 * Ctrl-C or 'q' to quit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <glob.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>

/* ---- RGB565 helpers ----
 *
 * Standard little-endian RGB565 as used by all DRM applications.
 * The byte-swap for the AX206 device is handled in the kernel driver
 * (dpf-drm-plane.c) so this program does not need to know about it.
 */

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint16_t)(r >> 3) << 11)
	     | ((uint16_t)(g >> 2) <<  5)
	     |  (uint16_t)(b >> 3);
}

/* ---- DRM state ---- */

struct dpf_test {
	int		fd;
	uint32_t	conn_id;
	uint32_t	crtc_id;
	uint32_t	fb_id;
	uint32_t	handle;
	uint32_t	pitch;
	uint64_t	size;
	void		*map;
	uint32_t	width;
	uint32_t	height;
	drmModeModeInfo	mode;
};

static int find_drm_device(char *out, size_t outlen)
{
	glob_t g;
	int ret = -1;

	if (glob("/dev/dri/card*", 0, NULL, &g) != 0)
		return -1;

	for (size_t i = 0; i < g.gl_pathc; i++) {
		int fd = open(g.gl_pathv[i], O_RDWR);
		if (fd < 0)
			continue;

		/* Check if it's our dpf device by looking for the driver name */
		drmVersionPtr v = drmGetVersion(fd);
		if (v) {
			if (strcmp(v->name, "ax206") == 0) {
				snprintf(out, outlen, "%s", g.gl_pathv[i]);
				drmFreeVersion(v);
				close(fd);
				ret = 0;
				break;
			}
			drmFreeVersion(v);
		}
		close(fd);
	}

	globfree(&g);
	return ret;
}

static int setup_drm(struct dpf_test *t, const char *path)
{
	drmModeRes *res;
	drmModeConnector *conn = NULL;
	drmModeEncoder *enc = NULL;
	struct drm_mode_create_dumb create = {};
	struct drm_mode_map_dumb map_req = {};
	uint32_t handles[4] = {}, pitches[4] = {}, offsets[4] = {};
	int i;

	t->fd = open(path, O_RDWR);
	if (t->fd < 0) {
		perror("open DRM device");
		return -1;
	}

	/* Need dumb buffer support */
	if (drmSetClientCap(t->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) < 0)
		fprintf(stderr, "warn: universal planes not available\n");

	res = drmModeGetResources(t->fd);
	if (!res) {
		fprintf(stderr, "drmModeGetResources failed\n");
		return -1;
	}

	/* Find a connected connector */
	for (i = 0; i < res->count_connectors; i++) {
		conn = drmModeGetConnector(t->fd, res->connectors[i]);
		if (!conn) continue;
		if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0)
			break;
		drmModeFreeConnector(conn);
		conn = NULL;
	}

	if (!conn) {
		fprintf(stderr, "No connected connector found\n");
		drmModeFreeResources(res);
		return -1;
	}

	t->conn_id = conn->connector_id;
	t->mode    = conn->modes[0];
	t->width   = t->mode.hdisplay;
	t->height  = t->mode.vdisplay;

	printf("Connector: %ux%u\n", t->width, t->height);

	/* Find encoder + CRTC */
	if (conn->encoder_id) {
		enc = drmModeGetEncoder(t->fd, conn->encoder_id);
	}
	if (enc) {
		t->crtc_id = enc->crtc_id;
		drmModeFreeEncoder(enc);
	}
	if (!t->crtc_id && res->count_crtcs > 0)
		t->crtc_id = res->crtcs[0];

	drmModeFreeConnector(conn);
	drmModeFreeResources(res);

	/* Create dumb buffer (RGB565 = 16 bpp) */
	create.width  = t->width;
	create.height = t->height;
	create.bpp    = 16;

	if (drmIoctl(t->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create) < 0) {
		perror("DRM_IOCTL_MODE_CREATE_DUMB");
		return -1;
	}

	t->handle = create.handle;
	t->pitch  = create.pitch;
	t->size   = create.size;

	/* Create framebuffer */
	handles[0] = t->handle;
	pitches[0] = t->pitch;

	if (drmModeAddFB2(t->fd, t->width, t->height,
			  DRM_FORMAT_RGB565,
			  handles, pitches, offsets,
			  &t->fb_id, 0) < 0) {
		perror("drmModeAddFB2");
		return -1;
	}

	/* Map dumb buffer */
	map_req.handle = t->handle;
	if (drmIoctl(t->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_req) < 0) {
		perror("DRM_IOCTL_MODE_MAP_DUMB");
		return -1;
	}

	t->map = mmap(NULL, t->size, PROT_READ | PROT_WRITE,
		      MAP_SHARED, t->fd, map_req.offset);
	if (t->map == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	/* Activate CRTC with our framebuffer */
	if (drmModeSetCrtc(t->fd, t->crtc_id, t->fb_id,
			   0, 0, &t->conn_id, 1, &t->mode) < 0) {
		perror("drmModeSetCrtc");
		return -1;
	}

	return 0;
}

static void cleanup_drm(struct dpf_test *t)
{
	struct drm_mode_destroy_dumb destroy = { .handle = t->handle };

	if (t->map && t->map != MAP_FAILED)
		munmap(t->map, t->size);
	if (t->fb_id)
		drmModeRmFB(t->fd, t->fb_id);
	if (t->handle)
		drmIoctl(t->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
	if (t->fd >= 0)
		close(t->fd);
}

/* ---- Flush: notify the driver the framebuffer has changed ---- */

static void flush_fb(struct dpf_test *t)
{
	/*
	 * drmModeDirtyFB sends DRM_IOCTL_MODE_DIRTYFB which triggers an
	 * atomic commit through the driver's fb_dirty path.  Without this,
	 * writes to the mmap'd dumb buffer are invisible to the driver.
	 * Passing NULL clip marks the entire framebuffer as dirty.
	 */
	drmModeDirtyFB(t->fd, t->fb_id, NULL, 0);
}

/* ---- Draw helpers ---- */

static void fill(struct dpf_test *t, uint16_t color)
{
	uint16_t *fb = t->map;
	uint32_t pixels = t->width * t->height;

	for (uint32_t i = 0; i < pixels; i++)
		fb[i] = color;
}

static void color_bars(struct dpf_test *t)
{
	uint16_t *fb = t->map;
	/* 8 equal-width bars, standard RGB565 */
	static const uint16_t bars[] = {
		0xFFFF, /* white   */
		0xFFE0, /* yellow  */
		0x07FF, /* cyan    */
		0x07E0, /* green   */
		0xF81F, /* magenta */
		0xF800, /* red     */
		0x001F, /* blue    */
		0x0000, /* black   */
	};

	for (uint32_t y = 0; y < t->height; y++) {
		for (uint32_t x = 0; x < t->width; x++) {
			int bar = (int)(x * 8 / t->width);
			fb[y * (t->pitch / 2) + x] = bars[bar];
		}
	}
}

static void checkerboard(struct dpf_test *t)
{
	uint16_t *fb = t->map;
	uint32_t cell = t->width / 16;
	if (cell < 4) cell = 4;

	for (uint32_t y = 0; y < t->height; y++) {
		for (uint32_t x = 0; x < t->width; x++) {
			/* Use (x/cell + y/cell) & 1 so cells tile perfectly
			 * regardless of whether width is divisible by cell. */
			int cx = (int)(x / cell);
			int cy = (int)(y / cell);
			fb[y * (t->pitch / 2) + x] =
				((cx ^ cy) & 1) ? rgb565(255, 255, 255)
						: rgb565(20,  20,  20);
		}
	}
}

static void gradient(struct dpf_test *t)
{
	uint16_t *fb = t->map;

	for (uint32_t y = 0; y < t->height; y++) {
		for (uint32_t x = 0; x < t->width; x++) {
			uint8_t r = (uint8_t)(255 * x / t->width);
			uint8_t g = (uint8_t)(255 * y / t->height);
			uint8_t b = (uint8_t)(128);
			fb[y * (t->pitch / 2) + x] = rgb565(r, g, b);
		}
	}
}

static void cross(struct dpf_test *t, uint16_t color)
{
	uint16_t *fb = t->map;
	uint32_t cx = t->width / 2, cy = t->height / 2;

	for (uint32_t x = 0; x < t->width; x++)
		fb[cy * (t->pitch / 2) + x] = color;
	for (uint32_t y = 0; y < t->height; y++)
		fb[y * (t->pitch / 2) + cx] = color;
}

/* ---- Main ---- */

struct pattern {
	const char *name;
	void (*draw)(struct dpf_test *);
};

static void draw_red(struct dpf_test *t)   { fill(t, rgb565(200, 0, 0)); }
static void draw_green(struct dpf_test *t) { fill(t, rgb565(0, 200, 0)); }
static void draw_blue(struct dpf_test *t)  { fill(t, rgb565(0, 0, 200)); }
static void draw_cross(struct dpf_test *t)
{
	fill(t, rgb565(30, 30, 30));
	cross(t, rgb565(255, 255, 0));
}

static const struct pattern patterns[] = {
	{ "Red fill",       draw_red       },
	{ "Green fill",     draw_green     },
	{ "Blue fill",      draw_blue      },
	{ "Color bars",     color_bars     },
	{ "Checkerboard",   checkerboard   },
	{ "Gradient",       gradient       },
	{ "Cross",          draw_cross     },
};
#define N_PATTERNS ((int)(sizeof(patterns) / sizeof(patterns[0])))

static void wait_enter(void)
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		if (c == 'q' || c == 'Q') exit(0);
}

int main(int argc, char *argv[])
{
	struct dpf_test t = {};
	char devpath[256];
	int ret;

	if (argc >= 2) {
		snprintf(devpath, sizeof(devpath), "%s", argv[1]);
	} else {
		if (find_drm_device(devpath, sizeof(devpath)) < 0) {
			fprintf(stderr,
				"No dpf DRM device found. "
				"Specify path manually: %s /dev/dri/cardN\n",
				argv[0]);
			return 1;
		}
		printf("Found device: %s\n", devpath);
	}

	ret = setup_drm(&t, devpath);
	if (ret < 0) {
		cleanup_drm(&t);
		return 1;
	}

	printf("Display: %ux%u  pitch: %u bytes\n",
	       t.width, t.height, t.pitch);
	printf("Press Enter for next pattern, 'q' to quit.\n\n");

	for (int i = 0; i < N_PATTERNS; i++) {
		printf("[%d/%d] %s ... ", i + 1, N_PATTERNS, patterns[i].name);
		fflush(stdout);

		patterns[i].draw(&t);
		flush_fb(&t);

		printf("drawn. Press Enter.\n");
		wait_enter();
	}

	/* Leave display black on exit */
	fill(&t, 0);
	flush_fb(&t);

	cleanup_drm(&t);
	printf("Done.\n");
	return 0;
}
