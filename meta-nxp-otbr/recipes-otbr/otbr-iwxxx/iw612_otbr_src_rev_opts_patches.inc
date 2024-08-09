inherit cmake
SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main;protocol=https"
SRCREV = "45c847a6b47cef00c9e3d46786127ef87475437d"

# patch files for otbr-posix
SRC_URI += "file://otbrpatches-061-9681690/otbr-0001-update-paths-UART-SPI.patch"
SRC_URI += "file://otbrpatches-061-9681690/otbr-0002-update-otbr-agent-opts-SPI.patch"
SRC_URI += "file://otbrpatches-061-9681690/otbr-0004-firewall-UART-SPI.patch"
SRC_URI += "file://otbrpatches-061-9681690/otbr-0006-use-systemctl-for-mdns-start-UART-SPI.patch"

# patch files for openthread
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0001-coex-buff-issue-fix.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0002-csl-debug_prints.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0003_all_VSC_commands.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0006-ot-daemon-release-resources.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0019-remove-toggle-reset-pin.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0020-lnt-fix-spi-latency.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0023-csl-ahead-time.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0024-spi-new-design-common.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0025-spi-new-design-host.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0026-128-sed.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0034-set-default-txpower-on-reset.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0036-set-default-rssi-on-reset-for-host.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0040-radio-spinel-stream-raw-retry-mechanism.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0045-disable-channel-monitor-feature.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0051-csl-ch-switch-performance-improvement.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0053-fix-software-reset-issue-on-rebased-OT.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0054-9c1f1f3df-Key-switch-gaurd-timer-update.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-061-9681690/openthread_patches/0057-reset-available-on-rcp-recovery.patch;patchdir=third_party/openthread/repo/"

EXTRA_OECMAKE += "-DCMAKE_CXX_STANDARD=17 -DBUILD_TESTING=OFF -DOTBR_BACKBONE_ROUTER=ON -DOTBR_BORDER_AGENT=ON -DOTBR_BORDER_ROUTING=ON -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_DUA_ROUTING=ON -DOTBR_INFRA_IF_NAME=eth0 -DOTBR_MDNS=mDNSResponder -DOTBR_REST=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOTBR_TREL=ON -DOTBR_WEB=OFF -DOTBR_NAT64=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOT_BORDER_ROUTER=ON -DOT_BORDER_AGENT=ON -DOT_COMMISSIONER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=OFF -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_DHCP6_SERVER=ON -DOT_DHCP6_CLIENT=ON -DOT_DNS_CLIENT=ON -DOT_DNSSD_SERVER=ON -DOT_DUA=ON -DOT_ECDSA=ON -DOT_FIREWALL=ON -DOT_FULL_LOGS=ON -DOT_JOINER=ON -DOT_LOG_LEVEL_DYNAMIC=OFF -DOT_LOG_LEVEL=DEBG -DOT_RCP_RESTORATION_MAX_COUNT=5 -DOT_REFERENCE_DEVICE=ON -DOT_SRP_CLIENT=ON -DOT_SRP_SERVER=ON -DOT_THREAD_VERSION=1.4 -DOT_NAT64_BORDER_ROUTING=ON -DOT_NAT64_TRANSLATOR=ON"

addtask do_openthread_repo_revision_reset after do_unpack before do_patch
do_openthread_repo_revision_reset() {
    cd ${S}/third_party/openthread/repo
    echo "Applying on ${S}/third_party/openthread/repo:"
    echo "-> git reset --hard 9681690fab100590566e4937cbf2d072de031ff3"
    git reset --hard 9681690fab100590566e4937cbf2d072de031ff3
    result=$?
    if [ ${result} -ne 0 ]; then
        echo "Cannot reset openthread to version 9681690fab100590566e4937cbf2d072de031ff3, ABORT"
        exit
    fi
}

do_install:append() {
    install -d -m 755 ${D}${sbindir}
    echo "Renaming ${D}${sbindir}/otbr-agent into ${D}${sbindir}/otbr-agent${BIN_NAME_PATTERN}"
    mv ${D}${sbindir}/otbr-agent ${D}${sbindir}/otbr-agent${BIN_NAME_PATTERN}
    echo "Renaming ${D}${sbindir}/ot-ctl into ${D}${sbindir}/ot-ctl${BIN_NAME_PATTERN}"
    mv ${D}${sbindir}/ot-ctl ${D}${sbindir}/ot-ctl${BIN_NAME_PATTERN}

    echo "Renaming ${D}${sysconfdir}/default/otbr-agent into ${D}${sysconfdir}/default/otbr-agent${BIN_NAME_PATTERN}"
    mv ${D}${sysconfdir}/default/otbr-agent ${D}${sysconfdir}/default/otbr-agent${BIN_NAME_PATTERN}

    mkdir ${D}${sysconfdir}/init.d
    cp ${S}/script/otbr-firewall ${D}${sysconfdir}/init.d
    chmod +x ${D}${sysconfdir}/init.d/otbr-firewall
    cp ${S}/script/README_firewall.txt ${D}${sysconfdir}/init.d
}
