if [ ! -n "$MACHINE" ]; then
    MACHINE=imx8mmevk
fi
echo "MACHINE = $MACHINE"

EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

echo "# layers for i.MX MATTER & OpenThread Border Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-baseline\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-advanced\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-openthread\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-otbr\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-connectivity-examples\"" >> conf/bblayers.conf

sed '/meta-matter/d' "conf/bblayers.conf" > temp && mv temp "conf/bblayers.conf"

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo ""
