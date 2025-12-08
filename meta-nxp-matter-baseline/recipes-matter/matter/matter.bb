PN = "matter"
SUMMARY = "Matter IoT connectivity on i.MX boards"
DESCRIPTION = "This layer loads the main Matter applications"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "1.5-branch-imx_matter_2025_q4-post"
IMX_MATTER_SRC ?= "gitsm://github.com/NXP/matter.git;protocol=https"
SRC_URI = "${IMX_MATTER_SRC};branch=${SRCBRANCH}"
SRC_URI:append = " \
    file://0001-MATTER-1352-2-Add-se_version.h.patch;patchdir=third_party/imx-secure-enclave/repo/ \
    file://0001-Enable-system_site_packages-option-in-pw_build.patch;patchdir=third_party/pigweed/repo/ \
"
MATTER_PY_PATH ?= "${STAGING_BINDIR_NATIVE}/python3-native/python3"

PATCHTOOL = "git"

SRCREV = "fd8e15b34bacf9eb78d33d573d80aa0abc6a108b"

TARGET_CC_ARCH += "${LDFLAGS}"
DEPENDS += " gn-native ninja-native avahi dbus-glib-native pkgconfig-native boost python3-pip-native python3-packaging-native python3-click-native openssl  matter-idl-native python3-jinja2-native python3-lark-native python3-setuptools-native python3-python-path-native "
RDEPENDS_${PN} += " libavahi-client boost boost-dev boost-staticdev openssl "
FILES:${PN} += "usr/share"

INSANE_SKIP:${PN} += "dev-so debug-deps strip"

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

USE_ELE ?= "false"

# Defines Matter applications to build. Format is a pipe-separated string:
# app-path|binary-name|output-dir|extra-gn-args|install-binary-name

MATTER_APPLICATIONS = " \
    'lighting-app/linux|chip-lighting-app|aarch64||chip-lighting-app' \
    'all-clusters-app/linux|chip-all-clusters-app|aarch64||chip-all-clusters-app' \
    'thermostat/linux|thermostat-app|aarch64||thermostat-app' \
    'chip-tool|chip-tool|aarch64|chip_with_imx_ele=false|chip-tool' \
    'ota-provider-app/linux|chip-ota-provider-app|aarch64||chip-ota-provider-app' \
    'ota-requestor-app/linux|chip-ota-requestor-app|aarch64||chip-ota-requestor-app' \
    'bridge-app/linux|chip-bridge-app|aarch64||chip-bridge-app' \
    'energy-management-app/linux|chip-energy-management-app|aarch64||chip-energy-management-app' \
    'chip-tool|chip-tool-web2|aarch64-web|chip_with_web2=1 enable_rtti=true chip_with_imx_ele=false|chip-tool-web2' \
    'nxp-thermostat/linux|nxp-thermostat-app|aarch64||nxp-thermostat-app' \
    'bridge-app/nxp/linux-imx|imx-chip-bridge-app|aarch64||imx-chip-bridge-app' \
"

do_configure() {
    cd ${S}/
    touch build_overrides/pigweed_environment.gni

    GN_BASE_ARGS_CONTENT='treat_warnings_as_errors=false target_os="linux" target_cpu="${TARGET_CPU}" arm_arch="${TARGET_ARM_ARCH}" arm_cpu="${TARGET_ARM_CPU}" build_without_pw=true chip_with_imx_ele=${USE_ELE} enable_exceptions=true chip_code_pre_generated_directory="${S}/zzz_pregencodes" import("//build_overrides/build.gni") target_cflags=["-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\"mlan0\""] custom_toolchain="${build_root}/toolchain/custom" target_cc="${CC}" target_cxx="${CXX}" target_ar="${AR}" pw_build_PYTHON_BUILD_VENV="//third_party/connectedhomeip/examples/platform/nxp/imx/config:imx_yocto_venv" '

    for app_info_quoted in ${MATTER_APPLICATIONS}; do
        app_info=$(echo "${app_info_quoted}" | tr -d "'")

        app_path=$(echo "${app_info}" | cut -d'|' -f1)
        output_dir=$(echo "${app_info}" | cut -d'|' -f3)
        extra_args=$(echo "${app_info}" | cut -d'|' -f4)

        cd "${S}"

        final_args_content="${GN_BASE_ARGS_CONTENT} ${extra_args}"

        PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
        PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
        gn gen --root="${S}/examples/${app_path}" "${S}/examples/${app_path}/out/${output_dir}" --script-executable="${MATTER_PY_PATH}" --args="${final_args_content}"

        cd ${S}
    done
}

do_compile() {

    for app_info_quoted in ${MATTER_APPLICATIONS}; do
        app_info=$(echo "${app_info_quoted}" | tr -d "'")
        app_path=$(echo "${app_info}" | cut -d'|' -f1)
        output_dir=$(echo "${app_info}" | cut -d'|' -f3)

        cd "${S}/examples/${app_path}"
        PW_PROJECT_ROOT=${S} ninja -C "${S}/examples/${app_path}/out/${output_dir}"
        cd ${S}
    done
}

do_install() {
    install -d -m 755 ${D}${bindir}

    for app_info_quoted in ${MATTER_APPLICATIONS}; do
        app_info=$(echo "${app_info_quoted}" | tr -d "'")
        app_path=$(echo "${app_info}" | cut -d'|' -f1)
        binary_name=$(echo "${app_info}" | cut -d'|' -f2)
        output_dir=$(echo "${app_info}" | cut -d'|' -f3)
        install_binary_name=$(echo "${app_info}" | cut -d'|' -f5)

        if [ -z "${install_binary_name}" ]; then
            install_binary_name=${binary_name}
        fi

        install "${S}/examples/${app_path}/out/${output_dir}/${binary_name}" "${D}${bindir}/${install_binary_name}"
    done

    install -d -m 755 ${D}/usr/share/chip-tool-web/
    cp -r "${S}/examples/chip-tool/webui-2_0/frontend2" "${D}/usr/share/chip-tool-web/"
}

INSANE_SKIP_${PN} = "ldflags"
