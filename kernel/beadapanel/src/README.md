# BeadaPanel Linux driver

A Linux kernel driver for [NXElec](https://www.nxelec.com/) BeadaPanel USB
display modules.  Exposes the panel as a standard DRM/KMS framebuffer with
backlight class support, so any compositor (Weston, kmscube, Sway, X11) can
target it like any other display.

## Status

Tested on **Alpine Linux** with **kernel 6.18.28-lts** and BeadaPanel model 3
(320×480, T113 platform, firmware 7.19).  Should work on any Linux kernel
**6.11 or newer**.

The driver covers Panel-Link v1.0 and Status-Link v1.3 protocols; brightness
control, gamma correction, rotation, suspend/resume, and USB autosuspend all
work.

## Supported devices

USB vendor `0x4e58` (NXElec) with product IDs `0x1001` and `0x1002`.  Known
panel models the driver auto-detects via Status-Link `GET_INFO`:

| Model | Resolution | Physical size |
|-------|------------|---------------|
| 7     | 800x480    | 188x118 mm    |
| 5     | 800x480    | 121x76 mm     |
| 6     | 480x1280   | 67x181 mm     |
| 3     | 320x480    | 70x99 mm      |
| 4     | 480x800    | 78x116 mm     |
| 5C    | 800x480    | 121x76 mm     |
| 5T    | 800x480    | 108x65 mm     |
| 7C    | 800x480    | 110x62 mm     |
| 3C    | 480x320    | 62x40 mm      |
| 4C    | 800x480    | 94x56 mm      |
| 6C    | 1280x480   | 161x60 mm     |
| 6S    | 1280x480   | 161x60 mm     |
| 2     | 480x480    | 73x73 mm      |
| 2W    | 480×480    | 90×90 mm      |
| 7S    | 1280x480   | 190x59 mm     |
| 5S    | 480x854    | 77x140 mm     |
| 8     | 480x1920   | 69x235 mm     |
| 11    | 440x1920   | 58x253 mm     |
| 9     | 462x1920   | 55x226 mm     |
| Y     | 480x1920   | 54x219 mm     |
| X     | 440x1920   | 58x253 mm     |
| Z     | 462x1920   | 55x226 mm     |

Other models attached to the same firmware family should also work; the panel
geometry is read live from the device.  If you have a model not in the table
above, the driver will fall back to the resolution reported by the device's
USB iProduct string and log a warning suggesting an entry be added.

Resolutions and dimensions come from various sources and existing JT365's driver
has been used as a fallback, for possibly missing specifics of some devices.
If you have one of these devices and see that this table is incorrect, please
make a issue, PR or contact me for correction.

## Features

- **DRM/KMS** display driver with shadow buffer and dirty-rect updates
- **Backlight class** support with software gamma curve
- **Rotation** via sysfs (8 orientations, mpro-style)
- **Suspend/resume** including USB autosuspend with idle pipe detection
- **Clock sync** — the device has an internal RTC that shows on its standby
  screen; the driver synchronises it to kernel UTC time at probe and on
  demand via sysfs
- **Configurable timezone** via module parameters (no DST rules in kernel)
- **Pixel format** RGB565 (BGR16 on the wire), labelled to match firmware
  expectations

## Requirements

### Kernel configuration

The following kernel options must be enabled:

- `CONFIG_USB` and `CONFIG_USB_SUPPORT`
- `CONFIG_DRM` and `CONFIG_DRM_KMS_HELPER`
- `CONFIG_BACKLIGHT_CLASS_DEVICE`
- `CONFIG_MFD_CORE`

These are enabled in virtually every distribution kernel.  If you build your
own kernel, ensure they are set to `=y` or `=m`.

### Build dependencies

- Kernel headers matching your running kernel
- GNU make
- A C compiler (gcc or clang)

On Alpine:

```sh
apk add linux-lts-dev make gcc musl-dev
```

On Debian/Ubuntu:

```sh
apt install linux-headers-$(uname -r) make gcc
```

On Fedora:

```sh
dnf install kernel-devel make gcc
```

### DKMS (optional, for automatic rebuilds across kernel updates)

On Alpine:

```sh
apk add dkms
```

On Debian/Ubuntu:

```sh
apt install dkms
```

On Fedora:

```sh
dnf install dkms
```

## Installation

### Option 1: Manual build and load

This is the quickest way to try the driver.  The modules are built against
your running kernel and loaded manually; they will not survive a kernel
upgrade.

```sh
git clone https://github.com/oskarirauta/beada-drm.git
cd beada
make
sudo make install
sudo depmod -a
```

Plug in the panel; the modules should load automatically via the USB device
ID table.  If they don't, load them manually:

```sh
sudo modprobe beadapanel
sudo modprobe beada-drm
sudo modprobe beada-bl

```

The `beada-drm` and `beada-bl` children are loaded automatically as
MFD cells of the parent.

### Option 2: DKMS

DKMS rebuilds the modules automatically whenever the kernel is updated.  This
is the recommended option for systems that receive regular kernel updates.

```sh
git clone https://github.com/oskarirauta/beada-drm.git /usr/src/beada-1.0
sudo dkms add -m beada -v 1.0
sudo dkms build -m beada -v 1.0
sudo dkms install -m beada -v 1.0
```

After installation the modules will be rebuilt automatically every time a new
kernel is installed, with no further action needed.

To uninstall:

```sh
sudo dkms remove -m beada -v 1.0 --all
sudo rm -rf /usr/src/beada-1.0
```

### Verifying installation

After plugging in the panel and waiting a moment, check that the driver is
loaded:

```sh
lsmod | grep beada
```

You should see `beadapanel`, `beada-drm`, and `beada-bl`.

Check the kernel log:

```sh
sudo dmesg | grep -i beada
```

Expected output (model and resolution depending on your panel):

```
beada-mfd 1-1.2:1.0: panel: BeadaPanel 3, 320x480 px
beada-mfd 1-1.2:1.0: panel: 40x62 mm, brightness_max=100
beada-mfd 1-1.2:1.0: BeadaPanel 3 ready
[drm] Initialized beada 1.0.0 for beada-drm.X.auto on minor 0
beada-drm beada-drm.X.auto: [drm] BeadaPanel 3 DRM driver ready (320x480, fbdev=off)
beada-backlight beada-backlight.X.auto: backlight ready: beada-1-1.2:1.0 (0..100, initial 88)
```

Verify the DRM device is exposed:

```sh
ls /dev/dri/
```

A new `cardN` device should appear.  Verify with:

```sh
ls /sys/class/drm/cardN-Unknown-*/
```

(Naming reflects the DRM connector type, which is "Unknown" for USB displays.)

Verify the backlight:

```sh
ls /sys/class/backlight/
cat /sys/class/backlight/beada-*/brightness
```

## Getting started

### Display an image with kmscube

The simplest test is the `kmscube` demo:

```sh
kmscube -D /dev/dri/cardN
```

You should see the rotating textured cube on the panel.

### Run a Wayland compositor

For Weston, add a backend entry to `weston.ini`:

```ini
[output]
name=DRM-1
mode=320x480
transform=normal
```

(Adjust `name` and `mode` to match your panel.)

Then run:

```sh
weston --tty=2
```

### Set brightness

The standard Linux backlight interface works:

```sh
echo 50 | sudo tee /sys/class/backlight/beada-*/brightness
```

Range is 0 to the device-reported maximum (typically 100, check
`/sys/class/backlight/beada-*/max_brightness`).

## Configuration

### Module parameters

Pass parameters at load time:

```sh
sudo modprobe beadapanel tz_offset=120
```

Or persist them in `/etc/modprobe.d/beada.conf`:

```
options beadapanel tz_offset=120 tz_dst=0
options beada-drm noblank=0
options beada-bl default_brightness=80 default_gamma=100
```

#### `beadapanel` parameters

- **`tz_offset`** (int, default 0)
  Local timezone offset from UTC in minutes.  Used when sending `SET_TIME`
  to the device.  Example: `120` for EET, `-300` for EST.

- **`tz_dst`** (bool, default 0)
  Add 60 minutes for daylight saving time.  The kernel does not handle DST
  rules, so toggle this twice a year as needed.  Can be changed at runtime
  via `/sys/module/beadapanel/parameters/tz_dst`.

- **`initial_brightness`** (int, default -1)
  Override the backlight value reported by the device at probe.  -1 means
  use whatever the device reports (typically the value from the previous
  session).  Set 0..max_brightness to start at a known level regardless of
  history.

- **`resume_handlers_delay_ms`** (uint, default 1200)
  Delay in milliseconds between USB resume completing and applying queued
  state (most notably brightness via `SL_SET_BL`).  The default works on
  tested firmware (7.19, T113); decrease for faster brightness restoration
  on faster hardware, increase if you see `SL_SET_BL` failing with -110
  (-ETIMEDOUT) in dmesg after resume.

#### `beada-drm` parameters

- **`noblank`** (bool, default 0)
  When 0 (default), `pipe_disable` sends a black frame to clear the display
  when the DRM pipe is closed (e.g. when the compositor exits).  Set to 1
  to leave the last rendered frame on the panel — useful for persistent
  display use cases.  Per-device override available via sysfs.

- **`enable_fbdev`** (bool, default 0)
  Opt-in fbdev emulation.  Off by default because modern userspace doesn't
  need it; enable if you use legacy fbcon or fbdev applications.

#### `beada-bl` parameters

- **`default_brightness`** (uint, default 100)
  Starting brightness value, 0..max_brightness.  Used only if the device
  did not report a current brightness.

- **`default_gamma`** (uint, default 100)
  Initial gamma curve value times 100.  Range 50..400 (= 0.5..4.0 in
  floating-point terms).  Higher values make the dim end darker.

### Sysfs attributes

#### Device info (`/sys/bus/usb/drivers/beadapanel/<device>/beadapanel/`)

Read-only:

- `model` — Model name (e.g. "3", "5C")
- `resolution` — `WIDTHxHEIGHT` in pixels
- `physical_size` — `WxH` in millimetres
- `firmware_version` — BCD firmware version
- `panellink_version`, `statuslink_version` — Protocol versions
- `platform` — SoC identifier (1=i.MX6UL, 2=i.MX6ULL, 4=V3S, 5=T113)
- `sn` — Serial number string
- `storage_size_kb` — On-board storage capacity
- `stats` — Pipeline statistics

Write-only:

- `sync_time` — Write any value to trigger an immediate `SET_TIME` to the
  device.  Useful after a system clock change.  Returns -EHOSTDOWN if the
  device is autosuspended; wake it briefly first.

#### Backlight (`/sys/class/backlight/beada-*/`)

Standard Linux backlight class attributes plus:

- `gamma_x100` — Read or write the gamma curve value × 100.  Range 50..400.
  Writes take effect immediately on the next brightness change.

Example: set gamma to 1.8 and brightness to 60:

```sh
echo 180 | sudo tee /sys/class/backlight/beada-*/gamma_x100
echo 60  | sudo tee /sys/class/backlight/beada-*/brightness
```

#### Rotation (`/sys/class/drm/cardN-Unknown-*/rotation`)

Read or write the panel rotation:

- `normal`
- `90` (rotated 90° clockwise)
- `180`
- `270`
- `flip-horizontal`
- `flip-vertical`
- `flip-horizontal-90`
- `flip-vertical-90`

Example:

```sh
echo 90 | sudo tee /sys/class/drm/card1-Unknown-1/rotation
```

The DRM mode and EDID are regenerated on rotation change.

### Setting the timezone persistently

For Finland (EET, UTC+2) with manual DST:

```sh
cat > /etc/modprobe.d/beada.conf << 'EOF'
# Winter time (EET = UTC+2)
options beadapanel tz_offset=120 tz_dst=0
EOF
```

For summer time, toggle the DST bit:

```sh
echo 1 | sudo tee /sys/module/beadapanel/parameters/tz_dst
echo 1 | sudo tee /sys/bus/usb/drivers/beadapanel/*/beadapanel/sync_time
```

## Suspend/resume behaviour

- **System suspend (S3)** is fully supported.  The screen blanks on suspend
  and the last image is restored on resume along with the user's brightness
  setting.

- **USB autosuspend** activates automatically when no DRM client has the
  panel open.  The device's internal firmware takes over and shows its
  built-in standby clock face.  Opening any DRM client (Weston, kmscube,
  etc.) wakes the device and restores the rendered image.

- **Brightness writes during autosuspend** are accepted silently; the value
  is stored in memory and pushed to hardware on the next resume.

- **Time sync during autosuspend** returns -EHOSTDOWN since updating the
  visible clock would require waking the device (which would replace the
  clock with a black frame).  Wake the device briefly first:

  ```sh
  echo on | sudo tee /sys/bus/usb/devices/<id>/power/control
  echo 1  | sudo tee /sys/bus/usb/drivers/beadapanel/*/beadapanel/sync_time
  echo auto | sudo tee /sys/bus/usb/devices/<id>/power/control
  ```

## Troubleshooting

### Driver doesn't load when panel is plugged in

Check that the device is recognised by USB:

```sh
lsusb | grep 4e58
```

If the panel appears but `lsmod | grep beada` shows nothing, try loading
manually:

```sh
sudo modprobe beadapanel
dmesg | tail -20
```

If `modprobe` fails with "module not found", the modules weren't installed
correctly.  Re-run `sudo make install` and `sudo depmod -a`.

### Image is shifted by ~135 pixels

This indicates the firmware was left in a "mid-frame" state by a previous
session.  The driver issues `PL_RESET` at probe to clear this; if you still
see the shift, try:

```sh
sudo rmmod beada-bl beada-drm beadapanel
sleep 2
sudo modprobe beadapanel
```

If the shift persists, physically unplug and replug the panel.

### Brightness doesn't restore after autosuspend

Check that `resume_handlers_delay_ms` is high enough.  Default 1200 ms is
sufficient on tested firmware (7.19); some hardware may need longer.
Increase it:

```sh
sudo rmmod beadapanel
sudo modprobe beadapanel resume_handlers_delay_ms=2000
```

Persist via `/etc/modprobe.d/beada.conf`.

### Compositor reports no displays

DRM device may not be visible to the user that the compositor runs as.
Check that the user is in the `video` group:

```sh
groups $USER
sudo usermod -aG video $USER
```

Then log out and back in.

### Kernel warning "cannot queue ... on wq"

This indicates a race during module unload.  It is harmless (a residual
delayed-work timer firing after the workqueue has been destroyed) and has
been worked around in driver versions 1.0+.  If you see this on a current
build, file a bug.

### Diagnostic commands

Show full driver state:

```sh
# Module info
modinfo beadapanel
modinfo beada-drm
modinfo beada-bl

# USB device state
BEADA=$(for d in /sys/bus/usb/devices/*/idVendor; do
  [ "$(cat $d 2>/dev/null)" = "4e58" ] && dirname $d
done | head -1)
echo "USB device: $BEADA"
cat $BEADA/power/runtime_status
cat $BEADA/power/runtime_usage
cat $BEADA/power/control
cat $BEADA/power/autosuspend_delay_ms

# Driver-specific
ls /sys/bus/usb/drivers/beadapanel/*/beadapanel/
ls /sys/class/backlight/beada-*/
ls /sys/class/drm/

# Kernel log
sudo dmesg | grep -i beada
```

## Known limitations

- **Single fixed resolution per probe session.**  The firmware latches the
  Panel-Link dimensions on the first `START` tag and refuses to accept a
  new one until the next `PL_RESET` (which the driver only issues at probe
  and on resume).  In practice this is not visible because the panel has
  one native mode anyway.

- **No EDID from hardware.**  The driver synthesises an EDID 1.3 block from
  the panel info table; the only real EDID would be a fake one anyway.

- **Backlight is software-curved.**  The device firmware accepts a linear
  0..100 value; the driver applies the configured gamma curve in software
  before sending.  This means the value reported by
  `/sys/class/backlight/beada-*/brightness` is the user-facing percentage,
  not the raw register value.

- **No DST handling.**  The kernel has no timezone database; the `tz_dst`
  parameter is a manual toggle.  Use a userspace cron job or systemd timer
  if you need automatic transitions.

- **Single-frame buffer for full-frame sends.**  The driver does not
  support partial display updates; every commit sends the entire
  framebuffer.  Bandwidth on USB 2.0 HS is the limit, which translates to
  about 45 fps at 800×480 and over 100 fps at smaller resolutions.

## Project structure

```
beada/
├── README.md                — this file
├── LICENSE                  — GPL-2.0-only
├── Makefile                 — top-level build
├── Kbuild                   — kernel build definitions
├── dkms.conf                — DKMS configuration
│
├── beada.h                  — public MFD ↔ child API
├── beada-priv.h             — private MFD-parent definitions
├── beada-drv.c              — USB probe / disconnect, module entry
├── beada-usb.c              — USB endpoint discovery, bulk I/O wrappers
├── beada-protocol.c         — Panel-Link / Status-Link wire helpers
├── beada-pipeline.c         — async transfer pipeline, public API
├── beada-pm.c               — USB suspend / resume, autopm
├── beada-model.c            — model table (geometry by ID and resolution)
├── beada-sysfs.c            — /sys/.../beada_panel/ attributes
│
├── drm/
│   ├── beada-drm.h              — DRM child private types
│   ├── beada-drm-drv.c          — DRM driver registration, probe / remove
│   ├── beada-drm-kms.c          — DRM mode setup
│   ├── beada-drm-conn.c         — DRM connector (synthesised EDID)
│   ├── beada-drm-edid.c         — EDID 1.3 builder
│   ├── beada-drm-plane.c        — pipe_update, blit + send
│   ├── beada-drm-color.c        — gamma LUT
│   ├── beada-drm-vblank.c       — vblank hrtimer + flip events
│   ├── beada-drm-sysfs.c        — rotation attribute
│   └── beada-drm-pm.c           — DRM resume handler
│
└── bl/
    ├── beada-backlight.h
    ├── beada-backlight-drv.c    — platform driver, probe / remove
    ├── beada-backlight-bl.c     — backlight ops, gamma application
    ├── beada-backlight-sysfs.c  — gamma_x100 attribute
    └── beada-backlight-pm.c     — backlight resume handler
```

## Credits

- **NXElec** for the Panel-Link and Status-Link protocol specifications and
  the reference daemon source.
- **JT365** for the `ub-6.11.0` reference kernel driver that served as the
  starting point for understanding the wire format.
- The Linux DRM, USB, MFD, and backlight subsystem maintainers for the
  excellent kernel infrastructure this driver builds on.

## License

GPL-2.0-only.  See [LICENSE](LICENSE).

## Author

Oskari Rauta <oskari.rauta@gmail.com>
