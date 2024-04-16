if [ ! -n "$MACHINE" ]; then
    MACHINE=imx8mmevk
fi
if [ ! -n "$OT_RCP_BUS" ]; then
    OT_RCP_BUS=UART
fi
echo "MACHINE = $MACHINE"
echo "OT_RCP_BUS = $OT_RCP_BUS"

EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

if [ "$OT_RCP_BUS" = "SPI" ]; then
    echo "OT_RCP_BUS = \"SPI\"" >> conf/local.conf
fi
if [ "$OT_RCP_BUS" = "UART" ]; then
    echo "OT_RCP_BUS = \"UART\"" >> conf/local.conf
fi
 
echo "# layers for i.MX IoT for MATTER & OpenThread Border Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-baseline\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-matter-advanced\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-openthread\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-otbr\"" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-nxp-connectivity/meta-nxp-connectivity-examples\"" >> conf/bblayers.conf

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo "To exit Matter build environment, please:"
echo "    $ deactivate"
echo ""
