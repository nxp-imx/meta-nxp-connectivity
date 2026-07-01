#!/usr/bin/env python3
"""Compute IMAGE_ROOTFS_SIZE (KiB) from an eMMC capacity.

Assumes the imx93 layout: imx-boot raw at IMX_BOOT_SEEK, a fixed 256 MiB
vfat boot partition, and an ext4 rootfs filling the remainder. Numeric
constants are kept together at the top for other SoCs.
"""

import argparse
import re
import shutil
import sys

KIB = 1024
MIB = 1024 * 1024
GIB = 1024 * 1024 * 1024

SAFETY_FACTOR = 0.92        # overprovision + wear + bad-block headroom
FIXED_OVERHEAD_MIB = 268    # imx-boot 32K + gap 8M + boot 256M + align 4M
ALIGN_MIB = 4               # rootfs floor-aligned to this
MIN_ROOTFS_GIB = 2          # below this, capacity is too small -> error


def align_down(value_bytes, align_bytes):
    """Return the largest multiple of align_bytes that is <= value_bytes."""
    return (value_bytes // align_bytes) * align_bytes


def parse_nominal(text):
    """Parse vendor decimal capacity ('8G','16G','512M') into bytes (base 10)."""
    s = text.strip().upper()
    multipliers = {"G": 10**9, "M": 10**6}
    if not s or s[-1] not in multipliers:
        raise ValueError(
            "nominal must end in G or M (vendor decimal), e.g. 8G; got %r" % text
        )
    num = s[:-1]
    if not num.replace(".", "", 1).isdigit():
        raise ValueError("nominal numeric part invalid: %r" % text)
    return int(float(num) * multipliers[s[-1]])


def compute_rootfs_kib(capacity_bytes, apply_safety):
    """Compute IMAGE_ROOTFS_SIZE in KiB from total eMMC bytes.

    apply_safety=True  -> nominal mode (multiply by SAFETY_FACTOR).
    apply_safety=False -> exact mode (use real bytes, no safety factor).
    """
    usable = int(capacity_bytes * SAFETY_FACTOR) if apply_safety else int(capacity_bytes)
    rootfs_b = usable - FIXED_OVERHEAD_MIB * MIB
    rootfs_b = align_down(rootfs_b, ALIGN_MIB * MIB)
    if rootfs_b < MIN_ROOTFS_GIB * GIB:
        raise ValueError(
            "computed rootfs %.2f GiB < minimum %d GiB; eMMC capacity too small"
            % (rootfs_b / GIB, MIN_ROOTFS_GIB)
        )
    return rootfs_b // KIB


def format_bill(input_desc, capacity_bytes, apply_safety, rootfs_kib):
    """Build the human-readable partition bill."""
    usable = int(capacity_bytes * SAFETY_FACTOR) if apply_safety else int(capacity_bytes)
    sf_line = (
        "Safety factor      : %.2f        -> %s B usable" % (SAFETY_FACTOR, f"{usable:,}")
        if apply_safety
        else "Safety factor      : (exact mode, none) -> %s B usable" % f"{usable:,}"
    )
    return "\n".join([
        "=== i.MX eMMC Sizer ===",
        "Machine assumption : imx93 (boot=256MiB, IMX_BOOT_SEEK=32KiB)",
        "Input              : %s  -> %s B" % (input_desc, f"{capacity_bytes:,}"),
        sf_line,
        "Fixed overhead     : %d MiB     (imx-boot 32K + gap 8M + boot 256M + align 4M)"
        % FIXED_OVERHEAD_MIB,
        "-" * 50,
        "Partition layout:",
        "  imx-boot      offset 32 KiB     (raw)",
        "  boot          256 MiB           (vfat, fixed)",
        "  rootfs        %s KiB     (ext4, fills rest)" % f"{rootfs_kib:,}",
        "-" * 50,
        "Required config (all three are needed to fit the eMMC):",
        "",
        "1) local.conf  -- make do_rootfs produce a tight ext4 (no 1.3x):",
        '     IMAGE_ROOTFS_SIZE       = "%d"' % rootfs_kib,
        '     IMAGE_OVERHEAD_FACTOR   = "1"',
        '     IMAGE_ROOTFS_EXTRA_SPACE = "0"',
        "",
        "2) wks rootfs partition -- already wired in the shipped wks",
        "   (imx-matter-emmc-fixed.wks.in) as --fixed-size ${IMAGE_ROOTFS_SIZE}k,",
        "   so it tracks the value above. The rootfs line resolves to:",
        "     part / --source rootfs --ondisk mmcblk --fstype=ext4 \\",
        "            --label root --align 8192 --fixed-size %dk" % rootfs_kib,
        "",
        "   (--fixed-size bypasses wic's per-partition overhead-factor, which",
        "    defaults to 1.3 and must be >1.0 so it cannot be set to 1. Without",
        "    it, wic multiplies the ext4 size by 1.3 and overflows the eMMC.)",
    ])


def _set_var(content, name, value):
    """Replace or append a single `name = "value"` assignment. Returns (content, action)."""
    new_line = '%s = "%s"' % (name, value)
    pattern = re.compile(r"^\s*%s\s*=.*$" % re.escape(name), re.MULTILINE)
    if pattern.search(content):
        return pattern.sub(new_line, content), "replaced"
    if content and not content.endswith("\n"):
        content += "\n"
    return content + new_line + "\n", "appended"


def apply_to_local_conf(path, rootfs_kib):
    """Write the eMMC-fit config into local.conf; backup first. Returns status.

    Writes the three variables that make IMAGE_ROOTFS_SIZE the exact rootfs
    size: IMAGE_ROOTFS_SIZE, IMAGE_OVERHEAD_FACTOR=1, IMAGE_ROOTFS_EXTRA_SPACE=0.
    The wks tracks IMAGE_ROOTFS_SIZE via ${IMAGE_ROOTFS_SIZE}k, so it needs no
    separate edit.
    """
    with open(path, "r") as f:
        content = f.read()
    shutil.copyfile(path, path + ".bak")
    actions = []
    for name, value in (
        ("IMAGE_ROOTFS_SIZE", rootfs_kib),
        ("IMAGE_OVERHEAD_FACTOR", 1),
        ("IMAGE_ROOTFS_EXTRA_SPACE", 0),
    ):
        content, action = _set_var(content, name, value)
        actions.append("%s %s" % (action, name))
    with open(path, "w") as f:
        f.write(content)
    return "%s in %s (backup: %s.bak)" % ("; ".join(actions), path, path)


def run_self_test():
    """Built-in assertions covering the core arithmetic."""
    # 8G nominal -> known worked example
    assert compute_rootfs_kib(8_000_000_000, True) == 6_909_952
    # 16G nominal -> sane and below usable bytes
    k16 = compute_rootfs_kib(16_000_000_000, True)
    assert k16 * KIB < 16_000_000_000
    assert k16 > compute_rootfs_kib(8_000_000_000, True)
    # exact mode skips the 0.92 factor
    cap = 7_818_182_656
    exact = compute_rootfs_kib(cap, False)
    expected = align_down(cap - FIXED_OVERHEAD_MIB * MIB, ALIGN_MIB * MIB) // KIB
    assert exact == expected
    # boundary: 1G nominal trips the guard
    tripped = False
    try:
        compute_rootfs_kib(1_000_000_000, True)
    except ValueError:
        tripped = True
    assert tripped, "1G should trip MIN_ROOTFS guard"
    # alignment: result is an integer multiple of ALIGN_MIB
    assert (compute_rootfs_kib(8_000_000_000, True) * KIB) % (ALIGN_MIB * MIB) == 0
    # parse_nominal sanity
    assert parse_nominal("8G") == 8_000_000_000
    # _set_var: replace existing, append missing
    c, a = _set_var('IMAGE_ROOTFS_SIZE = "5"\n', "IMAGE_ROOTFS_SIZE", 9)
    assert a == "replaced" and 'IMAGE_ROOTFS_SIZE = "9"' in c
    c, a = _set_var("FOO = \"1\"\n", "IMAGE_OVERHEAD_FACTOR", 1)
    assert a == "appended" and 'IMAGE_OVERHEAD_FACTOR = "1"' in c
    print("self-test: PASS")
    return 0


def build_parser():
    p = argparse.ArgumentParser(
        description="Compute IMAGE_ROOTFS_SIZE (KiB) from eMMC capacity (imx93 layout).",
    )
    g = p.add_mutually_exclusive_group()
    g.add_argument("--nominal", metavar="SIZE",
                   help="vendor decimal capacity, e.g. 8G, 16G")
    g.add_argument("--exact-bytes", type=int, metavar="N",
                   help="precise byte count (e.g. mmcblk0/size x 512)")
    p.add_argument("--apply", metavar="PATH",
                   help="write IMAGE_ROOTFS_SIZE into the given local.conf")
    p.add_argument("--self-test", action="store_true",
                   help="run built-in assertions and exit")
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    if args.self_test:
        return run_self_test()
    try:
        if args.nominal:
            cap = parse_nominal(args.nominal)
            apply_safety = True
            desc = "nominal %s" % args.nominal
        elif args.exact_bytes is not None:
            cap = args.exact_bytes
            apply_safety = False
            desc = "exact-bytes %d" % args.exact_bytes
        else:
            print("error: one of --nominal / --exact-bytes / --self-test is required",
                  file=sys.stderr)
            return 2
        rootfs_kib = compute_rootfs_kib(cap, apply_safety)
    except ValueError as e:
        print("error: %s" % e, file=sys.stderr)
        return 2
    print(format_bill(desc, cap, apply_safety, rootfs_kib))
    if args.apply:
        try:
            print("\n" + apply_to_local_conf(args.apply, rootfs_kib))
        except OSError as e:
            print("error: cannot apply: %s" % e, file=sys.stderr)
            return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
