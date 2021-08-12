if [ "$MACHINE" = "" ]; then
    MACHINE=imx8mmevk
fi
EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

echo "tttt"
echo "# layers for i.MX IoT for MATTER & OpenThread Broader Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-iot\"" >> conf/bblayers.conf

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo ""
