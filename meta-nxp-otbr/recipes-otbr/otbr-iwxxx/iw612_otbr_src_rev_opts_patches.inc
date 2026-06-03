inherit cmake
SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main;protocol=https"
SRCREV = "1c41b39ed97d4138945357d10b9daa6414397c03"

# patch files for otbr-posix
SRC_URI += "file://otbrpatches-101-964c1eb/otbr-0004-firewall.patch"
SRC_URI += "file://otbrpatches-101-964c1eb/otbr-0007-support-ot-fct.patch"
SRC_URI += "file://otbrpatches-101-964c1eb/otbr-0008-Fix-mcpu-cortex-a53-conflicts-with-march-armv8-a-crc.patch"
SRC_URI += "file://otbrpatches-101-964c1eb/otbr-0009-systemctl-mdns-otbr-options-set-by-otbr-agent-service.patch"

# patch files for openthread
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0001-coex-buff-issue-fix.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0002-csl-debug-prints.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0003-all-vsc-commands.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0006-ot-daemon-release-resources.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0019-remove-toggle-reset-pin.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0020-lnt-fix-spi-latency.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0025-spi-new-design-host.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0034-set-default-txpower-on-reset.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0040-radio-spinel-stream-raw-retry-mechanism.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0051-csl-ch-switch-performance-improvement.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0060-dualpan-otzb-pan-channel-sense.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0062-host-handle-power-save-mode-on-ssp-rxd.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0084-generate-one-time-passcode.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0085-pass-tcp-packet-to-host-if-endpoint-not-initialized.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0086-OT-Vendor-Patterns-at-runtime-with-IWxxx-FW-version.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0087-btnxpuart-State-file-check-before-SPI-transactions.patch;patchdir=third_party/openthread/repo/"
SRC_URI += "file://otbrpatches-101-964c1eb/openthread_patches/0088-GetBusSpeed-fix-vendor-interface-included-if-enabled.patch;patchdir=third_party/openthread/repo/"

EXTRA_OECMAKE += "-DCMAKE_CXX_STANDARD=17 -DBUILD_TESTING=OFF -DOTBR_BACKBONE_ROUTER=ON -DOTBR_BORDER_AGENT=ON -DOTBR_BORDER_ROUTING=ON -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=OFF -DOTBR_DUA_ROUTING=ON -DOTBR_INFRA_IF_NAME=eth0 -DOTBR_MDNS=mDNSResponder -DOTBR_REST=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOTBR_TREL=ON -DOTBR_WEB=OFF -DOTBR_NAT64=ON -DOTBR_DHCP6_PD=ON -DOTBR_DHCP6_PD_CLIENT=openthread -DOTBR_DNS_UPSTREAM_QUERY=ON -DOTBR_VENDOR_NAME=NXP -DOTBR_PRODUCT_NAME=Openthread_BorderRouter -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOT_BORDER_ROUTER=ON -DOT_BORDER_AGENT=ON -DOT_COMMISSIONER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=OFF -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_DHCP6_SERVER=ON -DOT_DHCP6_CLIENT=ON -DOT_DNS_CLIENT=ON -DOT_DNSSD_SERVER=ON -DOT_DUA=ON -DOT_ECDSA=ON -DOT_FIREWALL=ON -DOT_FULL_LOGS=ON -DOT_JOINER=ON -DOT_LOG_LEVEL_DYNAMIC=OFF -DOT_LOG_LEVEL=DEBG -DOT_RCP_RESTORATION_MAX_COUNT=7 -DOT_REFERENCE_DEVICE=OFF -DOT_SRP_CLIENT=ON -DOT_SRP_SERVER=ON -DOT_THREAD_VERSION=1.4 -DOT_NAT64_BORDER_ROUTING=ON -DOT_NAT64_TRANSLATOR=ON -DOT_VENDOR_NAME=NXP -DOT_VENDOR_MODEL=IWxxx -DOT_VENDOR_APP_URL=www.nxp.com -DOT_NETDIAG_VENDOR_INFO=OFF -DOT_POSIX_PRODUCT_CONFIG=/data/openthread/openthread.conf -DOT_MLE_MAX_CHILDREN=128 -DOT_MAC_CSL_REQUEST_AHEAD_US=16500 -DOT_POSIX_RCP_SPI_BUS=ON -DOT_POSIX_RCP_HDLC_BUS=ON -DOT_RCP_TIME_SYNC_INTERVAL=30000000 -DOT_TCP=ON -DOT_DNS_CLIENT_OVER_TCP=ON"

addtask do_openthread_repo_revision_reset after do_unpack before do_patch
do_openthread_repo_revision_reset() {
    cd ${S}/third_party/openthread/repo
    echo "Applying on ${S}/third_party/openthread/repo:"
    echo "-> git reset --hard 964c1ebe4f5aab42b321bc9d8719ea0afc708fde"
    git reset --hard 964c1ebe4f5aab42b321bc9d8719ea0afc708fde
    result=$?
    if [ ${result} -ne 0 ]; then
        echo "Cannot reset openthread to version 964c1ebe4f5aab42b321bc9d8719ea0afc708fde, ABORT"
        exit
    fi
}

do_install:append() {
    install -d -m 755 ${D}${sbindir}
    echo "Renaming ${D}${sbindir}/otbr-agent into ${D}${sbindir}/otbr-agent${BIN_NAME_PATTERN}"
    mv ${D}${sbindir}/otbr-agent ${D}${sbindir}/otbr-agent${BIN_NAME_PATTERN}
    echo "Renaming ${D}${sbindir}/ot-ctl into ${D}${sbindir}/ot-ctl${BIN_NAME_PATTERN}"
    mv ${D}${sbindir}/ot-ctl ${D}${sbindir}/ot-ctl${BIN_NAME_PATTERN}

    echo "Moving ${D}${systemd_system_unitdir}/imx_otbr_gpiochip_spidev_cfg.sh into ${D}${sbindir}"
    mv ${D}${systemd_system_unitdir}/imx_otbr_gpiochip_spidev_cfg.sh ${D}${sbindir}
    chmod +x ${D}${sbindir}/imx_otbr_gpiochip_spidev_cfg.sh

    echo "Renaming ${D}${sysconfdir}/default/otbr-agent into ${D}${sysconfdir}/default/otbr-agent${BIN_NAME_PATTERN}"
    mv ${D}${sysconfdir}/default/otbr-agent ${D}${sysconfdir}/default/otbr-agent${BIN_NAME_PATTERN}

    echo "Copying ${S}/script/otbr-firewall into ${D}${sysconfdir}/init.d"
    mkdir ${D}${sysconfdir}/init.d
    cp ${S}/script/otbr-firewall ${D}${sysconfdir}/init.d
    chmod +x ${D}${sysconfdir}/init.d/otbr-firewall
    echo "Copying ${S}/script/README_firewall.txt into ${D}${sysconfdir}/init.d"
    cp ${S}/script/README_firewall.txt ${D}${sysconfdir}/init.d
}
