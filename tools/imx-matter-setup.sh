if [ "$MACHINE" = "" ]; then
    MACHINE=imx8mmevk
fi

cd sources/meta-imx/
true | git apply ../meta-matter/tools/patches/0001-Apply-Matter-enhancement-change-for-linux-imx.patch --check

if [ $? == 0 ]; then
    git am -3 ../meta-matter/tools/patches/0001-Apply-Matter-enhancement-change-for-linux-imx.patch
    git am -3 ../meta-matter/tools/patches/0002-modified-imx93-bsp-for-supporting-for-Matter-functio.patch
fi
cd ../..

EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

if [ -n "$MATTER_OTBR_SPI" ]; then
  if ! grep -q "MATTER_OTBR_SPI = "true"" conf/local.conf; then
    echo "MATTER_OTBR_SPI = "true"" >> conf/local.conf
  fi
else
  if grep -q "MATTER_OTBR_SPI = "true"" conf/local.conf; then
    sed -i '/MATTER_OTBR_SPI = "true"/d' conf/local.conf
  fi
fi

echo "# layers for i.MX IoT for MATTER & OpenThread Broader Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-matter\"" >> conf/bblayers.conf
echo "IMAGE_INSTALL:append += \" boost boost-dev boost-staticdev \"" >> conf/local.conf
echo "PACKAGECONFIG:append:pn-iptables += \" libnftnl\"" >> conf/local.conf

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo ""
