# VoCore MPro USB Display Driver

Linux kernel driver for VoCore MPro USB displays (Vendor ID `0xc872`,
Product ID `0x1004`). The display surface is exposed through DRM/KMS,
the backlight as a standard Linux backlight class device, and the
two-finger touchscreen as a multitouch input device. Device does not
survive from resume after suspending, so suspending is disabled for
device, it is emulated by virtual idle state system.

## Supported devices

| Screen ID    | Model     | Description       | Resolution |
| ------------ | --------- | ----------------- | ---------- |
| `0x00000004` | MPRO-4    | MPro 4"           | 480×800    |
| `0x00000104` | MPRO-4    | MPro 4"           | 480×800    |
| `0x00000b04` | MPRO-4    | MPro 4"           | 480×800    |
| `0x00000304` | MPRO-4IN3 | MPro 4.3"         | 480×800    |
| `0x00000005` | MPRO-5    | MPro 5"           | 480×854    |
| `0x00001005` | MPRO-5H   | MPro 5" (OLED)    | 720×1280   |
| `0x00000007` | MPRO-6IN8 | MPro 6.8"         | 800×480    |
| `0x00000403` | MPRO-3IN4 | MPro 3.4" (round) | 800×800    |
| `0x0000000a` | MPRO-10   | MPro 10"          | 1024×600   |

The exact Screen ID is reported in `dmesg` and is also available at
`/sys/bus/usb/drivers/mpro/*/screen_id` once the module is loaded.

## Requirements

* Linux kernel 6.18 or newer
* Kernel configuration: `CONFIG_DRM`, `CONFIG_DRM_GEM_SHMEM_HELPER`,
  `CONFIG_DRM_KMS_HELPER`, `CONFIG_BACKLIGHT_CLASS_DEVICE`,
  `CONFIG_INPUT`, `CONFIG_USB`, `CONFIG_MFD_CORE`,
  `CONFIG_LZ4_COMPRESS`, `CONFIG_LZ4HC_COMPRESS`
* Kernel headers (`/lib/modules/$(uname -r)/build`)

## Building

```sh
make
```

The build produces four modules:

* `mpro.ko` — parent driver (USB probe, MFD registration)
* `drm/mpro_drm.ko` — DRM/KMS display driver
* `backlight/mpro_backlight.ko` — backlight controller
* `touchscreen/mpro_touchscreen.ko` — multitouch input driver

To install:

```sh
sudo make modules_install
sudo depmod -a
```

Registration via DKMS is also supported and recommended if the modules
should be rebuilt automatically on kernel upgrades.

## Usage

### Loading

When an MPro is connected and the modules are installed, they are loaded
automatically through udev. Manual loading:

```sh
sudo modprobe mpro
```

The child modules (`mpro_drm`, `mpro_backlight`, `mpro_touchscreen`)
are pulled in automatically through the MFD framework.

To verify everything was loaded:

```sh
lsmod | grep mpro
dmesg | tail -9
```

A successful load looks roughly like this:
```
mpro_touchscreen	16384	0
mpro_backlight		12288	0
mpro_drm		28672	0
mpro			40960	3 mpro_touchscreen,mpro_backlight,mpro_drm

mpro 3-3:1.0: Detected MPRO 6.8"
mpro 3-3:1.0: Firmware: v0.25, 800x480, VOCORE-6.8IN-SCREEN
mpro 3-3:1.0: USB autosuspend enabled (delay 30s)
mpro 3-3:1.0: MPRO core registered
usbcore: registered new interface driver mpro
[drm] Initialized mpro_drm 1.0.0 for mpro_drm on minor 1
mpro_backlight mpro_backlight: backlight registered: mpro-3-3:1.0 (default 100/255)
input: MPro touchscreen as /devices/pci0000:00/0000:00:05.0/usb3/3-3/3-3:1.0/mpro_touchscreen/input/input16
mpro_touchscreen mpro_touchscreen: touchscreen registered (800x480, 2-finger MT)
```

### Console (fbcon)

To make the kernel text console appear on the MPro display, append the
following to the kernel command line:

```
mpro.fbdev=1
fbcon=map:1
```

If another DRM device is present (such as `virtio_gpu` or an integrated
GPU), it usually claims `fb0` and the MPro becomes `fb1`. The other
device can be hidden with `module_blacklist=virtio_gpu` if desired.

