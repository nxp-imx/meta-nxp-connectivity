SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main"
SRCREV = "6dc3d2f640292820e6377a2500f0c375f447d207"

SRC_URI += "file://0001-Update-otbr-agent-options-and-otbr-firewall-for-IW61.patch"

addtask do_openthread_repo_reset_and_patch after do_unpack before do_patch
do_openthread_repo_reset_and_patch() {
    SHA1_OPENTHREAD_FOR_OTBR="98e644d538ff2c57e96c54bdd6e13df31c7d19ba"
    #SHA1_OTBR_COMMIT_FOR_5_15="528c152a35036ed0effe61e80cf25a978c458bfa"

    #Bug for BSP 5.15 in OTBR repo, check if ${SHA1_OTBR_COMMIT_FOR_5_15} is present
    #cd ${S}
    #git show ${SHA1_OTBR_COMMIT_FOR_5_15}
    #res=$?
    #if [ ${res} -eq 1 ]; then
    #    echo "Error: ${SHA1_OTBR_COMMIT_FOR_5_15} does not exist"
    #else
    #    echo "OTBR patches: cherry-pick: solve compilation issue on BSP 5.15"
    #    echo "----> cherry-pick to ${SHA1_OTBR_COMMIT_FOR_5_15}"
    #    git cherry-pick ${SHA1_OTBR_COMMIT_FOR_5_15}
    #fi

    cd ${S}/third_party/openthread/repo
    echo "Applying on ${S}/third_party/openthread/repo:"
    echo "-> git reset --hard ${SHA1_OPENTHREAD_FOR_OTBR}"
    git reset --hard ${SHA1_OPENTHREAD_FOR_OTBR}
    result=$?
    if [ ${result} -ne 0 ]; then
        echo "Cannot reset openthread to version '$SHA1_OPENTHREAD_FOR_OTBR', ABORT"
        exit
    fi

    mkdir ${WORKDIR}/tmp_patches
    cp $(dirname ${FILE})/files/openthread_patches/* ${WORKDIR}/tmp_patches
    PATCHES_FILES_LIST=$(ls ${WORKDIR}/tmp_patches)
    for i in ${PATCHES_FILES_LIST}; do
        true | git apply ${WORKDIR}/tmp_patches/${i} --check
        if [ $? == 0 ]; then
            echo "----> Applying ${i}"
            git apply ${WORKDIR}/tmp_patches/${i}
        fi
    done
    rm -rf  ${WORKDIR}/tmp_patches
}

EXTRA_OECMAKE = "-GNinja"
EXTRA_OECMAKE += "-DOTBR_BORDER_ROUTING=ON"
EXTRA_OECMAKE += "-DOTBR_WEB=OFF"
EXTRA_OECMAKE += "-DBUILD_TESTING=OFF"
EXTRA_OECMAKE += "-DOTBR_DBUS=ON"
EXTRA_OECMAKE += "-DOTBR_SRP_ADVERTISING_PROXY=ON"
EXTRA_OECMAKE += "-DOT_THREAD_VERSION=1.3"
EXTRA_OECMAKE += "-DOTBR_INFRA_IF_NAME=mlan0"
EXTRA_OECMAKE += "-DOT_LOG_LEVEL_DYNAMIC=OFF"
EXTRA_OECMAKE += "-DOT_FULL_LOGS=ON"
EXTRA_OECMAKE += "-DOT_JOINER=ON"
EXTRA_OECMAKE += "-DOT_LOG_LEVEL=DEBG"
EXTRA_OECMAKE += "-DOT_BACKBONE_ROUTER_DUA_NDPROXYING=ON"
EXTRA_OECMAKE += "-DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON"
EXTRA_OECMAKE += "-DOT_FIREWALL=ON"
EXTRA_OECMAKE += "-DOT_DUA=ON"
EXTRA_OECMAKE += "-DOTBR_DUA_ROUTING=ON"
EXTRA_OECMAKE += "-DOT_DHCP6_SERVER=ON"
EXTRA_OECMAKE += "-DOT_DHCP6_CLIENT=ON"
EXTRA_OECMAKE += "-DOTBR_BACKBONE_ROUTER=ON"
EXTRA_OECMAKE += "-DOTBR_MDNS="mDNSResponder""
EXTRA_OECMAKE += "-DOTBR_ENABLE_MDNS_MDNSSD=1"
EXTRA_OECMAKE += "-DOTBR_TREL=ON"
EXTRA_OECMAKE += "-DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON"
EXTRA_OECMAKE += "-DOT_COMMISSIONER=ON"
EXTRA_OECMAKE += "-DOT_COAP=ON"
EXTRA_OECMAKE += "-DOT_COAP_BLOCK=OFF"
EXTRA_OECMAKE += "-DOT_COAP_OBSERVE=ON"
EXTRA_OECMAKE += "-DOT_COAPS=ON"
EXTRA_OECMAKE += "-DOT_BORDER_ROUTER=ON"
EXTRA_OECMAKE += "-DOT_BORDER_AGENT=ON"
EXTRA_OECMAKE += "-DOT_BORDER_ROUTING_NAT64=ON"
EXTRA_OECMAKE += "-DOTBR_BORDER_AGENT=ON"
EXTRA_OECMAKE += "-DOT_DNS_CLIENT=ON"
EXTRA_OECMAKE += "-DOT_ECDSA=ON"
EXTRA_OECMAKE += "-DOTBR_REST=ON"
EXTRA_OECMAKE += "-DOT_SRP_SERVER=ON"
EXTRA_OECMAKE += "-DOT_SRP_CLIENT=ON"
EXTRA_OECMAKE += "-DOTBR_DNSSD_DISCOVERY_PROXY=ON"
EXTRA_OECMAKE += "-DOT_REFERENCE_DEVICE=ON"
EXTRA_OECMAKE += "-DOT_DNSSD_SERVER=ON"
 
do_install:append() {
    mkdir ${D}${sysconfdir}/init.d
    cp ${S}/script/otbr-firewall ${D}${sysconfdir}/init.d
    chmod +x ${D}${sysconfdir}/init.d/otbr-firewall
    cp ${S}/script/otbr-firewall_README.txt ${D}${sysconfdir}/init.d
}
