inherit cmake
SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main;protocol=https"
SRCREV = "790dc775144e33995cd1cb2c15b348849cacf737"
SRC_URI += "file://otbrpatches-050-0f7e849/otbr-0001-update-paths-UART-SPI.patch"
SRC_URI += "file://otbrpatches-050-0f7e849/otbr-0002-update-otbr-agent-opts-SPI.patch"
SRC_URI += "file://otbrpatches-050-0f7e849/otbr-0004-firewall-UART-SPI.patch"
SRC_URI += "file://otbrpatches-050-0f7e849/otbr-0005-add-absl-support-for-higher-version-protobuf-UART-SPI.patch"
EXTRA_OECMAKE += "-DCMAKE_CXX_STANDARD=14 -DOT_POSIX_CONFIG_RCP_BUS=SPI -DBUILD_TESTING=OFF -DOTBR_BACKBONE_ROUTER=ON -DOTBR_BORDER_AGENT=ON -DOTBR_BORDER_ROUTING=ON -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_DUA_ROUTING=ON -DOTBR_INFRA_IF_NAME=eth0 -DOTBR_MDNS=mDNSResponder -DOTBR_REST=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOTBR_TREL=ON -DOTBR_WEB=OFF -DOTBR_NAT64=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOT_BORDER_ROUTER=ON -DOT_BORDER_AGENT=ON -DOT_COMMISSIONER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=OFF -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_DHCP6_SERVER=ON -DOT_DHCP6_CLIENT=ON -DOT_DNS_CLIENT=ON -DOT_DNSSD_SERVER=ON -DOT_DUA=ON -DOT_ECDSA=ON -DOT_FIREWALL=ON -DOT_FULL_LOGS=ON -DOT_JOINER=ON -DOT_LOG_LEVEL_DYNAMIC=OFF -DOT_LOG_LEVEL=DEBG -DOT_RCP_RESTORATION_MAX_COUNT=5 -DOT_REFERENCE_DEVICE=ON -DOT_SRP_CLIENT=ON -DOT_SRP_SERVER=ON -DOT_THREAD_VERSION=1.3 -DOT_NAT64_BORDER_ROUTING=ON -DOT_NAT64_TRANSLATOR=ON"

addtask do_openthread_repo_reset_and_patch after do_unpack before do_patch
do_openthread_repo_reset_and_patch() {
    cd ${S}/third_party/openthread/repo
    echo "Applying on ${S}/third_party/openthread/repo:"
    echo "-> git reset --hard 0f7e8491e2c2445331d5febcbe3a24c0c1d4e1e3"
    git reset --hard 0f7e8491e2c2445331d5febcbe3a24c0c1d4e1e3
    result=$?
    if [ ${result} -ne 0 ]; then
        echo "Cannot reset openthread to version '$SHA1_OPENTHREAD_FOR_OTBR', ABORT"
        exit
    fi
    mkdir -p ${WORKDIR}/tmp_patches
    cp ${THISDIR}/files/otbrpatches-050-0f7e849/openthread_patches/* ${WORKDIR}/tmp_patches
    PATCHES_FILES_LIST=$(ls ${WORKDIR}/tmp_patches)
    for i in ${PATCHES_FILES_LIST}; do
        true | git apply ${WORKDIR}/tmp_patches/${i} --check
        if [ $? -eq 0 ]; then
            echo "----> Applying ${i}"
            git apply ${WORKDIR}/tmp_patches/${i}
        fi
    done
    rm -rf  ${WORKDIR}/tmp_patches
}

do_install:append() {
    mkdir ${D}${sysconfdir}/init.d
    cp ${S}/script/otbr-firewall ${D}${sysconfdir}/init.d
    chmod +x ${D}${sysconfdir}/init.d/otbr-firewall
    cp ${S}/script/README_firewall.txt ${D}${sysconfdir}/init.d
}