Note that fbdev emulation prevents runtime power management from
suspending the display, since fbdev keeps the display pipe permanently
active.

### Graphical environment

The DRM device (`/dev/dri/card1` or similar) works directly with Wayland
compositors (Weston, Sway, Mutter) and X.Org.

```sh
weston --backend=drm --drm-device=card1 --use-pixman
```

### Video playback

Video playback to the DRM device works from the console using common
players such as `ffplay` or `mpv`, provided they were built with DRM
output support.

```sh
mpv --vo=drm --drm-device=/dev/dri/card1 --hwdec=no bbb_sunflower_1080p_30fps_normal.mp4
```

Some sample videos can be found from
[here](https://test-videos.co.uk/bigbuckbunny/mp4-h264).

## Sysfs interfaces

### Parent driver (`/sys/bus/usb/drivers/mpro/<usb-id>/`)

| File             | Mode | Description                                                                  |
| ---------------- | ---- | ---------------------------------------------------------------------------- |
| `model`          | r--  | Short model name (e.g. `MPRO-6IN8`)                                          |
| `description`    | r--  | Human-readable model description                                             |
| `firmware`       | r--  | Firmware version string as reported by the device                            |
| `fw_major`       | r--  | Parsed firmware version, major number                                        |
| `fw_minor`       | r--  | Parsed firmware version, minor number                                        |
| `resolution`     | r--  | `WIDTH HEIGHT` in pixels                                                     |
| `physical_size`  | r--  | `WIDTH_MM HEIGHT_MM`                                                         |
| `width_mm`       | r--  | `WIDTH_MM` (separate field for convenience)                                  |
| `height_mm`      | r--  | `HEIGHT_MM` (separate field for convenience)                                 |
| `screen_id`      | r--  | Device's Screen ID (hex)                                                     |
| `version_id`     | r--  | Device's hardware version ID (hex)                                           |
| `device_id`      | r--  | Device's unique 8-byte ID                                                    |
| `margin`         | r--  | Frame margin in bytes (0 on most models)                                     |
| `fbdev_enabled`  | r--  | Whether fbdev console emulation is enabled                                   |
| `lz4_level`      | rw   | LZ4 compression: `0`=off, `1`=fast, `2..12`=HC                               |
| `lz4_threshold`  | rw   | LZ4 compression threshold for compression                                    |
| `idle_delay_ms`  | rw   | Idle delay in ms before backlight is switched off (0 = disabled)             |
| `idle_state`     | rw   | Read: `active` or `idle`. Write: force the state.                            |
| `active_refs`    | r--  | Number of children currently holding an active reference                     |
| `touch_wake`     | rw   | Wake the display from virtual idle on touch (requires `mpro_touchscreen.ko`) |
| `pm_stats`       | r--  | Cumulative idle/active state statistics                                      |
| `reset_pm_stats` | -w-  | Write `1` to reset PM statistics counters                                    |
| `fps`            | r--  | Measured frames per second sent to the device                                |
| `stats`          | r--  | `submitted=N displayed=N dropped=N fps=N efficiency=N`                       |
| 'reset_stats'    | -w-  | Reset statistic counters by writing 1 to this attribute                      |

### DRM (`/sys/bus/platform/drivers/mpro_drm/<id>/`)

| File              | Mode | Description                                              |
| ----------------- | ---- | -------------------------------------------------------- |
| `rotation`        | rw   | `0`..`7` (0..3 = rotate, 4..7 = rotate + reflect-x)      |
| `brightness`      | rw   | Software brightness 0..100                               |
| `gamma`           | rw   | Gamma correction `0.50`..`4.00` (default `1.00`)         |
| `disable_partial` | rw   | `0`/`1` — force full-frame updates                       |
| `gamma_lut`       | -w-  | Direct 768-byte LUT write (R[256] G[256] B[256])         |

### Backlight (`/sys/class/backlight/mpro-<id>/`)

Standard Linux backlight class interface:

| File             | Mode | Description                                              |
| ---------------- | ---- | -------------------------------------------------------- |
| `brightness`     | rw   | Brightness value 0..255                                  |
| `max_brightness` | r--  | Maximum value (255)                                      |
| `bl_power`       | rw   | Standard power state                                     |
| `gamma`          | rw   | Backlight gamma curve `0.50`..`4.00`                     |

Compatible with tools such as `brightnessctl`, `xbacklight`, GNOME and KDE panels.

```sh
brightnessctl --device='mpro-3-3' set 50%
echo 200 > /sys/class/backlight/mpro-3-3/brightness
```

### Touchscreen (`/dev/input/eventN`)

A standard Linux input device using the multitouch protocol. It can be
identified with `evtest` and `libinput` by the name "MPro touchscreen".

```sh
evtest /dev/input/eventN
libinput debug-events --device /dev/input/eventN
```

## Module parameters

### `mpro`

```sh
# /etc/modprobe.d/mpro.conf
options mpro fbdev=1
options mpro lz4_level=1
options mpro lz4_threshold=2048
```

| Parameter       | Type | Default | Description                                                                                       |
| --------------- | ---- | ------- | ------------------------------------------------------------------------------------------------- |
| `fbdev`         | int  | `0`     | Enable fbdev console emulation. Disables USB autosuspend.                                         |
| `lz4_level`     | int  | `0`     | LZ4 compression level at boot. Requires firmware ≥ 0.22 and a non-margin model.                   |
| `lz4_threshold` | int  | `1024`  | LZ4 compression threshold, do not compress transfers below this size.                             |
| `idle_delay_ms` | uint | `30000` | Initial idle delay before the display goes idle (backlight off, touch URB stopped). 0 = disabled. |
| `touch_wake`     | bool | `1`    | Wake the display from virtual idle when the touchscreen is touched. Default: enabled.            |

### `mpro_drm`

| Parameter         | Type | Default | Description                                                                            |
| ----------------- | ---- | ------- | -------------------------------------------------------------------------------------- |
| `disable_partial` | bool | `0`     | Force full-frame updates. Required on old firmware (≤ 0.15). Also exposed via sysfs.   |

### `mpro_backlight`

| Parameter            | Type | Default | Description                                                                |
| -------------------- | ---- | ------- | -------------------------------------------------------------------------- |
| `default_brightness` | int  | `100`   | Initial backlight brightness (0..255). The device has no readback, so this value is sent on every probe. |
| `default_gamma`      | int  | `100`   | Initial backlight gamma × 100 (50..400). 100 = linear curve.               |

## Performance

The parent driver's pipeline uses a latest-wins coalescing strategy:
when an application produces frames faster than USB can deliver them,
older pending frames are dropped automatically. The `stats` sysfs file
exposes how the pipeline is performing:

```sh
$ cat /sys/bus/usb/drivers/mpro/3-3:1.0/stats
submitted=7200 displayed=2040 dropped=5160 fps=34.00 efficiency=28.33%
```

In this example the compositor submitted about 120 frame updates
per second to the pipeline, but only ~34 fps reached the device —
the limit being USB 2.0 throughput. The remaining updates were
coalesced (newer frames replacing older pending ones).
LZ4 compression can significantly improve effective throughput:

```sh
echo 1 > /sys/bus/usb/drivers/mpro/3-3:1.0/lz4_level
```

Levels:

* `1` = LZ4 default (fastest, moderate compression — recommended)
* `2..12` = LZ4HC (higher compression ratio, slower)

LZ4 is not available on margin models (legacy MPRO-5 units) or older
firmware.

The `lz4_threshold` parameter controls the minimum frame size in bytes
for LZ4 compression. Frames smaller than this are sent uncompressed
because the compression overhead outweighs the bandwidth saved.

## Statistics

Pipeline statistics can be resetted with

```sh
echo 1 > /sys/bus/usb/drivers/mpro/3-3:1.0/reset_stats
```

## Power management

The device firmware does not implement a reliable USB-bus resume
protocol — once the USB host puts the device into selective suspend,
control transfers fail and the link must be reset. Real USB
autosuspend is therefore disabled.

Instead the driver implements a virtual idle state. After a
configurable period of inactivity, the backlight is switched off
and the touchscreen URB pipeline is stopped. The USB bus stays
active, so resume is instantaneous and reliable when an application
re-opens the display or the input device.

This is essential because the device's backlight has a limited
lifetime — leaving it lit through long idle periods (e.g. overnight)
shortens it noticeably.

### Configuration

The idle delay can be set at module load time:

    options mpro idle_delay_ms=60000     # 60-second idle delay
    options mpro idle_delay_ms=0          # disable virtual idle

Or at runtime via sysfs:

    echo 60000 > /sys/bus/usb/drivers/mpro/<id>/idle_delay_ms

### Status

    cat /sys/bus/usb/drivers/mpro/<id>/idle_state        # active|idle
    cat /sys/bus/usb/drivers/mpro/<id>/active_refs       # number of holders

### Manual control

    echo idle > /sys/bus/usb/drivers/mpro/<id>/idle_state    # force idle now
    echo active > /sys/bus/usb/drivers/mpro/<id>/idle_state  # force active now

### Wake on touch

By default, touching the screen wakes the display from idle. This
requires the `mpro_touchscreen.ko` module to be loaded — the
parameter is set on the parent (`mpro`), but the wake mechanism
relies on the touchscreen URB pipeline, which is provided by the
child module.

If `mpro_touchscreen` is not loaded, the `touch_wake` setting has
no effect and the display will only wake when the DRM pipe is
re-enabled (e.g. when a compositor opens it).

#### Setting at module load:

    options mpro touch_wake=0

#### Setting at runtime via sysfs:

    cat /sys/bus/usb/drivers/mpro/<id>/touch_wake     # 0 or 1
    echo 0 > /sys/bus/usb/drivers/mpro/<id>/touch_wake
    echo 1 > /sys/bus/usb/drivers/mpro/<id>/touch_wake

The runtime change takes effect on the next idle cycle. To force
the change to take effect immediately, manually toggle the idle
state:

    echo active > /sys/bus/usb/drivers/mpro/<id>/idle_state

Note: this wake mechanism is independent of system suspend (S3,
hibernate). The device firmware does not advertise USB remote
wakeup (`bmAttributes & 0x20 == 0` in the configuration
descriptor), so a touch cannot wake the host from system suspend
— only from the driver's virtual idle state.

### Statistics

The driver tracks idle/active state for diagnostics:

    cat /sys/bus/usb/drivers/mpro/<id>/pm_stats
    state=active
    active_time_ms=125430
    idle_time_ms=580120
    current_state_time_ms=4150
    idle_transitions=12
    wake_transitions=12
    touch_wakes=8

Fields:

  - `state` — current state, `active` or `idle`
  - `active_time_ms` — total time spent in the active state across
    completed sessions, in milliseconds
  - `idle_time_ms` — total time spent in the idle state across
    completed sessions, in milliseconds
  - `current_state_time_ms` — duration of the current state
    (not yet counted in `active_time_ms` / `idle_time_ms`)
  - `idle_transitions` — number of active → idle transitions since
    the last reset
  - `wake_transitions` — number of idle → active transitions
  - `touch_wakes` — subset of `wake_transitions` that were caused
    by a touch event (the rest are caused by the DRM pipe or
    touchscreen input device opening)

Reset counters and timers:

    echo 1 > /sys/bus/usb/drivers/mpro/<id>/reset_pm_stats

After a reset, `current_state_time_ms` resumes from zero and the
cumulative totals are cleared. The state itself is unchanged.

### fbdev console mode

When `fbdev=1` is set, the kernel text console keeps the display
pipe permanently active. The idle timer is disabled in this mode —
the console remains visible until userspace explicitly switches
modes.

## Troubleshooting

### Console does not appear on the MPro display

Add the boot parameters `mpro.fbdev=1` and `fbcon=map:1`. If they are
already set, verify that no other DRM driver is holding the console
binding:

```sh
for v in /sys/class/vtconsole/vtcon*; do
    echo "$v: $(cat $v/name) bound=$(cat $v/bind)"
done
```

The console binding can be moved manually:

```sh
echo 0 > /sys/class/vtconsole/vtcon0/bind
echo 1 > /sys/class/vtconsole/vtcon1/bind
```

### Image is upside down or mirrored

Adjust the `rotation` attribute:

```sh
for i in 0 1 2 3 4 5 6 7; do
    echo "rotation=$i"
    echo $i > /sys/bus/platform/drivers/mpro_drm/*/rotation
    sleep 2
done
```

Values 0..3 are pure rotations (0°, 90°, 180°, 270°); 4..7 are the same
plus an X-axis reflection.

The native rotation of each model is encoded in `mpro_model.c` and
applied automatically at probe time. If a new model needs different
defaults, please open an issue.

### Touchscreen does not respond

First verify that the input device is registered:

```sh
ls /dev/input/event*
cat /proc/bus/input/devices | grep -A4 "MPro touchscreen"
```

If it is missing, check `dmesg`. The most common cause is that MFD
could not auto-load `mpro_touchscreen` because `modules.alias` is out
of date:

```sh
sudo depmod -a
sudo modprobe mpro_touchscreen
```

### LZ4 freezes the display

Older firmware (< 0.22) does not support LZ4. Disable it:

```sh
echo 0 > /sys/bus/usb/drivers/mpro/*/lz4_level
```

## Architecture

The driver is built on the MFD (Multi-Function Device) framework. A
single USB device exposes three independent function blocks:

```
mpro (USB driver, MFD parent)
├── mpro_drm        (DRM/KMS display)
├── mpro_backlight  (backlight class)
└── mpro_touchscreen (input device)
```

The parent driver (`mpro`) handles all USB I/O: synchronous control
messages, the asynchronous frame pipeline, and probe-time queries. The
child modules use the parent API to communicate with the device — they
never call USB functions directly.

### Source layout

```
mpro/
├── LICENSE
├── README.md
├── Makefile, Kbuild
├── dkms.conf
├── mpro.h                      # public API for child drivers
├── mpro_internal.h             # internal protocol definitions
├── mpro_main.c                 # USB probe/disconnect/PM, MFD registration
├── mpro_protocol.c             # synchronous control messages
├── mpro_pipeline.c             # async frame pipeline + LZ4
├── mpro_pm.c                   # virtual idle state / "power management"
├── mpro_model.c                # model database + detection
├── mpro_touch.c                # touch input api
├── mpro_screen.c               # screen-state callback mechanism
├── mpro_sysfs.c                # parent-level sysfs
├── drm/
│   ├── Kbuild, mpro_drm.h
│   ├── mpro_drm_main.c         # probe + drm_driver registration
│   ├── mpro_drm_pipe.c         # pipe enable/disable/update, vblank, connector
│   ├── mpro_drm_color.c        # gamma, brightness, format conversion
│   ├── mpro_drm_sysfs.c        # rotation, brightness, gamma sysfs
│   └── mpro_drm_edid.c         # EDID 1.3 emulation
├── backlight/
│   ├── Kbuild, mpro_backlight.h
│   └── mpro_backlight.c
└── touchscreen/
├── Kbuild, mpro_touchscreen.h
└── mpro_touchscreen.c
```

### Frame pipeline

The DRM driver submits frames through one of two parent functions:

```c
int mpro_send_full_frame(struct mpro_device *mpro,
const void *data, size_t len);
int mpro_send_partial_frame(struct mpro_device *mpro,
const void *data,
u16 x, u16 y, u16 w, u16 h);
```

The parent's pipeline accepts the frame, optionally compresses it with
LZ4, and sends it over USB. Only one transfer is in flight at any time;
new frames replace any pending frame (latest-wins coalescing). This
provides natural backpressure — the producer cannot outrun USB
throughput, and stale frames are dropped automatically.

### Screen-state mechanism

The DRM `pipe_enable` / `pipe_disable` callbacks fan out notifications
to all registered listeners:

```c
struct mpro_screen_listener {
void (*screen_off)(void *priv);
void (*screen_on)(void *priv);
void *priv;
};

mpro_screen_listener_register(mpro, &listener);
```

The backlight uses this to switch off the LEDs when the DRM pipe goes
blank; the touchscreen uses it to stop the URB pipeline and avoid
spurious power draw while the display is off.

## License

GPL-2.0. See the `SPDX-License-Identifier` lines in each source file.

## Author

Oskari Rauta — `<oskari.rauta@gmail.com>`

Partly based on VoCore's original `mpro_drm` driver
(<https://github.com/Vonger/mpro_drm>) and the
[`v2scrctl` userspace package](https://vocore.io/misc/v2scrctl.zip)
for the LZ4 protocol, multitouch decoding, and firmware-version probe
sequence.
