if [ ! -n "$MACHINE" ]; then
    MACHINE=imx8mmevk
fi
if [ ! -n "$TARGET_15_4_CHIP" ]; then
    TARGET_15_4_CHIP=""
fi
if [ ! -n "$OT_RCP_BUS" ]; then
    OT_RCP_BUS=UART
fi
echo "MACHINE = $MACHINE"
echo "TARGET_15_4_CHIP = $TARGET_15_4_CHIP"
echo "OT_RCP_BUS = $OT_RCP_BUS"

cd sources/meta-matter
git rev-list --header @{u}..|grep -q "OT: commit option"
meta_matter_iw612_patched=$?

reset_meta_matter=0
if [ "$TARGET_15_4_CHIP" = "IW612" ] && [ "$MACHINE" = "imx8mmevk" ]; then
    if [ "$meta_matter_iw612_patched" -eq 0 ] ; then reset_meta_matter=0; else reset_meta_matter=1; fi
else
    if [ "$meta_matter_iw612_patched" -eq 1 ] ; then reset_meta_matter=1; else reset_meta_matter=0; fi
fi
cd ../..

if [ "$reset_meta_matter" -eq 1 ]; then
    echo "Clean and Reset sources/meta-matter"
    cd sources/meta-matter
    modified_files=$(git status --porcelain|wc -m)
    echo $modified_files
    if [ $modified_files -gt 0 ]; then
        echo "Warning: you have modified or untracked files in meta-matter layer"
        echo "         Save them or stash them before to run this command"
        return
    fi
    git clean -dfx
    git reset --hard imx_matter_2023_q1
    cd ../..
else
    echo "No Clean/Reset on sources/meta-matter"
fi

cd sources/meta-imx
ahead_commits=$((($(git rev-list --count HEAD)-$(git rev-list --count nxp-imx/kirkstone-5.15.71-2.2.0))))
if [ "$ahead_commits" -gt 0 ]; then
    echo "Clean and Reset sources/meta-imx"
    git clean -dfx
    git reset --hard HEAD~$ahead_commits
else
    echo "No Clean/Reset on sources/meta-imx"
fi
cd ../..

if [ "$TARGET_15_4_CHIP" = "IW612" ] && [ "$MACHINE" = "imx8mmevk" ]; then
	echo "TARGET_15_4_CHIP is $TARGET_15_4_CHIP"
	echo "Applying 0001-dtb-and-network-kernel-config-for-IW612.patch -> meta-bsp/recipes-kernel/linux"
	cd sources/meta-imx/
	true | git apply --3way ../meta-matter/tools/patches/0001-dtb-and-network-kernel-config-for-IW612.patch --check

	if [ $? == 0 ]; then
		git am -3 ../meta-matter/tools/patches/0001-dtb-and-network-kernel-config-for-IW612.patch
	fi
	if [ "$reset_meta_matter" -eq 1 ]; then
        cd ../../sources/meta-matter/
        echo "Applying 0001-OT-commit-options-and-patches-for-IW612.patch -> recipes-openthread/openthread"
        true | git apply --3way tools/patches/0001-OT-commit-options-and-patches-for-IW612.patch --check

        if [ $? == 0 ]; then
            git am -3 tools/patches/0001-OT-commit-options-and-patches-for-IW612.patch
        fi
        echo "Applying 0001-OTBR-commit-options-and-patches-for-IW612.patch -> recipes-otbr/otbr"
        true | git apply --3way tools/patches/0001-OTBR-commit-options-and-patches-for-IW612.patch --check

        if [ $? == 0 ]; then
            git am -3 tools/patches/0001-OTBR-commit-options-and-patches-for-IW612.patch
        fi
    fi
	cd ../..
else
	cd sources/meta-imx/
	true | git apply --3way ../meta-matter/tools/patches/0001-Apply-Matter-enhancement-change-for-linux-imx.patch --check

	if [ $? == 0 ]; then
		git am -3 ../meta-matter/tools/patches/0001-Apply-Matter-enhancement-change-for-linux-imx.patch
		git am -3 ../meta-matter/tools/patches/0002-modified-imx93-bsp-for-supporting-for-Matter-functio.patch
        git am -3 ../meta-matter/tools/patches/0001-Add-matter-and-Trusty-enabled-build-target-for-imx8m.patch
	fi
	cd ../..
fi

EULA=$EULA DISTRO=$DISTRO MACHINE=$MACHINE . ./imx-setup-release.sh -b $@

if [ "$OT_RCP_BUS" = "SPI" ]; then
    echo "OT_RCP_BUS = \"SPI\"" >> conf/local.conf
fi
if [ "$OT_RCP_BUS" = "UART" ]; then
    echo "OT_RCP_BUS = \"UART\"" >> conf/local.conf
fi
 
echo "# layers for i.MX IoT for MATTER & OpenThread Border Router" >> conf/bblayers.conf
echo "BBLAYERS += \"\${BSPDIR}/sources/meta-matter\"" >> conf/bblayers.conf
echo "IMAGE_INSTALL:append = \" boost boost-dev boost-staticdev \"" >> conf/local.conf
echo "PACKAGECONFIG:append:pn-iptables = \" libnftnl\"" >> conf/local.conf

echo ""
echo "Now you can use below command to generate your image:"
echo "    $ bitbake imx-image-core"
echo "             or "
echo "    $ bitbake imx-image-multimedia"
echo "======================================================="
echo "If you want to generate SDK, please use:"
echo "    $ bitbake imx-image-core -c populate_sdk"
echo ""
