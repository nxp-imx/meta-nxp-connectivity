PN = "matter-ncp"
SUMMARY = "Matter Smart Home connectivity on i.MX boards with Redfinch NCP module"
DESCRIPTION = "This layer loads the Matter applications with Redfinch NCP"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "release-matter-ncp-2025q4"
IMX_MATTER_SRC = "gitsm://github.com/NXP/matter.git;protocol=https"
SRC_URI = "${IMX_MATTER_SRC};branch=${SRCBRANCH}"
SRC_URI += "file://0001-MATTER-3758-Remove-fatal-warnings-ld-flags.patch"
MATTER_PY_PATH ?= "${STAGING_BINDIR_NATIVE}/python3-native/python3"

PATCHTOOL = "git"

SRCREV = "b772a02270a03ebadaa211ebfe6b34af4c901bba"

TARGET_CC_ARCH += "${LDFLAGS}"
DEPENDS += " gn-native ninja-native avahi dbus-glib-native pkgconfig-native boost python3-pip-native python3-packaging python3-click libusb glib-2.0 "

inherit pkgconfig
RDEPENDS_${PN} += " libavahi-client boost boost-dev boost-staticdev libusb "
FILES:${PN} += "usr/share"

INSANE_SKIP:${PN} += "dev-so debug-deps strip buildpaths"
INSANE_SKIP:${PN}-dbg += "buildpaths"

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

MATTER_PKG_CONFIG_LIBDIR = "${STAGING_LIBDIR}/pkgconfig:${STAGING_DATADIR}/pkgconfig"

do_configure() {
    cd ${S}/
    if ${DEPLOY_TRUSTY}; then
        git submodule update --init
        ./scripts/checkout_submodules.py
    fi
    cd ${S}
    touch build_overrides/pigweed_environment.gni
    cd ${S}/examples/all-clusters-app/nxp/linux_ncp

    # Configuration for IF_TYPE=1 (usb)
    # Note: Set pw_protobuf_compiler_TOOLCHAIN to match custom_toolchain to avoid
    # duplicate proto output files between custom toolchain and protocol_buffer toolchain
    # The path must use ${chip_root} since it resolves relative to the example directory
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${MATTER_PKG_CONFIG_LIBDIR} \
    PKG_CONFIG_PATH=${MATTER_PKG_CONFIG_LIBDIR} \
    gn gen out/aarch64_usb --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_imx_ele=0 enable_exceptions=true chip_with_linux_ncp_host=1 ncp_host_interface=1 chip_enable_wifi=true chip_code_pre_generated_directory="${S}/zzz_pregencodes" chip_build_libshell=false
        import("//build_overrides/build.gni")
        import("//build_overrides/chip.gni")
        pw_protobuf_compiler_TOOLCHAIN="${chip_root}/build/toolchain/custom:custom"
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    # Configuration for IF_TYPE=2 (uart)
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${MATTER_PKG_CONFIG_LIBDIR} \
    PKG_CONFIG_PATH=${MATTER_PKG_CONFIG_LIBDIR} \
    gn gen out/aarch64_uart --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_imx_ele=0 enable_exceptions=true chip_with_linux_ncp_host=1 ncp_host_interface=2 chip_enable_wifi=true chip_code_pre_generated_directory="${S}/zzz_pregencodes" chip_build_libshell=false
        import("//build_overrides/build.gni")
        import("//build_overrides/chip.gni")
        pw_protobuf_compiler_TOOLCHAIN="${chip_root}/build/toolchain/custom:custom"
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    # Configuration for IF_TYPE=3 (spi)
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${MATTER_PKG_CONFIG_LIBDIR} \
    PKG_CONFIG_PATH=${MATTER_PKG_CONFIG_LIBDIR} \
    gn gen out/aarch64_spi --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_imx_ele=0 enable_exceptions=true chip_with_linux_ncp_host=1 ncp_host_interface=3 chip_enable_wifi=true chip_code_pre_generated_directory="${S}/zzz_pregencodes" chip_build_libshell=false
        import("//build_overrides/build.gni")
        import("//build_overrides/chip.gni")
        pw_protobuf_compiler_TOOLCHAIN="${chip_root}/build/toolchain/custom:custom"
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'

    # Configuration for IF_TYPE=4 (sdio)
    PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
    PKG_CONFIG_LIBDIR=${MATTER_PKG_CONFIG_LIBDIR} \
    PKG_CONFIG_PATH=${MATTER_PKG_CONFIG_LIBDIR} \
    gn gen out/aarch64_sdio --script-executable="${MATTER_PY_PATH}" --args='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" chip_with_imx_ele=0 enable_exceptions=true chip_with_linux_ncp_host=1 ncp_host_interface=4 chip_enable_wifi=true chip_code_pre_generated_directory="${S}/zzz_pregencodes" chip_build_libshell=false
        import("//build_overrides/build.gni")
        import("//build_overrides/chip.gni")
        pw_protobuf_compiler_TOOLCHAIN="${chip_root}/build/toolchain/custom:custom"
        target_cflags=[
                        "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\"",
                       ]
        custom_toolchain="${build_root}/toolchain/custom"
        target_cc="${CC}"
        target_cxx="${CXX}"
        target_ar="${AR}"'
}

do_compile() {
    cd ${S}/examples/all-clusters-app/nxp/linux_ncp
    ninja -C out/aarch64_usb
    ninja -C out/aarch64_uart
    ninja -C out/aarch64_spi
    ninja -C out/aarch64_sdio
}

do_install() {
    install -d -m 755 ${D}${bindir}
    install ${S}/examples/all-clusters-app/nxp/linux_ncp/out/aarch64_usb/chip-all-clusters-app ${D}${bindir}/chip-all-clusters-app-ncp-usb
    install ${S}/examples/all-clusters-app/nxp/linux_ncp/out/aarch64_uart/chip-all-clusters-app ${D}${bindir}/chip-all-clusters-app-ncp-uart
    install ${S}/examples/all-clusters-app/nxp/linux_ncp/out/aarch64_spi/chip-all-clusters-app ${D}${bindir}/chip-all-clusters-app-ncp-spi
    install ${S}/examples/all-clusters-app/nxp/linux_ncp/out/aarch64_sdio/chip-all-clusters-app ${D}${bindir}/chip-all-clusters-app-ncp-sdio
}

INSANE_SKIP_${PN} = "ldflags"

