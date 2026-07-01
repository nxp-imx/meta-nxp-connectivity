if [ ! -n "$MACHINE" ]; then
    MACHINE=imx8mmevk
fi
echo "MACHINE = $MACHINE"

# >>> emmc-sizing >>>
# Optionally size the rootfs to the target eMMC. Reads EMMC_BYTES (exact,
# wins) or EMMC_NOMINAL (e.g. 32G). If neither is set, writes nothing and the
# build falls back to the BSP default rootfs sizing. If the sizer is missing
# or fails (e.g. capacity below the minimum guard), warn and continue so the
# sourced shell is never killed.
apply_emmc_sizing() {
    # Declare locals so sourcing this script does not leak these names into
    # the caller's interactive shell.
    local _sizer _conf _mode _val
    _sizer="$1"
    _conf="$2"
    if [ -n "${EMMC_BYTES:-}" ]; then
        _mode="--exact-bytes"; _val="$EMMC_BYTES"
    elif [ -n "${EMMC_NOMINAL:-}" ]; then
        _mode="--nominal"; _val="$EMMC_NOMINAL"
    else
        return 0
    fi
    if [ -x "$_sizer" ] && python3 "$_sizer" "$_mode" "$_val" --apply "$_conf"; then
        echo "eMMC sizing applied to $_conf (backup: ${_conf}.bak)."
    else
        echo "WARNING: eMMC sizing failed ($_mode $_val); leaving rootfs at BSP default." >&2
    fi
    return 0
}
# <<< emmc-sizing <<<

# Resolve this script's directory so the sizer path works after the build
# dir cd performed by imx-setup-release.sh below.
_SETUP_SRC="${BASH_SOURCE[0]:-$0}"
_SIZER="$(cd "$(dirname "$_SETUP_SRC")" && pwd)/imx-emmc-sizer.py"

EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

sed '/meta-nxp-connectivity/d' "conf/bblayers.conf" > temp && mv temp "conf/bblayers.conf"

echo "# layers for i.MX MATTER & OpenThread Border Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-baseline\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-advanced\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-openthread\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-otbr\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-connectivity-examples\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-zigbee-rcp\"" >> conf/bblayers.conf

# Size the rootfs from EMMC_BYTES / EMMC_NOMINAL if set; otherwise leave the
# BSP default.
apply_emmc_sizing "$_SIZER" "conf/local.conf"

# Drop the helper variables so sourcing leaves a clean caller environment.
unset _SETUP_SRC _SIZER

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo ""
