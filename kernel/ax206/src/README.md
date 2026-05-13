# ax206 — AX206 Digital Photo Frame Linux Kernel Driver

Linux DRM/KMS driver for digital photo frames (DPF) based on the AX206
chip, commonly found in rebranded USB "hacked" photo frames.

The device communicates over USB using a SCSI-over-USB (Bulk-Only
Transport) protocol with proprietary AX206 SCSI command extensions.

**Author:** Oskari Rauta <oskari.rauta@gmail.com>
**Repository:** https://github.com/oskarirauta/ax206
**License:** GPL-2.0-only
**Kernel:** Linux 6.18+

---

## Hardware

- USB VID/PID: `0x1908:0x0102` (AX206)
- Display: RGB565 big-endian, typically 320×240 or 240×320
- Backlight: 8 levels (0–7)
- Protocol: SCSI-over-USB bulk transfers

The AX206 chip is found in many rebranded digital photo frames.
Protocol documentation: http://dpf-ax.sourceforge.net/
Additional reference: https://github.com/dreamlayers/dpf-ax

---

## Driver structure

```
ax206/
├── Kbuild                    # MFD parent build rules
├── Makefile                  # Out-of-tree build
├── ax206.h                   # Public API (shared with child drivers)
├── ax206-priv.h              # Private API (parent-internal only)
├── ax206-drv.c               # USB probe/disconnect, MFD cell registration
├── ax206-usb.c               # SCSI-over-USB protocol implementation
├── bl/
│   ├── Kbuild
│   ├── ax206-bl.h            # Backlight child header
│   └── ax206-bl-drv.c        # backlight_device registration
├── drm/
│   ├── Kbuild
│   ├── ax206-drm-priv.h      # DRM child private shared header
│   ├── ax206-drm-drv.c       # DRM probe/remove, KMS initialisation
│   ├── ax206-drm-plane.c     # Display pipe, shadow buffer, coalescing blit
│   ├── ax206-drm-rotation.c  # 8-mode pixel rotation mapping
│   ├── ax206-drm-pm.c        # Virtual idle/power-save timer
│   ├── ax206-drm-stats.c     # Frame counters and EWMA FPS
│   └── ax206-drm-sysfs.c     # sysfs attribute interface
└── tests/
    ├── Makefile
    └── ax206-test.c          # Userspace DRM draw test utility
```

### Architecture

```
USB device (AX206)
    │
    ▼
ax206.ko  (MFD parent)
    ├── ax206_dev context (udev, dimensions, workqueue, mutex)
    ├── USB workqueue "ax206_usb" — all post-probe I/O
    │
    ├── ax206-bl.ko  (backlight child)
    │       └── backlight_device "ax206-bl"  (0–7)
    │
    └── ax206-drm.ko  (DRM child)
            ├── drm_simple_display_pipe
            ├── Shadow buffer (big-endian RGB565, physical dims)
            ├── Coalescing blit engine (5 ms delay, single work item)
            ├── Dirty-rectangle partial update optimisation
            ├── 8-mode rotation (4 angles × reflect-x)
            ├── Virtual idle timer → blank / resume
            └── sysfs attributes
```

### Pixel format note

The AX206 device expects big-endian RGB565 (high byte first, R bits
first).  Standard DRM uses little-endian RGB565.  The driver applies
`swab16()` per pixel in the flush path; userspace applications write
standard little-endian RGB565 as normal.  XRGB8888 input is also
accepted and converted to RGB565 internally.

---

## Building (out-of-tree)

```sh
# Install kernel headers if needed
apt install linux-headers-$(uname -r)   # Debian/Ubuntu
apk add linux-headers                   # Alpine

# Build
make

# Install
sudo make modules_install
sudo depmod -a

# Load
sudo modprobe ax206
sudo modprobe ax206-drm
sudo modprobe ax206-bl
```

---

## Module parameters / kernel command line

Parameters belong to the `ax206-drm` module.  When set on the kernel
command line, prefix with `ax206_drm.`:

```
ax206_drm.rotation=2 ax206_drm.noblank=1
```

When set via `/etc/modprobe.d/ax206.conf`:
```
options ax206-drm rotation=2 noblank=1
```

