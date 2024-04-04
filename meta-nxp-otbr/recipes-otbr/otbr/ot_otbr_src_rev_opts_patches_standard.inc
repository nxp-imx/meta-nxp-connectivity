SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main;protocol=https"

SRC_URI += "file://0001-correct-otbr-reference-scripts-install-folder.patch"
SRC_URI += "file://0002-copy-prebuilt-frontend-files-instead-of-build.patch"
SRC_URI += "file://0003-Install-pskc-for-otbr-agent-credentials-generation.patch"
SRC_URI += "file://0004-Add-service-otbr_fwcfg-to-config-the-firewall-policy.patch"
SRC_URI += "file://0001-dbus-add-absl-support-for-higher-version-protobuf.patch"

SRCREV = "8d12b242dbf2398e8df20aa4ee6d387a41abb537"

do_configure:prepend () {
    export REFERENCE_DEVICE=1
    export OTBRWEB_PREBUILT_FRONTEND=1
}

addtask do_nxp_patch after do_unpack before do_patch
do_nxp_patch () {
    if [ ${MACHINE} = "imx93evk-matter" ]; then
        cd "${S}/third_party/openthread/repo/"
        git am -3 "${THISDIR}/files/0001-Apply-IW612-change-to-openthread.patch"
    fi
}

EXTRA_OECMAKE = "-GNinja -DOTBR_BORDER_ROUTING=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=eth0 -DOT_LOG_LEVEL_DYNAMIC=OFF -DOT_FULL_LOGS=ON -DOT_JOINER=ON -DOT_LOG_LEVEL=DEBG -DOPENTHREAD_CONFIG_BACKBONE_ROUTER_DUA_NDPROXYING_ENABLE=1 -DOPENTHREAD_CONFIG_BACKBONE_ROUTER_MULTICAST_ROUTING_ENABLE=1 -DOT_FIREWALL=ON -DOT_DUA=ON -DOTBR_DUA_ROUTING=ON -DOT_DHCP6_SERVER=ON -DOT_DHCP6_CLIENT=ON -DOTBR_BACKBONE_ROUTER=ON -DOTBR_MDNS="avahi" -DOTBR_ENABLE_MDNS_AVAHI=1 -DOTBR_TREL=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOT_COMMISSIONER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=OFF -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_BORDER_ROUTER=ON -DOT_BORDER_AGENT=ON -DOTBR_BORDER_ROUTING_NAT64=ON -DOTBR_BORDER_AGENT=ON -DOT_DNS_CLIENT_SERVICE_DISCOVERY=ON -DOT_DNS_CLIENT=ON -DOT_ECDSA=ON -DOTBR_REST=ON -DOT_SRP_SERVER=ON -DOT_SRP_CLIENT=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOT_REFERENCE_DEVICE=ON -DOT_DNSSD_SERVER=ON -DCMAKE_CXX_STANDARD=17"
EXTRA_OECMAKE += "${@bb.utils.contains_any('MACHINE', "imx93evk-matter",  ' -DOT_RCP_RESTORATION_MAX_COUNT=5 ', '', d)}"
