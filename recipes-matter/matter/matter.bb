PN = "matter"
SUMMARY = "Matter IoT connectivity on i.MX boards"
DESCRIPTION = "This layer loads the main Matter applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "v1.0-branch-nxp_imx_2023_q1"
SRC_URI = "gitsm://androidsource.ap.freescale.net/project/a/github/connectedhomeip;protocol=http;branch=${SRCBRANCH}"
SRC_URI += "file://0001-Add-Network-Commissioning-cluster-setup-for-bridge-a.patch"
#SRC_URI += "file://0001-MA-21007-integrate-libtrustymatter.patch"
#SRC_URI += "file://0002-MA-21007-2-support-device-attestation-based-on-Trust.patch"
#SRC_URI += "file://0003-MA-21007-3-support-P256KeyPair-based-on-Trusty-OS.patch"

SRCREV = "5de26d55ab8ad87ce541ee401f1c47553d45000d"

TARGET_CC_ARCH += "${LDFLAGS}"
DEPENDS += " gn-native ninja-native avahi python3-native dbus-glib-native pkgconfig-native "
RDEPENDS_${PN} += " libavahi-client "

DEPLOY_TRUSTY = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', 'true', 'false', d)}"

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

S = "${WORKDIR}/git"

do_configure() {
    cd ${S}/
    if ${DEPLOY_TRUSTY}; then
        git submodule update --init
        ./scripts/checkout_submodules.py
    fi
    cd ${S}/examples/lighting-app/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

	cd ${S}/examples/all-clusters-app/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

	cd ${S}/examples/thermostat/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

	cd ${S}/examples/nxp-thermostat/linux
	PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    cd ${S}/examples/chip-tool
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
        ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    cd ${S}/examples/ota-provider-app/linux
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
        ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    cd ${S}/examples/ota-requestor-app/linux
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
        ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    cd ${S}/examples/bridge-app/linux
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
    gn gen out/aarch64 --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}"
        import("//build_overrides/build.gni")
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                        "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                        "-O3"
        ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
        PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
        gn gen out/aarch64-trusty --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_trusty_os=1
            import("//build_overrides/build.gni")
            target_cflags=[
                            "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                            "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                            "-O3"
            ]
            custom_toolchain="${build_root}/toolchain/custom"
            target_cc="${CC}"
            target_cxx="${CXX}"
            target_ar="${AR}"'

        cd ${S}/examples/chip-tool
        PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
        PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
        gn gen out/aarch64-trusty --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_trusty_os=1
            import("//build_overrides/build.gni")
            target_cflags=[
                            "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                            "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                            "-O3"
            ]
            custom_toolchain="${build_root}/toolchain/custom"
            target_cc="${CC}"
            target_cxx="${CXX}"
            target_ar="${AR}"'

        cd ${S}/examples/nxp-thermostat/linux
        PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
        PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
        gn gen out/aarch64-trusty --script-executable="/usr/bin/python3" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_trusty_os=1
            import("//build_overrides/build.gni")
            target_cflags=[
                            "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                            "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\"udhcpc -b -i %s \"",
                            "-O3"
            ]
            custom_toolchain="${build_root}/toolchain/custom"
            target_cc="${CC}"
            target_cxx="${CXX}"
            target_ar="${AR}"'
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

	cd ${S}/examples/chip-tool
    ninja -C out/aarch64

    cd ${S}/examples/ota-provider-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/ota-requestor-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/bridge-app/linux
    ninja -C out/aarch64

    if ${DEPLOY_TRUSTY}; then
        cd ${S}/examples/lighting-app/linux
        ninja -C out/aarch64-trusty

        cd ${S}/examples/nxp-thermostat/linux
        ninja -C out/aarch64-trusty

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
	install ${S}/examples/chip-tool/out/aarch64/chip-tool ${D}${bindir}
	install ${S}/examples/ota-provider-app/linux/out/aarch64/chip-ota-provider-app ${D}${bindir}
	install ${S}/examples/ota-requestor-app/linux/out/aarch64/chip-ota-requestor-app ${D}${bindir}
	install ${S}/examples/bridge-app/linux/out/aarch64/chip-bridge-app ${D}${bindir}

    if ${DEPLOY_TRUSTY}; then
        install ${S}/examples/lighting-app/linux/out/aarch64-trusty/chip-lighting-app ${D}${bindir}/chip-lighting-app-trusty
        install ${S}/examples/nxp-thermostat/linux/out/aarch64-trusty/nxp-thermostat-app ${D}${bindir}/nxp-thermostat-app-trusty
        install ${S}/examples/chip-tool/out/aarch64-trusty/chip-tool ${D}${bindir}/chip-tool-trusty
    fi
}

INSANE_SKIP_${PN} = "ldflags"
