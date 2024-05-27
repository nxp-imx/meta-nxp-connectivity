FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPLOY_TRUSTY = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', 'true', 'false', d)}"

trusty_configure() {
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64-trusty --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" build_without_pw=true chip_with_trusty_os=1 enable_exceptions=true chip_code_pre_generated_directory="${S}/zzz_pregencodes"

        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'
}

do_configure:append() {
    cd ${S}/examples/nxp-media-app/linux
    common_configure

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        trusty_configure

        cd ${S}/examples/chip-tool
        trusty_configure

        cd ${S}/examples/nxp-thermostat/linux
        trusty_configure

        cd ${S}/examples/nxp-media-app/linux
        trusty_configure
    fi
}

do_compile:append() {

    cd ${S}/examples/nxp-media-app/linux
    ninja -C out/aarch64

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        ninja -C out/aarch64-trusty

        cd ${S}/examples/nxp-thermostat/linux
        ninja -C out/aarch64-trusty

        cd ${S}/examples/nxp-media-app/linux
        ninja -C out/aarch64-trusty

        cd ${S}/examples/chip-tool
        ninja -C out/aarch64-trusty
    fi
}

do_install:append() {
    install ${S}/examples/nxp-media-app/linux/out/aarch64/nxp-media-app ${D}${bindir}

    if ${DEPLOY_TRUSTY}; then
        install ${S}/examples/lighting-app/linux/out/aarch64-trusty/chip-lighting-app ${D}${bindir}/chip-lighting-app-trusty
        install ${S}/examples/nxp-thermostat/linux/out/aarch64-trusty/nxp-thermostat-app ${D}${bindir}/nxp-thermostat-app-trusty
        install ${S}/examples/nxp-media-app/linux/out/aarch64-trusty/nxp-media-app ${D}${bindir}/nxp-media-app-trusty
        install ${S}/examples/chip-tool/out/aarch64-trusty/chip-tool ${D}${bindir}/chip-tool-trusty
    fi
}
