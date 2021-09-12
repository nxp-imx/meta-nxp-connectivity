if [ "$MACHINE" = "" ]; then
    MACHINE=imx8mmevk
fi
EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

echo "# layers for i.MX IoT for MATTER & OpenThread Broader Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-iot\"" >> conf/bblayers.conf
echo "IMAGE_INSTALL_append += \" boost boost-dev boost-staticdev \"" >> conf/local.conf

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo ""
