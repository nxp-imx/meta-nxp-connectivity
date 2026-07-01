# Partition Tools

Tools for sizing the eMMC rootfs so a Matter image fills the target device
without overflowing it.

## imx-emmc-sizer.py

Computes `IMAGE_ROOTFS_SIZE` (KiB) from an eMMC capacity and prints the full
set of config changes needed to fit the image on the device.

### Why this exists

The old flow hardcoded `IMAGE_ROOTFS_SIZE = "6606000"` (~6.3 GiB) for every
board regardless of eMMC size, wasting most of a larger eMMC. Worse, two
independent 1.3x overhead multipliers inflate the final image:

1. bitbake `IMAGE_OVERHEAD_FACTOR` (default 1.3) scales the ext4 that
   `do_rootfs` produces.
2. wic's per-partition `--overhead-factor` (default 1.3, in the wks file)
   scales it again for any rootfs partition without an explicit size.

So `IMAGE_ROOTFS_SIZE` alone is not enough. To pin the rootfs to an exact
size you need all three of: `IMAGE_OVERHEAD_FACTOR="1"`,
`IMAGE_ROOTFS_EXTRA_SPACE="0"`, and a wks rootfs line with `--fixed-size`.
The tool emits all three, and the shipped wks already wires `--fixed-size`
to `${IMAGE_ROOTFS_SIZE}` so it tracks the value.

## Quick start

### 1. Get the target eMMC capacity (exact mode, recommended)

On the running board:

```sh
echo $(( $(cat /sys/class/block/mmcblk0/size) * 512 ))
```

This prints the exact user-area byte count, e.g. `31037849600` for a
nominal 32G eMMC.

### 2. Preview the sizing (dry-run, writes nothing)

```sh
cd sources/meta-nxp-connectivity
python3 tools/imx-emmc-sizer.py --exact-bytes 31037849600
```

This prints the partition layout and the three-part config recipe:

```
Required config (all three are needed to fit the eMMC):

1) local.conf:
     IMAGE_ROOTFS_SIZE        = "30035968"
     IMAGE_OVERHEAD_FACTOR    = "1"
     IMAGE_ROOTFS_EXTRA_SPACE = "0"

2) wks rootfs partition -- add --fixed-size:
     part / --source rootfs --ondisk mmcblk --fstype=ext4 \
            --label root --align 8192 --fixed-size 30035968k
```

### 3. Apply the local.conf variables automatically

```sh
python3 tools/imx-emmc-sizer.py --exact-bytes 31037849600 \
    --apply ../../bld-xwayland-imx93evk-iwxxx-matter/conf/local.conf
```

`--apply` backs up the file to `local.conf.bak` first, then writes/updates
all three variables. Adjust the path to your build directory.

### 4. The wks tracks IMAGE_ROOTFS_SIZE

The wks rootfs line uses `--fixed-size ${IMAGE_ROOTFS_SIZE}k`, which wic
expands from the value you set in local.conf, so you do not edit the wks
per capacity. Step 3 (`--apply`) is the only change when switching to a
differently-sized eMMC.

The wks lives at:

```
meta-nxp-matter-advanced/files/wic/imx-matter-emmc-fixed.wks.in
```

The machine conf already selects it via `WKS_FILE` for
imx93evk-iwxxx-matter; other machines need the same
`WKS_FILE = "imx-matter-emmc-fixed.wks.in"` line added.

### 5. Build and verify

```sh
bitbake imx-image-multimedia
```

Check the built image fits the eMMC (should be <= your capacity):

```sh
grep ImageSize tmp/deploy/images/<machine>/imx-image-multimedia-*.wic.bmap
```

## Nominal mode (when you only know the marketing size)

If you don't have the board handy and only know it's "a 32G eMMC", use
nominal mode. It treats the size as vendor decimal (32G = 32 x 10^9 bytes)
and applies a 0.92 safety factor to leave margin for the real usable area:

```sh
python3 tools/imx-emmc-sizer.py --nominal 16G
```

Prefer `--exact-bytes` whenever you can read the device — it is exact and
skips the safety factor.

## On-device verification

After flashing, confirm the rootfs fills the eMMC:

```sh
df -h /                                  # Size should be ~28G, not ~6.2G
cat /sys/class/block/mmcblk0p2/size      # sectors; x512 = fixed-size bytes
```

For a 31,037,849,600-byte eMMC the expected rootfs partition is
30,756,831,232 bytes (= 30,035,968 KiB x 1024).

## Constants (tunable at the top of the script)

| Constant | Value | Meaning |
|---|---|---|
| `SAFETY_FACTOR` | 0.92 | nominal-mode margin (exact mode ignores it) |
| `FIXED_OVERHEAD_MIB` | 268 | imx-boot 32K + gap 8M + boot 256M + align 4M |
| `ALIGN_MIB` | 4 | rootfs floor-aligned to this |
| `MIN_ROOTFS_GIB` | 2 | fatal error if computed rootfs is smaller |

## Self-test

```sh
python3 tools/imx-emmc-sizer.py --self-test   # prints "self-test: PASS"
```

## Scope

Targets the imx93 layout (boot 256 MiB, IMX_BOOT_SEEK 32 KiB). The numeric
constants are kept together at the top of the script for other SoCs.

## Sizing at setup time (imx-matter-setup.sh)

`imx-matter-setup.sh` accepts two optional environment variables that size the
rootfs to your target eMMC automatically (it runs `imx-emmc-sizer.py --apply`
on the new build's `conf/local.conf`):

| Variable | Example | Meaning |
|---|---|---|
| `EMMC_BYTES` | `31037849600` | Exact user-area size in bytes (`--exact-bytes`). Wins if both are set. |
| `EMMC_NOMINAL` | `32G` | Vendor nominal capacity (`--nominal`, decimal, x0.92). |

```sh
# Fill an exact eMMC
EMMC_BYTES=31037849600 MACHINE=imx93evk-iwxxx-matter DISTRO=fsl-imx-xwayland \
    EULA=1 source tools/imx-matter-setup.sh bld-xwayland-imx93evk-iwxxx-matter
```

If neither variable is set, the setup script writes no rootfs config and the
build falls back to the BSP upstream default (a compact, content-sized rootfs
that fits any device). If the sizer fails (e.g. the capacity is below the
minimum guard), setup prints a warning and continues with that default.
