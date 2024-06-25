# Contents

[**Introduction**](#Introduction)

[**i.MX MPU Matter platform**](#imx-mpu-platform)

[**What's new**](#new-feature)

[**How to build the Yocto image with integrated OpenThread Border Router**](#How-to-build-the-Yocto-image)

[**How to build OpenThread Border Router and OpenThread Daemon with Yocto SDK**](#How-to-build-OTBR-OT)

[**How to setup OpenThread Border Router and OpenThread Daemon environment within the Yocto**](#How-to-setup-OTBR-OT)

[**How to build Matter application**](#How-to-build-Matter-application)

[**Security configuration for Matter**](#Security-configuration-for-Matter)

[**FAQ**](#FAQ)

<a name="Introduction"></a>

# Introduction
This repository contains the i.MX MPU project Matter related Yocto recipes. The following modules will be built with this meta-nxp-connectivity layer:
 - Matter (CHIP) : https://github.com/nxp/matter.git
 - OpenThread Daemon: https://github.com/openthread/openthread
 - OpenThread Border Router: https://github.com/openthread/ot-br-posix

All the software components revisions are based on [Matter v1.3](https://github.com/project-chip/connectedhomeip/tree/v1.3-branch)

The Following Matter related binaries will be installed into the Yocto image root filesystem by this Yocto layer recipes:
 - chip-lighting-app: Matter lighting app demo
 - chip-lighting-app-trusty: Matter lighting app with enhanced security on i.MX8M Mini
 - chip-all-clusters-app: Matter all-clusters demo
 - thermostat-app: Matter thermostat demo
 - nxp-thermostat-app: NXP customized thermostat application which used for Matter Certification
 - nxp-thermostat-app-trusty: NXP customized thermostat application with enhanced security on i.MX8M Mini
 - chip-bridge-app: Matter bridge demo
 - imx-chip-bridge-app: NXP customized Zigbee bridge application
 - nxp-media-app: NXP customized media application
 - nxp-media-app-trusty: NXP customized media application with enhanced security on i.MX8M Mini
 - chip-energy-management-app: Matter energy management app demo
 - chip-tool: Matter Controller tool
 - chip-tool-trusty: Matter Controller tool with enhanced security for i.MX8M Mini
 - chip-tool-web: Matter Web Controller tool
 - chip-tool-web2: Matter Web Controller tool2 alpha version with new Angular Material UI
 - chip-ota-provider-app: Matter ota provider app demo
 - chip-ota-requestor-app: Matter ota requestor app demo
 - ot-daemon: OpenThread Daemon for OpenThread client
 - ot-client-ctl: OpenThread ctrl tool for OpenThread client
 - otbr-agent: OpenThread Border Router agent
 - ot-ctl: OpenThread Border Router ctrl tool
 - ot-daemon-iwxxx-spi: OpenThread Daemon for OpenThread client of the IW612 chipset
 - ot-client-iwxxx-spi: OpenThread ctrl tool for OpenThread client of the IW612 chipset
 - otbr-agent-iwxxx-spi: OpenThread Border Router agent of the IW612 chipset
 - ot-ctl-iwxxx-spi: OpenThread Border Router ctrl tool of the IW612 chipset
 - otbr-web: OpenThread Border Router web management daemon

<a name="imx-mpu-platform"></a>

# i.MX MPU Matter platform

We currently support 4 i.MX MPU platforms, which are the i.MX93 EVK, the i.MX8M Mini EVK, the i.MX6ULL EVK and the i.MX8ULP EVK. For more details, please refer to the [NXP i.MX MPU Matter Platform](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/mpu-linux-hosted-matter-development-platform:MPU-LINUX-MATTER-DEV-PLATFORM).

<a name="How-to-build-the-Yocto-image"></a>

<a name="new-feature">

# What's new

- Upgraded the Matter software component revisions to [Matter v1.3](https://github.com/project-chip/connectedhomeip/tree/v1.3-branch)
- Added the Energy Management, including the Energy Management Cluster and the Energy EVSE Cluster support
- Introduced chip-tool-web2 alpha with Angular Material based UI, chip-tool-web2 is similar to chip-tool-web and currently supports onnetwork / ble-wifi/ ble-thread pairing and onoff read / on / off / toggle / subscribe features
- Integrated Linux L6.6.3_2.0.0 and Yocto scarthgap

# How to build the Yocto image with integrated OpenThread Border Router

The following packages are required to build the Yocto Project:

    $ sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
    build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev \
    pylint3 xterm npm zstd build-essential libpython3-dev libdbus-1-dev python3.8-venv lz4 \
    git gcc g++ pkg-config libssl-dev libglib2.0-dev libavahi-client-dev ninja-build \
    python3-venv python3-dev unzip libgirepository1.0-dev libcairo2-dev libreadline-dev

Make sure that your default Python3 version is 3.8:

    $ python3 --version
      Python 3.8.0

Then, Yocto build environment must be setup.

The Yocto source code is maintained with a manifest file, used by repo tool to download the corresponding source code.
This document is tested with the i.MX Yocto 6.6.3-1.0.0 release. The hardware tested are: i.MX 8M Mini EVK, i.MX6ULL EVK, i.MX93 EVK and i.MX8ULP EVK.
Run the commands below to download this release:

    $ mkdir ~/bin
    $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
    $ chmod a+x ~/bin/repo
    $ export PATH=${PATH}:~/bin

    $ mkdir ${MY_YOCTO} # this directory will be the top directory of the Yocto source code
    $ cd ${MY_YOCTO}
    $ repo init -u https://github.com/nxp-imx/imx-manifest -b imx-linux-nanbield -m imx-6.6.3-1.0.0.xml
    $ repo sync

Then integrate the meta-nxp-connectivity recipes into the Yocto code base

    $ cd ${MY_YOCTO}/sources/
    $ git clone https://github.com/nxp-imx/meta-nxp-connectivity.git meta-nxp-connectivity
    $ cd meta-nxp-connectivity
    $ git checkout imx_matter_2024_q2

More information about the downloaded Yocto release can be found in the corresponding i.MX Yocto Project User’s Guide, which can be found at [NXP official website](http://www.nxp.com/imxlinux).

Change the current directory to the top directory of the Yocto source code and execute the command below:

    # For i.MX8M Mini EVK
    $ MACHINE=imx8mmevk-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8mm

    # For i.MX6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx6ull

    # For i.MX93 EVK:
    $ MACHINE=imx93evk-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx93

    # For i.MX8ULP EVK:
    $ MACHINE=imx8ulpevk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8ulp

This will create a Python virtual environment for the Matter build. To exit the Python virtual environment, please run "$ deactivate". You can also run "$ source matter_venv/bin/activate" at the top directory of the Yocto source code to re-enter the Python virtual environment for the Matter build.

This will also create a build directory (namely bld-xwayland-imx8mm/ for i.MX8M Mini EVK, bld-xwayland-imx6ull/ for i.MX6ULL EVK, bld-xwayland-imx93/ for i.MX93 EVK or bld-xwayland-imx8ulp/ for i.MX8ULP EVK), and enter this directory automatically. Please execute the command below to generate the Yocto images:

    $ bitbake imx-image-multimedia

After execution of previous commands, the Yocto images will be generated:
- ${MY_YOCTO}/bld-xwayland-imx8mm-iw612/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk-matter.wic.zst for i.MX8M Mini EVK with certified IW612 chipset usage.
- ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst for i.MX6ULL EVK.
- ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk-iwxxx-matter/imx-image-multimedia-imx93evk-iwxxx-matter.wic.zst for i.MX93 EVK.
- ${MY_YOCTO}/bld-xwayland-imx8ulp/tmp/deploy/images/imx8ulpevk/imx-image-multimedia-imx8ulpevk.wic.zst for i.MX8ULP EVK.

The zst images are symbolic link files, so you should copy them to a dedicated folder ${MY_images} before unziping them.

    # For i.MX8M Mini EVK
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mm-iw612/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk-matter.wic.zst ${MY_images}

    # For i.MX6ULL EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst ${MY_images}

    # For i.MX93 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk-iwxxx-matter/imx-image-multimedia-imx93evk-iwxxx-matter.wic.zst ${MY_images}

    # For i.MX8ULP EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8ulp/tmp/deploy/images/imx8ulpevk/imx-image-multimedia-imx8ulpevk.wic.zst ${MY_images}

You can use the zstd and dd commands to flash the images to a microSD card for i.MX 8M Mini EVK, i.MX6ULL EVK or i.MX93 EVK. You can also use the [Universal Update Utility](https://github.com/nxp-imx/mfgtools) to flash the images to a microSD card or EMMC for all 4 boards. The microSD card will be used to boot the image on an i.MX 8M Mini EVK, i.MX6ULL EVK or i.MX93 EVK, and the EMMC will be used to boot the image on an i.MX8ULP EVK.

For use with the zstd and dd command method, please use the zstd command to unzip this .zst archive, and then use the dd command to program the output file to a microSD card.

___Be cautious when executing the dd command below, making sure that the output ("of" parameter) represents the microSD card device! /dev/sdc in the below command represents a microSD card connected to the host machine with a USB adapter; however the output device name may vary. Please use "ls /dev/sd*" command to verify the name of the SD card device.___

    $ cd ${MY_images}

    # For i.MX8M Mini EVK
    $ zstd -d imx-image-multimedia-imx8mmevk-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx8mmevk-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX6ULL EVK:
    $ zstd -d imx-image-multimedia-imx6ullevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx6ullevk.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX93 EVK:
    $ zstd -d imx-image-multimedia-imx93evk-iwxxx-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx93evk-iwxxx-matter.wic of=/dev/sdc bs=4M conv=fsync

For use with the uuu method, please install [uuu](https://github.com/nxp-imx/mfgtools/releases/tag/uuu_1.5.21) on your host and make sure it is at least version 1.5.109.

    $ uuu -version
    uuu (Universal Update Utility) for nxp imx chips -- libuuu_1.5.109-0-g6c3190c

___Before flashing the image, follow the prompts on the board to put the board into serial download mode. After flashing the image, please put i.MX8M Mini EVK, i.MX6ULL EVK. i.MX93 EVK into MicroSD boot mode to boot the image from the MicroSD card. And please put i.MX8ULP EVK into EMMC boot mode to boot the image from the EMMC.___

    $ cd ${MY_images}

    # For i.MX8M Mini EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx8mmevk-matter.wic.zst

    # For i.MX6ULL EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx6ullevk.wic.zst

    # For i.MX93 EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx93evk-iwxxx-matter.wic.zst

    # For i.MX8ULP EVK:
    $ sudo uuu -b emmc_all imx-image-multimedia-imx8ulpevk.wic.zst

The prebuilt images for i.MX8M Mini EVK, i.MX6ULL EVK, i.MX93 EVK and i.MX8ULP EVK can be downloaded from [NXP i.MX MPU Matter Platform](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/mpu-linux-hosted-matter-development-platform:MPU-LINUX-MATTER-DEV-PLATFORM).

<a name="How-to-build-OTBR-OT"></a>

# How to build OpenThread Border Router and OpenThread Daemon with Yocto SDK

There are 3 modules for OpenThread Border Router (OTBR): otbr-agent, ot-ctl and otbr-web.
There are 2 modules for OpenThread: ot-daemon, ot-client-ctl.

To build these binaries, we need to use the Yocto SDK toolchain with meta-nxp-connectivity included.
This SDK can be generated with below commands:

    # For i.MX8M Mini EVK, i.MX93 EVK and i.MX8ULP EVK:
    $ MACHINE=imx8n9-sdk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8n9sdk
    $ cd ${MY_YOCTO}/bld-xwayland-imx8n9sdk

    # For i.MX6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx6ull
    $ cd ${MY_YOCTO}/bld-xwayland-imx6ull

    $ bitbake imx-image-sdk -c populate_sdk

Then, install the Yocto SDK, by running the SDK installation script with root permission:

    # For i.MX8M Mini EVK, i.MX93 EVK and i.MX8ULP EVK:
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8n9-sdk-toolchain-6.6-nanbield.sh

    # For i.MX6ULL EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-cortexa7t2hf-neon-imx6ullevk-toolchain-6.6-nanbield.sh

The SDK installation directory will be prompted during the SDK installation; user can specify the installation directory, or keep the default one \${/opt/fsl-imx-xwayland/}.
___Please use board specific paths if you need to build the SDK for several boards EVK; for exmaple, you can use /opt/fsl-imx-xwayland/6.6-nanbield-imx8n9 for i.MX8M Mini EVK SDK, i.MX93 EVK SDK and i.MX8ULP EVK, /opt/fsl-imx-xwayland/6.6-nanbield-imx6ull for i.MX6ULL EVK.___

    NXP i.MX Release Distro SDK installer version 6.6-nanbield
    ============================================================
    Enter target directory for SDK (default: /opt/fsl-imx-xwayland/6.6-nanbield):

After the Yocto SDK is installed on the host machine, an SDK environment setup script is also generated.
User needs to import Yocto build environment, by sourcing this script each time the SDK is used in a new shell; for example:

    # For i.MX8M Mini EVK, i.MX93 EVK and i.MX8ULP EVK
    $ . /opt/fsl-imx-xwayland/6.6-nanbield-imx8n9/environment-setup-armv8a-poky-linux

    $ For i.MX6ULL EVK
    $ . /opt/fsl-imx-wayland/6.6-nanbield-imx6ull/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi

Fetch the latest otbr source code and execute the build for OTBR:

    $ mkdir ${MY_OTBR}  # this directory will be the top directory of the OTBR source code
    $ cd ${MY_OTBR}
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git checkout 8d12b242dbf2398e8df20aa4ee6d387a41abb537
    $ git cherry-pick afe85ccdc5b8557281241d21bb175a55db83c28f
    $ git submodule update --init
    $ cd ..
    $ mkdir ${PROTOC_DIR}
    $ cd ${PROTOC_DIR}
    $ wget https://github.com/protocolbuffers/protobuf/releases/download/v23.0/protoc-23.0-linux-x86_64.zip
    $ unzip protoc-23.0-linux-x86_64.zip
    $ cd ../ot-br-posix

    # For i.MX8M Mini EVK and i.MX8ULP EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=avahi \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake \
      -DProtobuf_PROTOC_EXECUTABLE=${PROTOC_DIR}/bin/protoc -DCMAKE_CXX_STANDARD=14

    # For i.MX6ULL EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=avahi \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/arm.cmake \
      -DProtobuf_PROTOC_EXECUTABLE=${PROTOC_DIR}/bin/protoc -DCMAKE_CXX_STANDARD=14

    # For i.MX93 EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=avahi \
      -DOT_POSIX_CONFIG_RCP_BUS=SPI -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake \
      -DProtobuf_PROTOC_EXECUTABLE=${PROTOC_DIR}/bin/protoc -DCMAKE_CXX_STANDARD=14 -DOT_RCP_RESTORATION_MAX_COUNT=5

The otbr-agent is built in \${MY_OTBR}/build/otbr/src/agent/otbr-agent.
The otbr-web is built in \${MY_OTBR}/build/otbr/src/web/otbr-web.
The ot-ctl is built in \${MY_OTBR}/build/otbr/third_party/openthread/repo/src/posix/ot-ctl.

Please copy them into target /usr/sbin/ directory.

__The OTBR does not support incremental compilation. If an error occurs during compilation, or if you need to recompile, please delete ${MY_OTBR}/build before recompiling.__

    $ cd ${MY_OTBR}
    $ rm -rf build/

Fetch the latest openthread source code and execute the build for OpenThread:

    $ mkdir ${MY_OPENTHREAD}  # this directory will be the top directory of the Open Thread source code
    $ cd ${MY_OPENTHREAD}
    $ git clone https://github.com/openthread/openthread
    $ cd openthread
    $ git checkout 5beae143700db54c6e9bd4b15a568abe2f305723

    # For i.MX8M Mini EVK, i.MX8ULP EVK and i.MX6ULL EVK
    $ cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOT_COMPILE_WARNING_AS_ERROR=OFF -DOT_PLATFORM=posix -DOT_SLAAC=ON \
      -DOT_BORDER_AGENT=ON -DOT_BORDER_ROUTER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=ON -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_COMMISSIONER=ON \
      -DOT_CHANNEL_MANAGER=ON -DOT_CHANNEL_MONITOR=ON -DOT_CHILD_SUPERVISION=ON -DOT_DATASET_UPDATER=ON -DOT_DHCP6_CLIENT=ON \
      -DOT_DHCP6_SERVER=ON -DOT_DIAGNOSTIC=ON -DOT_DNS_CLIENT=ON -DOT_ECDSA=ON -DOT_IP6_FRAGM=ON -DOT_JAM_DETECTION=ON -DOT_JOINER=ON \
      -DOT_LEGACY=ON -DOT_MAC_FILTER=ON -DOT_NETDIAG_CLIENT=ON -DOT_NEIGHBOR_DISCOVERY_AGENT=ON -DOT_PING_SENDER=ON \
      -DOT_REFERENCE_DEVICE=ON -DOT_SERVICE=ON -DOT_SNTP_CLIENT=ON -DOT_SRP_CLIENT=ON -DOT_COVERAGE=OFF -DOT_LOG_LEVEL_DYNAMIC=ON \
      -DOT_RCP_RESTORATION_MAX_COUNT=2 -DOT_LOG_OUTPUT=PLATFORM_DEFINED -DOT_POSIX_MAX_POWER_TABLE=ON -DOT_DAEMON=ON \
      -DOT_THREAD_VERSION=1.3 -DCMAKE_BUILD_TYPE=Release -DOT_RCP_RESTORATION_MAX_COUNT=10 -DOT_POSIX_CONFIG_RCP_BUS=UART
    $ ninjia

The ot-daemon is built in \${MY_OPENTHREAD}/src/posix/otbr-agent.
The ot-ctl for ot-daemon is built in \${MY_OPENTHREAD}/src/posix/ot-ctl.

Please rename the ot-ctl to ot-client-ctl and then copy ot-daemon and ot-client-ctl into target /usr/sbin/ directory.

<a name="How-to-setup-OTBR-OT"></a>

# How to setup OpenThread Border Router and Openther Deamon on the target

Use below commands to connect the OTBR to the Wi-Fi access point:

    # For i.MX8M Mini EVK and i.MX6ULL EVK with 88W8987 WiFi module, and for i.MX93 EVK with IW612 chipset:
    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ wpa_passphrase ${SSID} ${PASSWORD} > wifiap.conf
    $ wpa_supplicant -d -B -i mlan0 -c ./wifiap.conf
    $ udhcpc -i mlan0
    $ systemctl start otbr_fwcfg  #if no systemd installed, please use /usr/bin/otbr_fwcfg.sh instead

Then configure the Thread device:

On __i.MX8M Mini EVK__, __i.MX6ULL EVK__ or __i.MX8ULP EVK__, we will use a dedicated Thread device (NXP K32W or any third part RCP):
Plugin the Thread module into the USB OTG port of __i.MX8M Mini EVK__, __i.MX6ULL EVK__ or __i.MX8ULP EVK__. A USB device should be visible as _/dev/ttyUSB_ or _/dev/ttyACM_.
Once the USB device is detected, start te OTBR related services.

When using the RCP module, programmed with OpenThread Spinel firmware image, execute the following commands:

    # If you are using third-party reference RCP
    $ otbr-agent -I wpan0 -B mlan0 spinel+hdlc+uart:///dev/ttyACM0 &

    # If you are using K32W RCP
    $ otbr-agent -I wpan0 -B mlan0 'spinel+hdlc+uart:///dev/ttyUSB0?uart-baudrate=1000000' &
    $ iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $ iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $ otbr-web &

For __i.MX93 EVK__, we will use IW612 as Thread device:

    $ otbr-agent-iwxxx-spi -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip4&gpio-int-device=/dev/gpiochip5&gpio-int-line=10&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    $ iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $ iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $ otbr-web &

A document explaining how to use Matter with OTBR and OpenThread on the i.MX MPU platform can be found in the [NXP Matter demos guide](docs/guides/nxp_mpu_matter_demos.md).

<a name="How-to-build-Matter-application"></a>

# How to build Matter application

The Matter application has been installed into the Yocto image by default. If you want build it separately, run the below commands to download the Matter application source code and switch to v1.2.1 branch:

    $ mkdir ${MY_Matter_Apps}     # this is top level directory of this project
    $ cd ${MY_Matter_Apps}
    $ git clone https://github.com/NXP/matter.git
    $ cd matter
    $ git checkout origin/v1.3-branch-nxp_imx_2024_q2
    $ git submodule update --init

 ___Make sure the shell isn't in Yocto SDK environment___. Then, export a shell environment variable named IMX_SDK_ROOT to specify the path of the SDK.

    # For i.MX8M Mini EVK  #/opt/fsl-imx-xwayland/6.6-nanbield-imx8mm is ${IMX8MM_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.6-nanbield-imx8mm

    # For i.MX6ULL EVK     #/opt/fsl-imx-xwayland/6.6-nanbield-imx6ull is ${IMX6ULL_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.6-nanbield-imx6ull

    # For i.MX93 EVK  #/opt/fsl-imx-xwayland/6.6-nanbield-imx93 is ${IMX93_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.6-nanbield-imx93

    # For i.MX8ULP EVK  #/opt/fsl-imx-xwayland/6.6-nanbield-imx8ulp is ${IMX8ULP_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.6-nanbield-imx8ulp

User can build Matter applications (with the Yocto SDK specified by the IMX_SDK_ROOT) with the imxlinux_example.sh script. Please refer to below examples.

Assuming that the working directory is changed to the top level directory of this project.

    $ source scripts/activate.sh

    # Build the all-clusters example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/all-clusters-app/linux/ -o out/all-clusters -d

    # Build the lighting example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/lighting-app/linux/ -o out/lighting -d

    # Build the thermostat example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/thermostat/linux/ -o out/thermostat -d

    # Build the chip-tool example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/chip-tool -d

    # Build the ota-provider example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/ota-provider-app/linux/ -o out/ota-provider -d

    # Build the ota-requestor-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/ota-requestor-app/linux/ -o out/ota-requestor -d

    # Build the nxp-thermostat-app for certification device reference
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-thermostat/linux/ -o out/nxp-thermostat -d

    # Build the chip-bridge-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/bridge-app/linux/ -o out/bridge-app -d

    # Build the nxp-media-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-media-app/linux/ -o out/nxp-media -d

    # Build the chip-energy-management-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/energy-management-app/linux/ -o out/energy-management-app -d

    # Build the Matter Controller tool with enhanced security using imxlinux_example.sh, by adding "-t" to the target. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/imx-chip-tool-trusty -t

    # Build the Matter lighting app with enhanced security using imxlinux_example.sh, by adding "-t" to the target. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/lighting-app/linux -o out/imx-lighting-app-trusty -t

    # Build the NXP customized thermostat application with enhanced security using imxlinux_example.sh, by adding "-t" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-thermostat/linux -o out/nxp-thermostat-trusty -t

    # Build the NXP customized media application with enhanced security using imxlinux_example.sh, by adding "-t" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-media-app/linux/ -o out/nxp-media-trusty -t

    # Build the chip-tool-web application using imxlinux_example.sh, by adding "NXP_CHIPTOOL_WITH_WEB=1" to the command. For example:
    $ NXP_CHIPTOOL_WITH_WEB=1 ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/chip-tool-web -d

    # Build the chip-tool-web2 application using imxlinux_example.sh, by adding "NXP_CHIPTOOL_WITH_WEB2=1" to the command. For example:
    $ NXP_CHIPTOOL_WITH_WEB2=1 ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/chip-tool-web2 -d

    # Build the NXP customized Zigbee bridge application with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/bridge-app/nxp/linux-imx -o out/zigbee-bridge/

The applications are built in out/ subdirectories; the subdirectory name is specified with -o option, when building the examples. For example, the chip-all-clusters-app executable files can found in \${MY_Matter_Apps}/connectedhomeip/out/all-clusters/.

___Make sure the subdirectories do not exist before building an application with the same name.___
If an application needs to be built for several boards (both i.MX8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK), user can specify a board dedicated directory with -o option; for example:

    $ ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/imx8mm-chip-tool -d

After executing the above command, the chip-tool executable files will be found in ${MY_Matter_Apps}/out/imx8mm-chip-tool/.

An official Matter document explaining how to use chip-tool as a Matter controller can be found [here](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md).

A document explaining how to use Matter applications on the i.MX MPU platform can be found in the [NXP Matter demos guide](docs/guides/nxp_mpu_matter_demos.md). A document explaining how to use chip-tool-web application can be found in the [NXP chip-tool-web guide](docs/guides/nxp_chip_tool_web_guide.md). A document explaining how to use NXP customized Zigbee bridge application imx-chip-bridge-app application can be found in the [NXP imx-chip-bridge-app guide](https://github.com/NXP/matter/blob/v1.3-branch-nxp_imx_2024_q2/examples/bridge-app/nxp/linux-imx/README.md).

<a name="Security-configuration-for-Matter"></a>

# Security configuration for Matter

Since the i.MX Matter 2023 Q1 release, the hardware security feature is enabled to enhance the security of Matter on __i.MX8M Mini__. The certification attestation and P256Keypair keys are protected by the ARM Trustzone and stored into a secure storage based on the i.MX Matter security enhancement solution on i.MX8M Mini using [Trusty OS](https://source.android.com/docs/security/features/trusty) TEE. The design is based on the CSA Matter Attestation of Security Requirements document.

The i.MX Matter secure storage is based on eMMC RPMB. Initialising the secure storage and providing credentials is done using _fastboot_. Please download _fastboot_ from [SDK Platform-Tools](https://developer.android.com/studio/releases/platform-tools) and then add _fastboot_ to your _${PATH}_. In order to initialise the secure storage, please follow the instructions below:

    # Connect the OTG port of the i.MX8M Mini to the host PC.
    # Boot the i.MX8M Mini EVK board, during the U-Boot bootloader procedure, press any key on the target console to stop boot process and input U-Boot commands.
    u-boot=> fastboot 0

    # On host side, use fastboot command to initialise the RPMB partition as secure storage. Note that this is a one time programable partition and cannot be revoked.
    $ fastboot oem set-rpmb-hardware-key

    # Then provision the PAI, DAC, CD and DAC private key via _fastboot_ instructions on your host.
    $ fastboot stage <path-to-PAI-CERT>
    $ fastboot oem set-matter-pai-cert
    $ fastboot stage <path-to-DAC-CERT>
    $ fastboot oem set-matter-dac-cert
    $ fastboot stage <path-to-CD-CERT>
    $ fastboot oem set-matter-cd-cert
    $ fastboot stage <path-to-DAC-PRIVATE_KEY>
    $ fastboot oem set-matter-dac-private-key

    # You will see the following output from the target U-Boot console when it has been successfully provisioned:
    u-boot=> fastboot 0
    Starting download of 463 bytes downloading of 463 bytes finished
    Set matter pai cert successfully!
    Starting download of 491 bytes downloading of 491 bytes finished
    Set matter dac cert successfully!
    Starting download of 539 bytes downloading of 539 bytes finished
    Set matter cd cert successfully!
    Starting download of 32 bytes downloading of 32 bytes finished
    Set matter dac private key successfully!

Test attestation binary can be found in: _meta-nxp-connectivity/tools/test_attestation_

The Trusty OS, which contains the Trusted Application (TA) for i.MX Matter, is maintained by NXP and released as open source. Please follow below instructions to fetch the Trusty OS source code and build it:

    $ repo init -u https://github.com/nxp-imx/imx-manifest.git -b imx-trusty-matter -m imx_trusty_matter_2023_q2.xml
    $ repo sync -c

    # Setup the build environment. This will only configure the current terminal.
    $ source trusty/vendor/google/aosp/scripts/envsetup.sh

    # Build the i.MX8M Mini Trusty OS binary:
    $ ./trusty/vendor/google/aosp/scripts/build.py imx8mm --dynamic_param BUILD_MATTER=true
    # The target binary will be put on: build-root/build-imx8mm/lk.bin

    # Enable the secure storage service on first boot on i.MX8M Mini Linux shell
    $ systemctl enable storageproxyd
    $ systemctl start storageproxyd

Since the i.MX Matter 2023 Q3 release, the built-in security subsystem [ELE (EdgeLock Secure Enclave)](https://www.nxp.com/products/nxp-product-information/nxp-product-programs/edgelock-secure-enclave:EDGELOCK-SECURE-ENCLAVE) is integrated to enhance the security of Matter on __i.MX93__. To enable ELE, please start the nvm_daemon service on the i.MX93 Linux shell after each power-up.

    $ systemctl start nvm_daemon

<a name="FAQ"></a>

# FAQ

Q1 : why "zstd -d imx-image-multimedia-imx8mmevk.wic.zst" command cannot be executed in the folder ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/ ?

A : Because imx-image-multimedia-imx8mmevk.wic.zst is a link file; please zstd the link target file or copy imx-image-multimedia-imx8mmevk.wic.zst to another folder, then uncompress it using zstd.

    $ ls -al
    imx-image-multimedia-imx8mmevk.wic.zst -> imx-image-multimedia-imx8mmevk-20220721181418.rootfs.wic.zst

Q2 : What if the Yocto SDK Python3 is exported into the shell environment and makes the Matter bootstrap/active process fail?

A : Open a new shell, then remove the Yocto SDK environment and initialise the applications build enviroment.

    $ cd ${MY_Matter_Apps}
    $ rm -rf .environment
    $ source scripts/activate.sh

Q3 : How to download the official PAA files from CSA and how to use the official PAA files?

A : Connect the i.MX Matter device to a network that can access CSA resources, and then execute the following command. This will store the PAA files in the "/etc/dcl_paas" directory.

    $ dcldownloader

You can configure the macro with the following command, and then directly execute the chip-tool command directly to use it.

    $ export CHIPTOOL_PAA_TRUST_STORE_PATH=/etc/dcl_paas
    $ ${chip-tool command}

Another way is to add a suffix when executing the command, as shown below:

    $ ${chip-tool command} --paa-trust-store-path /etc/dcl_paas

>  **Note** If you are using the official PAA files, the end Matter device must have the official DAC and PAI installed.

Q4 : How to save the commission information so that the board does not need a commission process after a reboot?

A : You can save the commission information by below command:

    $ mkdir -p /etc/matter && export TMPDIR=/etc/matter

    After rebooting the device, re-export the `TMPDIR` environment by below command:

    $ export TMPDIR=/etc/matter
