PN = "matter"
SUMMARY = "Matter IoT connectivity on i.MX boards"
DESCRIPTION = "This layer loads the main Matter applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "v1.2.1-branch-nxp_imx_2024_q1-post"
IMX_MATTER_SRC ?= "gitsm://github.com/NXP/matter.git;protocol=https"
SRC_URI = "${IMX_MATTER_SRC};branch=${SRCBRANCH}"
MATTER_PY_PATH ?= "${STAGING_BINDIR_NATIVE}/python3-native/python3"

PATCHTOOL = "git"

SRCREV = "78c1ab87be9f3aceb2a19e3e0a6e289d500d6b63"

TARGET_CC_ARCH += "${LDFLAGS}"
DEPENDS += " gn-native ninja-native avahi dbus-glib-native pkgconfig-native boost python3-pip-native python3-packaging python3-click "
RDEPENDS_${PN} += " libavahi-client boost boost-dev boost-staticdev "
FILES:${PN} += "usr/share"

INSANE_SKIP:${PN} += "dev-so debug-deps strip"

DEPLOY_TRUSTY = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', 'true', 'false', d)}"
MATTER_ADVANCED = "${@bb.utils.contains('MACHINE_FEATURES', 'matter-advanced', 'true', 'false', d)}"

def get_target_cpu(d):
    for arg in (d.getVar('TUNE_FEATURES') or '').split():
        if arg == "cortexa7":
            return 'arm'
        if arg == "armv8a":
            return 'arm64'
    return 'arm64'

def get_arm_arch(d):
    for arg in (d.getVar('TUNE_FEATURES') or '').split():
        if arg == "cortexa7":
            return 'armv7ve'
        if arg == "armv8a":
            return 'armv8-a'
    return 'armv8-a'

def get_arm_cpu(d):
    for arg in (d.getVar('TUNE_FEATURES') or '').split():
        if arg == "cortexa7":
            return 'cortex-a7'
        if arg == "armv8a":
            return 'cortex-a53'
    return 'cortex-a53'

TARGET_CPU = "${@get_target_cpu(d)}"
TARGET_ARM_ARCH = "${@get_arm_arch(d)}"
TARGET_ARM_CPU = "${@get_arm_cpu(d)}"

USE_ELE = "${@bb.utils.contains('MACHINE', 'imx93evk-matter', 1, 0, d)}"

S = "${WORKDIR}/git"

common_configure() {
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" build_without_pw=true chip_with_imx_ele=${USE_ELE} enable_exceptions=true chip_code_pre_generated_directory="${S}/zzz_pregencodes"
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

do_configure() {
    cd ${S}/
    if ${DEPLOY_TRUSTY}; then
        git submodule update --init
        ./scripts/checkout_submodules.py
    fi
    cd ${S}/examples/lighting-app/linux
    common_configure

    cd ${S}/examples/all-clusters-app/linux
    common_configure

    cd ${S}/examples/thermostat/linux
    common_configure

    cd ${S}/examples/nxp-thermostat/linux
    common_configure

    if ${MATTER_ADVANCED}; then
        cd ${S}/examples/nxp-media-app/linux
        common_configure
    fi

    cd ${S}/examples/chip-tool
    common_configure

    cd ${S}/examples/ota-provider-app/linux
    common_configure

    cd ${S}/examples/ota-requestor-app/linux
    common_configure

    cd ${S}/examples/bridge-app/linux
    common_configure

    cd ${S}/examples/bridge-app/nxp/linux-imx
    common_configure

    # Build chip-tool-web
    cd ${S}/examples/chip-tool
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64-web --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" enable_rtti=true enable_exceptions=true chip_with_web=1 build_without_pw=true chip_code_pre_generated_directory="${S}/zzz_pregencodes"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
        ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        trusty_configure

        cd ${S}/examples/chip-tool
        trusty_configure

        cd ${S}/examples/nxp-thermostat/linux
        trusty_configure


        if ${MATTER_ADVANCED}; then
            cd ${S}/examples/nxp-media-app/linux
            trusty_configure
        fi
    fi
}

do_compile() {
    cd ${S}/examples/lighting-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/all-clusters-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/thermostat/linux
    ninja -C out/aarch64

    cd ${S}/examples/nxp-thermostat/linux
    ninja -C out/aarch64

    if ${MATTER_ADVANCED}; then
        cd ${S}/examples/nxp-media-app/linux
        ninja -C out/aarch64
    fi

    cd ${S}/examples/chip-tool
    ninja -C out/aarch64

    cd ${S}/examples/ota-provider-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/ota-requestor-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/bridge-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/bridge-app/nxp/linux-imx
    ninja -C out/aarch64

    # Build chip-tool-web
    cd ${S}/examples/chip-tool
    ninja -C out/aarch64-web

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        ninja -C out/aarch64-trusty

        cd ${S}/examples/nxp-thermostat/linux
        ninja -C out/aarch64-trusty

        if ${MATTER_ADVANCED}; then
            cd ${S}/examples/nxp-media-app/linux
            ninja -C out/aarch64-trusty
        fi

        cd ${S}/examples/chip-tool
        ninja -C out/aarch64-trusty
    fi
}

do_install() {
    install -d -m 755 ${D}${bindir}
    install ${S}/examples/lighting-app/linux/out/aarch64/chip-lighting-app ${D}${bindir}
    install ${S}/examples/all-clusters-app/linux/out/aarch64/chip-all-clusters-app ${D}${bindir}
    install ${S}/examples/thermostat/linux/out/aarch64/thermostat-app ${D}${bindir}
    install ${S}/examples/nxp-thermostat/linux/out/aarch64/nxp-thermostat-app ${D}${bindir}

    if ${MATTER_ADVANCED}; then
        install ${S}/examples/nxp-media-app/linux/out/aarch64/nxp-media-app ${D}${bindir}
    fi

    install ${S}/examples/chip-tool/out/aarch64/chip-tool ${D}${bindir}
    install ${S}/examples/ota-provider-app/linux/out/aarch64/chip-ota-provider-app ${D}${bindir}
    install ${S}/examples/ota-requestor-app/linux/out/aarch64/chip-ota-requestor-app ${D}${bindir}
    install ${S}/examples/bridge-app/linux/out/aarch64/chip-bridge-app ${D}${bindir}
    install ${S}/examples/bridge-app/nxp/linux-imx/out/aarch64/imx-chip-bridge-app ${D}${bindir}

    # Install chip-tool-web
    install ${S}/examples/chip-tool/out/aarch64-web/chip-tool-web ${D}${bindir}
    install -d -m 755 ${D}/usr/share/chip-tool-web/
    cp -r ${S}/examples/chip-tool/webui/frontend ${D}/usr/share/chip-tool-web/


    if ${DEPLOY_TRUSTY}; then
        install ${S}/examples/lighting-app/linux/out/aarch64-trusty/chip-lighting-app ${D}${bindir}/chip-lighting-app-trusty
        install ${S}/examples/nxp-thermostat/linux/out/aarch64-trusty/nxp-thermostat-app ${D}${bindir}/nxp-thermostat-app-trusty

        if ${MATTER_ADVANCED}; then
            install ${S}/examples/nxp-media-app/linux/out/aarch64-trusty/nxp-media-app ${D}${bindir}/nxp-media-app-trusty
        fi
        install ${S}/examples/chip-tool/out/aarch64-trusty/chip-tool ${D}${bindir}/chip-tool-trusty
    fi
}

INSANE_SKIP_${PN} = "ldflags"
