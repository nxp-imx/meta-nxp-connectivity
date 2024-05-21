inherit cmake
SRC_URI = "gitsm://github.com/openthread/openthread.git;branch=main;protocol=https"
SRCREV = "0f7e8491e2c2445331d5febcbe3a24c0c1d4e1e3"

# patch files for openthread
SRC_URI += "file://otpatches-052-0f7e849/0001-coex-buff-issue-fix.patch"
SRC_URI += "file://otpatches-052-0f7e849/0002-csl-debug_prints.patch"
SRC_URI += "file://otpatches-052-0f7e849/0003-set-vsc-ir-eui64.patch"
SRC_URI += "file://otpatches-052-0f7e849/0004-set-vsc-txpwr-limit.patch"
SRC_URI += "file://otpatches-052-0f7e849/0005-host-spi-clear-tx-buffer.patch"
SRC_URI += "file://otpatches-052-0f7e849/0006-ot-daemon-release-resources.patch"
SRC_URI += "file://otpatches-052-0f7e849/0008-add-mfg-cmd-for-pfw.patch"
SRC_URI += "file://otpatches-052-0f7e849/0011-get-vsc-txpwr-limit.patch"
SRC_URI += "file://otpatches-052-0f7e849/0012-vsc-for-otbr.patch"
SRC_URI += "file://otpatches-052-0f7e849/0013-cca-configuration.patch"
SRC_URI += "file://otpatches-052-0f7e849/0015-get-vsc-fwversion.patch"
SRC_URI += "file://otpatches-052-0f7e849/0016-lnt-fix-read-settings.patch"
SRC_URI += "file://otpatches-052-0f7e849/0017-lnt-buffer-handle-when-recover.patch"
SRC_URI += "file://otpatches-052-0f7e849/0019-remove-toggle-reset-pin.patch"
SRC_URI += "file://otpatches-052-0f7e849/0020-lnt-fix-spi-latency.patch"
SRC_URI += "file://otpatches-052-0f7e849/0023-csl-ahead-time.patch"
SRC_URI += "file://otpatches-052-0f7e849/0024-spi-new-design.patch"
SRC_URI += "file://otpatches-052-0f7e849/0025-spi-default-packet-size.patch"
SRC_URI += "file://otpatches-052-0f7e849/0026-128-sed.patch"
SRC_URI += "file://otpatches-052-0f7e849/0029-fix-get-txpwrlimit-ret.patch"
SRC_URI += "file://otpatches-052-0f7e849/0030-range-check-for-rf-test-mode.patch"
SRC_URI += "file://otpatches-052-0f7e849/0032-cca3-configuration.patch"
SRC_URI += "file://otpatches-052-0f7e849/0033-remove-duplicate-error-msg.patch"
SRC_URI += "file://otpatches-052-0f7e849/0034-set-default-txpower-on-reset.patch"
SRC_URI += "file://otpatches-052-0f7e849/0035-set-ch-on-ed-scan-with-param.patch"
SRC_URI += "file://otpatches-052-0f7e849/0036-set-default-rssi-on-reset-for-host.patch"
SRC_URI += "file://otpatches-052-0f7e849/0040-radio-spinel-stream-raw-retry-mechanism.patch"
SRC_URI += "file://otpatches-052-0f7e849/0042-ir-oob-threshold.patch"
SRC_URI += "file://otpatches-052-0f7e849/0045-disable-channel-monitor-feature.patch"
SRC_URI += "file://otpatches-052-0f7e849/0047-host-dualpan-feature-integration.patch"
SRC_URI += "file://otpatches-052-0f7e849/0049-add-ircfg-get-command-in-host.patch"
SRC_URI += "file://otpatches-052-0f7e849/0051-csl-ch-switch-performance-improvement.patch"

EXTRA_OECMAKE += "-DOT_SLAAC=ON -DOT_ANYCAST_LOCATOR=ON -DOT_BORDER_AGENT=ON -DOT_BORDER_AGENT_ID=ON -DOT_BORDER_ROUTER=ON -DOT_CHANNEL_MANAGER=ON -DOT_CHANNEL_MONITOR=ON -DOT_COAP=ON -DOT_COAPS=ON -DOT_COAP_BLOCK=ON -DOT_COAP_OBSERVE=ON -DOT_COMMISSIONER=ON -DOT_COMPILE_WARNING_AS_ERROR=ON -DOT_COVERAGE=ON -DOT_DATASET_UPDATER=ON -DOT_DHCP6_CLIENT=ON -DOT_DHCP6_SERVER=ON -DOT_DIAGNOSTIC=ON -DOT_DNSSD_SERVER=ON -DOT_DNS_CLIENT=ON -DOT_ECDSA=ON -DOT_HISTORY_TRACKER=ON -DOT_IP6_FRAGM=ON -DOT_JAM_DETECTION=ON -DOT_JOINER=ON -DOT_LOG_LEVEL_DYNAMIC=ON -DOT_MAC_FILTER=ON -DOT_NEIGHBOR_DISCOVERY_AGENT=ON -DOT_NETDATA_PUBLISHER=ON -DOT_NETDIAG_CLIENT=ON -DOT_PING_SENDER=ON -DOT_RCP_RESTORATION_MAX_COUNT=2 -DOT_REFERENCE_DEVICE=ON -DOT_SERVICE=ON -DOT_SNTP_CLIENT=ON -DOT_SRP_CLIENT=ON -DOT_SRP_SERVER=ON -DOT_UPTIME=ON -DOT_TCP=OFF -DOT_LOG_OUTPUT=PLATFORM_DEFINED -DOT_POSIX_MAX_POWER_TABLE=ON -DOT_PLATFORM=posix -DCMAKE_BUILD_TYPE=Release -DOT_COVERAGE=0 -DOT_COMPILE_WARNING_AS_ERROR=0 -DOT_DAEMON=1 -DOT_POSIX_CONFIG_RCP_BUS=SPI -DOT_BACKBONE_ROUTER=1 -DOT_FULL_LOGS=1 -DOT_DUA=1 -DOT_LINK_METRICS_INITIATOR=1 -DOT_LINK_METRICS_SUBJECT=1 -DOT_MLR=1 -DOT_RCP_RESTORATION_MAX_COUNT=5 -DOT_TCP=OFF -DOT_THREAD_VERSION=1.3 -DOT_CHANNEL_MONITOR=0"

do_install:append() {
    install -d -m 755 ${D}${bindir}
    echo "Renaming ${D}${sbindir}/ot-daemon into ${D}${bindir}/ot-daemon${BIN_NAME_PATTERN}"
    mv ${D}${sbindir}/ot-daemon ${D}${bindir}/ot-daemon${BIN_NAME_PATTERN}
    echo "Renaming ${D}${bindir}/ot-ctl into ${D}${bindir}/ot-client${BIN_NAME_PATTERN}"
    mv ${D}${bindir}/ot-ctl ${D}${bindir}/ot-client${BIN_NAME_PATTERN}
    install -d -m 755 ${D}${sbindir}
    rm -rf ${D}${sbindir}
}
