BUILD_TRUSTY = "${@bb.utils.contains('MACHINE_FEATURES', 'trusty', 'true', 'false', d)}"

do_compile:append() {
    if ${BUILD_TRUSTY}; then
        oe_runmake clean BUILD_BASE=build-trusty
        oe_runmake BUILD_BASE=build-trusty SPD=trusty bl31
    fi
}

do_deploy:append() {
    if ${BUILD_TRUSTY}; then
        install -m 0644 ${S}/build-trusty/${ATF_PLATFORM}/release/bl31.bin ${DEPLOYDIR}/bl31-${ATF_PLATFORM}.bin-trusty
        install -m 0644 ${S}/build-trusty/${ATF_PLATFORM}/release/bl31.bin ${DEPLOYDIR}/${BOOT_TOOLS}/bl31-${ATF_PLATFORM}.bin-trusty
    fi
}
