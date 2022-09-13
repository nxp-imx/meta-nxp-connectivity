PN = "matter"
SUMMARY = "Matter IoT connectivity on i.MX boards"
DESCRIPTION = "This layer loads the main Matter applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "gitsm://github.com/nxptest/connectedhomeip.git;protocol=https;branch=sve-2-patch"

SRCREV = "3c2fa18632b12a69f7da4dc8c2d87896b73a0b31"

TARGET_CC_ARCH += "${LDFLAGS}"
DEPENDS += " gn-native ninja-native avahi python3-native dbus-glib-native pkgconfig-native "
RDEPENDS_${PN} += " libavahi-client "

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

}

do_compile() {
    cd ${S}/examples/lighting-app/linux
    ninja -C out/aarch64

	cd ${S}/examples/all-clusters-app/linux
    ninja -C out/aarch64

	cd ${S}/examples/thermostat/linux
    ninja -C out/aarch64

	cd ${S}/examples/chip-tool
    ninja -C out/aarch64

    cd ${S}/examples/ota-provider-app/linux
    ninja -C out/aarch64

    cd ${S}/examples/ota-requestor-app/linux
    ninja -C out/aarch64
}

do_install() {
	install -d -m 755 ${D}${bindir}
	install ${S}/examples/lighting-app/linux/out/aarch64/chip-lighting-app ${D}${bindir}
	install ${S}/examples/all-clusters-app/linux/out/aarch64/chip-all-clusters-app ${D}${bindir}
	install ${S}/examples/thermostat/linux/out/aarch64/thermostat-app ${D}${bindir}
	install ${S}/examples/chip-tool/out/aarch64/chip-tool ${D}${bindir}
	install ${S}/examples/ota-provider-app/linux/out/aarch64/chip-ota-provider-app ${D}${bindir}
	install ${S}/examples/ota-requestor-app/linux/out/aarch64/chip-ota-requestor-app ${D}${bindir}
}

INSANE_SKIP_${PN} = "ldflags"