| Parameter       | Type   | Default | Description                                      |
|-----------------|--------|---------|--------------------------------------------------|
| `rotation`      | int    | 0       | Initial rotation mode (see Rotation table)       |
| `noblank`       | bool   | 0       | Keep last image when DRM pipe closes             |
| `fbcon`         | bool   | 0       | Enable fbcon (useful for testing)                |
| `idle_timeout`  | uint   | 0       | Virtual idle timeout in seconds (0 = disabled)   |

---

## Rotation modes

| Value | Description              |
|-------|--------------------------|
| 0     | 0° (natural)             |
| 1     | 90° clockwise            |
| 2     | 180°                     |
| 3     | 270° clockwise           |
| 4     | 0° + horizontal flip     |
| 5     | 90° + horizontal flip    |
| 6     | 180° + horizontal flip   |
| 7     | 270° + horizontal flip   |

Rotation can also be changed at runtime via sysfs.

---

## sysfs interface

Attributes are under the DRM platform device, typically at
`/sys/bus/platform/devices/ax206-drm.N/`.

### Display attributes

| Attribute      | Access | Description                                              |
|----------------|--------|----------------------------------------------------------|
| `rotation`     | R/W    | Current rotation mode (0–7); takes effect on next frame |
| `noblank`      | R/W    | `1` = keep image on pipe close; `0` = blank to black   |
| `stats`        | R      | Frame statistics (see below)                            |
| `reset_stats`  | W      | Write any value to reset all counters                   |

### Virtual power management (`virtual_power/` subdirectory)

Emulates real PM semantics for tooling compatibility.

| Attribute                      | Access | Description                                            |
|--------------------------------|--------|--------------------------------------------------------|
| `virtual_power/control`        | R/W    | `"auto"` = idle timer on; `"on"` = always on         |
| `virtual_power/autosuspend_delay_ms` | R/W | Idle timeout in ms (0 = disabled)              |
| `virtual_power/runtime_status` | R      | `"active"` or `"suspended"`                          |

### Statistics format (`stats` attribute)

```
received:  42        # frames handed to the flush path
drawn:     38        # frames actually sent over USB
skipped:   4         # frames dropped (unchanged or blanked)
errors:    0         # USB errors during blit
fps_ewma:  1.250     # EWMA frames per second
```

---

## Virtual idle / power management

The AX206 has no hardware suspend capability.  Power management is
implemented in software with an idle timer:

- After `idle_timeout` seconds with no frame update the display is
  blanked: backlight set to 0, screen filled black.
- New framebuffer data continues to arrive into the shadow buffer
  but is not sent to USB while blanked.  Backlight requests are
  stored but not applied.
- The display resumes automatically on the next frame update, or
  immediately when `virtual_power/control` is written to `"on"`.
- On resume, the shadow buffer is blitted to the display and the
  stored backlight level is restored.
- `idle_timeout = 0` (default) disables the timer entirely.

---

## Performance / update rate

USB bulk throughput limits practical update rates to roughly 1–10 fps
depending on changed area.  The driver applies several optimisations:

- **Dirty-rectangle tracking**: only changed pixels are sent.  A
  full-screen update sends up to `320×240×2 = 150 KiB`; a small
  widget update sends only the bounding rectangle of changed pixels.
- **Coalescing blit engine**: a single persistent work item handles
  all USB transfers.  Before each transfer it waits 5 ms, allowing
  multiple rapid commits to merge into one USB payload.  After the
  transfer it loops if new data arrived during the send, ensuring
  the display always converges to the latest state.
- **Shadow buffer comparison**: unchanged pixels are never
  re-transmitted, even across framebuffer format changes.

The display is best suited for static images, slow animations, and
status displays.  Interactive use (compositors, terminals) is possible
but fps will be low.  kmscube works and is useful for verifying the
driver, though the low fps makes it a poor demo.

---

## Test utility

```sh
cd tests
make
./ax206-test              # auto-detects first ax206 DRM device
./ax206-test /dev/dri/cardN

# Build options
gcc -O2 -Wall -I/usr/include/libdrm -o ax206-test ax206-test.c -ldrm
```

Press **Enter** for the next test pattern, **q** to quit.

---

## Notes

- **No fbcon by default.** The display is slow and not suitable for
  interactive terminal use.  Enable with `fbcon=1` for testing.
- **No real suspend/resume.** Hardware PM is not available; virtual
  idle timer is software-only.
- **No fbcon.** fbcon is not enabled by default; use `fbcon=1` module
  parameter when needed for testing.
