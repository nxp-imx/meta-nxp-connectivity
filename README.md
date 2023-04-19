# Contents

[**Introduction**](#Introduction)

[**i.MX MPU Matter platform**](#imx-mpu-platform)

[**How to build the Yocto image with integrated OpenThread Border Router**](#How-to-build-the-Yocto-image)

[**How to build OpenThread Border Router with Yocto SDK**](#How-to-build-OTBR)

[**How to setup OpenThread Border Router environment within the Yocto**](#How-to-setup-OTBR)

[**How to build Matter application**](#How-to-build-Matter-application)

[**Security configuration for Matter**](#Security-configuration-for-Matter)

[**FAQ**](#FAQ)

<a name="Introduction"></a>

# Introduction
This repository contains the i.MX MPU project Matter related Yocto recipes. The following modules will be built with this meta-matter layer:
 - Matter (CHIP) : https://github.com/nxp-imx/matter.git
 - OpenThread Daemon: https://github.com/openthread/openthread
 - OpenThread Border Router: https://github.com/openthread/ot-br-posix

All the software components revisions are based on [project Matter SVE_23_03/rc2 tag](https://github.com/nxp-imx/matter.git).

The Following Matter related binaries will be installed into the Yocto image root filesystem by this Yocto layer recipes:
 - chip-lighting-app: Matter lighting app demo
 - chip-lighting-app-trusty: Matter lighting app with enhanced security on i.MX8M Mini
 - chip-all-clusters-app: Matter all-clusters demo
 - thermostat-app: Matter thermostat demo
 - nxp-thermostat-app: NXP customized thermostat application which used for Matter Certification
 - nxp-thermostat-app-trusty: NXP customized thermostat application with enhanced security on i.MX8M Mini
 - chip-bridge-app: Matter bridge demo
 - chip-tool: Matter Controller tools
 - chip-tool-trusty: Matter Controller tools with enhanced security for i.MX8M Mini
 - ot-daemon: OpenThread Daemon for OpenThread client
 - ot-client-ctl: OpenThread ctrl tool for OpenThread client
 - otbr-agent: OpenThread Border Router agent
 - ot-ctl: OpenThread Border Router ctrl tool
 - otbr-web: OpenThread Border Router web management daemon

<a name="imx-mpu-platform"></a>

# i.MX MPU Matter platform

We currently support 3 i.MX MPU platforms, which are the i.MX93 EVK, the i.MX8M Mini EVK, and the i.MX6ULL EVK. For more details, please refer to the [NXP i.MX MPU Matter Platform](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/mpu-linux-hosted-matter-development-platform:MPU-LINUX-MATTER-DEV-PLATFORM).

<a name="How-to-build-the-Yocto-image"></a>

# How to build the Yocto image with integrated OpenThread Border Router

The following packages are required to build the Yocto Project:

    $ sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
    build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev \
    pylint3 xterm npm zstd build-essential libpython3-dev libdbus-1-dev python3.8-venv

The following python packages need to be installed:

    $ pip3 install testresources build mypy==0.910 types-setuptools pylint==2.9.3
    $ wget https://raw.githubusercontent.com/project-chip/connectedhomeip/SVE_23_03/rc2/scripts/setup/constraints.txt
    $ pip3 install -r constraints.txt
    $ pip install dbus-python

Make sure your default Python of the Linux host is Python2:

    $ python --version
      Python 2.7.18

Then, Yocto build environment must be setup.

The Yocto source code is maintained with a manifest file, used by repo tool to download the corresponding source code.
This document is tested with the i.MX Yocto 6.1.1_1.0.0 release. The hardware tested are: i.MX 8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK.
Run the commands below to download this release:

    $ mkdir ~/bin
    $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
    $ chmod a+x ~/bin/repo
    $ export PATH=${PATH}:~/bin

    $ mkdir ${MY_YOCTO} # this directory will be the top directory of the Yocto source code
    $ cd ${MY_YOCTO}
    $ repo init -u https://github.com/nxp-imx/imx-manifest -b imx-linux-langdale -m imx-6.1.1-1.0.0.xml
    $ repo sync

Then integrate the meta-matter recipes into the Yocto code base

    $ cd ${MY_YOCTO}/sources/
    $ git clone https://github.com/nxp-imx/meta-matter.git

More information about the downloaded Yocto release can be found in the corresponding i.MX Yocto Project User’s Guide, which can be found at [NXP official website](http://www.nxp.com/imxlinux).

Change the current directory to the top directory of the Yocto source code and execute the command below:

    # For i.MX8M Mini EVK:
    $ MACHINE=imx8mmevk-matter DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx8mm
    # For i.MX8M Mini EVK which use Firecrest (IW612) module (Note this operation will switch meta-imx and meta-matter repo to specific revision):
    $ TARGET_15_4_CHIP=IW612 OT_RCP_BUS=SPI MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx8mm
    # For i.MX6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx6ull
    # For i.MX93 EVK:
    $ MACHINE=imx93evk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx93

This will create a build directory (namely bld-xwayland-imx8mm/ for i.MX8M Mini EVK, bld-xwayland-imx6ull/ for i.MX6ULL EVK or bld-xwayland-imx93/ for i.MX93 EVK), and enter this directory automatically. Please execute the command below to generate the Yocto images:

    $ bitbake imx-image-multimedia

After execution of previous commands, the Yocto images will be generated:
- ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk.wic.zst for i.MX8M Mini EVK.
- ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst for i.MX6ULL EVK.
- ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk/imx-image-multimedia-imx93evk.wic.zst for i.MX93 EVK.

The zst images are symbolic link files, so you should copy them to a dedicated folder ${MY_images} before unziping them.

    # For i.MX8M Mini EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk.wic.zst ${MY_images}

    # For i.MX6ULL EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst ${MY_images}

    # For i.MX93 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk/imx-image-multimedia-imx93evk.wic.zst ${MY_images}

Please use zstd command unzip this .zst archive, and then dd command to program the output file to a microSD card. This microSD card will be used to boot the image on an i.MX 8M Mini EVK, i.MX6ULL EVK or i.MX93 EVK.

___Be cautious when executing the dd command below, making sure that the output ("of" parameter) represents the microSD card device! /dev/sdc in the below command represents a microSD card connected to the host machine with a USB adapter; however the output device name may vary. Please use "ls /dev/sd*" command to verify the name of the SD card device.___

    $ cd ${MY_images}

    # For i.MX8M Mini EVK:
    $ zstd -d imx-image-multimedia-imx8mmevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx8mmevk.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX6ULL EVK:
    $ zstd -d imx-image-multimedia-imx6ullevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx6ullevk.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX93 EVK:
    $ zstd -d imx-image-multimedia-imx93evk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx93evk.wic of=/dev/sdc bs=4M conv=fsync

<a name="How-to-build-OTBR"></a>

# How to build OpenThread Border Router with the Yocto SDK

There are 3 modules for OpenThread Border Router (OTBR): otbr-agent, ot-ctl and otbr-web. The otbr-web module requires liboost static and jsoncpp packages, which are not included in the default Yocto images.

To build these binaries, we need to use the Yocto SDK toolchain with meta-matter included.
This SDK can be generated with below commands:

    # For i.MX8M Mini EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx8mm

    # For i.MX6ULL EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx6ull

    # For i.MX93 EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx93

    $ bitbake imx-image-multimedia -c populate_sdk

Then, install the Yocto SDK, by running the SDK installation script with root permission:

    # For i.MX8M Mini EVK:
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-6.1-langdale.sh

    # For i.MX6ULL EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-cortexa7t2hf-neon-imx6ullevk-toolchain-6.1-langdale.sh

    # For i.MX93 EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx93evk-toolchain-6.1-langdale.sh

The SDK installation directory will be prompted during the SDK installation; user can specify the installation directory, or keep the default one \${/opt/fsl-imx-xwayland/}.
___Please use board specific paths if you need to build the SDK for several boards EVK); for exmaple, you can use /opt/fsl-imx-xwayland/6.1-langdale-imx8mm for i.MX8M Mini EVK SDK, /opt/fsl-imx-xwayland/6.1-langdale-imx6ull for i.MX6ULL EVK and /opt/fsl-imx-xwayland/6.1-langdale-imx93 for i.MX93 EVK.___

    NXP i.MX Release Distro SDK installer version 6.1-langdale
    ============================================================
    Enter target directory for SDK (default: /opt/fsl-imx-xwayland/6.1-langdale):

After the Yocto SDK is installed on the host machine, an SDK environment setup script is also generated.
User needs to import Yocto build environment, by sourcing this script each time the SDK is used in a new shell; for example:

    # For i.MX8M Mini EVK
    $ . /opt/fsl-imx-xwayland/6.1-langdale-imx8mm/environment-setup-armv8a-poky-linux

    $ For i.MX6ULL EVK
    $ . /opt/fsl-imx-wayland/6.1-langdale-imx6ull/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi

    # For i.MX93 EVK
    $ . /opt/fsl-imx-wayland/6.1-langdale-imx93/environment-setup-armv8a-poky-linux

Fetch the latest otbr source code and execute the build:

    $ mkdir ${MY_OTBR}  # this directory will be the top directory of the OTBR source code
    $ cd ${MY_OTBR}
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git checkout origin/main
    $ git submodule update --init

    # For i.MX8M Mini EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake

    # For i.MX6ULL EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/arm.cmake

    # For i.MX93 EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DOT_POSIX_CONFIG_RCP_BUS=SPI -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake

The otbr-agent is built in \${MY_OTBR}/build/otbr/src/agent/otbr-agent.
The otbr-web is built in \${MY_OTBR}/build/otbr/src/web/otbr-web.
The ot-ctl is built in \${MY_OTBR}/build/otbr/third_party/openthread/repo/src/posix/ot-ctl.

Please copy them into target /usr/sbin/ directory.

__The OTBR does not support incremental compilation. If an error occurs during compilation, or if you need to recompile, please delete ${MY_OTBR}/build before recompiling.__

    $ cd ${MY_OTBR}
    $ rm -rf build/

<a name="How-to-setup-OTBR"></a>

# How to setup OpenThread Border Router on the target

Use below commands to connect the OTBR to the Wi-Fi access point:

    # For i.MX8M Mini EVK and i.MX6ULL EVK with 88W8987 WiFi module, and for i.MX93 EVK with IW612 module:
    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ wpa_passphrase ${SSID} ${PASSWORD} > imxrouter.conf
    $ wpa_supplicant -d -B -i mlan0 -c ./imxrouter.conf
    $ udhcpc -i mlan0
    $ service otbr_fwcfg start  #if no systemd installed, please use /usr/bin/otbr_fwcfg.sh instead

Then configure the Thread device:

On __i.MX8M Mini EVK__ or __i.MX6ULL EVK__, we will use a dedicated Thread device (NXP K32W or any third part RCP):
Plugin the Thread module into the USB OTG port of __i.MX8M Mini EVK__ or __i.MX6ULL EVK__. A USB device should be visible as _/dev/ttyUSB_ or _/dev/ttyACM_.
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
We just need set a GPIO and then setup the otbr.

    $ gpioset gpiochip6 0=1
    $ otbr-agent -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip6&gpio-int-device=/dev/gpiochip4&gpio-int-line=10&gpio-reset-line=1&
    spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    $ iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $ iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $ otbr-web &

A document explaining how to use Matter with OTBR on the i.MX MPU platform can be found in the [NXP Matter binaries guide](docs/guides/nxp_mpu_matter_binaries.md).

<a name="How-to-build-Matter-application"></a>

# How to build Matter application

The Matter application has been installed into the Yocto image by default. If you want build it separately, run the below commands to download the Matter application source code and switch to v1.0 branch:

    $ mkdir ${MY_Matter_Apps}     # this is top level directory of this project
    $ cd ${MY_Matter_Apps}
    $ git clone https://github.com/NXP/matter.git
    $ cd matter
    $ git checkout origin/v1.0-branch-nxp_imx_2023_q1
    $ git submodule update --init

 ___Make sure the shell isn't in Yocto SDK environment___. Then, export a shell environment variable named IMX_SDK_ROOT to specify the path of the SDK.

    # For i.MX8M Mini EVK  #/opt/fsl-imx-xwayland/6.1-langdale-imx8mm is ${IMX8MM_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.1-langdale-imx8mm

    # For i.MX6ULL EVK     #/opt/fsl-imx-xwayland/6.1-langdale-imx6ull is ${IMX6ULL_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.1-langdale-imx6ull

    # For i.MX93 EVK  #/opt/fsl-imx-xwayland/6.1-langdale-imx93 is ${IMX93_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.1-langdale-imx93

User can build Matter applications (with the Yocto SDK specified by the IMX_SDK_ROOT) with the build_examples.py script. Please refer to below examples.

Assuming that the working directory is changed to the top level directory of this project.

    $ source scripts/activate.sh

    # Build the all-clusters example with below command
    $ ./scripts/build/build_examples.py --target imx-all-clusters-app build

    # Build the lighting example with below command
    $ ./scripts/build/build_examples.py --target imx-lighting-app  build

    # Build the thermostat example with below command
    $ ./scripts/build/build_examples.py  --target imx-thermostat build

    # Build the chip-tool example with below command
    $ ./scripts/build/build_examples.py  --target imx-chip-tool build

    # Build the ota-provider example with below command
    $ ./scripts/build/build_examples.py  --target imx-ota-provider-app build

    # Build the nxp-thermostat-app for certification device reference
    $ ./scripts/examples/imxlinux_example.sh examples/nxp-thermostat/linux/ out/nxp-thermostat debug

    # Build the chip-bridge-app example with below command
    $ ./scripts/examples/imxlinux_example.sh examples/bridge-app/linux/ out/bridge-app debug

    # Build the security enhanced with Trusty OS application using build_examples.py, by adding "-trusty" to te target. For example:
    $ ./scripts/build/build_examples.py  --target imx-chip-tool-trusty build

    # Build the security enhanced with Trusty OS application using imxlinux_example.sh, by adding "trusty" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh examples/nxp-thermostat/linux out/nxp-thermostat-trusty trusty

The applications are built in out/ subdirectories; the subdirectory name is specified with --target option, when building the examples. For example, the imx-all-clusters-app executable files can found in \${MY_Matter_Apps}/connectedhomeip/out/imx-all-clusters-app/.

___Make sure the subdirectories do not exist before building an application with the same name.___
If an application needs to be built for several boards (both i.MX8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK), user can specify a board dedicated directory with --out-prefix option; for example:

    $./scripts/build/build_examples.py  --target imx-chip-tool --out-prefix ./out/imx8mm build

After executing the above command, the chip-tool executable files will be found in ${MY_Matter_Apps}/out/imx8mm/imx-chip-tool/.

An official Matter document explaining how to use chip-tool as a Matter controller can be found [here](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md).

A document explaining how to use Matter applications on the i.MX MPU platform can be found in the [NXP Matter binaries guide](docs/guides/nxp_mpu_matter_binaries.md).

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

Test attestation binary can be found in: _meta-matter/tools/test_attestation_

The Trusty OS, which contains the Trusted Application (TA) for i.MX Matter, is maintained by NXP and released as open source. Please follow below instructions to fetch the Trusty OS source code and build it:

    $ repo init -u https://github.com/nxp-imx/imx-manifest.git -b imx_matter_2023_q1 -m imx-trusty-matter-2023-q1.xml
    $ repo sync -c

    # Setup the build environment. This will only configure the current terminal.
    $ source trusty/vendor/google/aosp/scripts/envsetup.sh

    # Build the i.MX8M Mini Trusty OS binary:
    $ ./trusty/vendor/google/aosp/scripts/build.py imx8mm
    # The target binary will be put on: build-root/build-imx8mm/lk.bin

    # Enable the secure storage service on first boot on i.MX8M Mini Linux shell
    $ systemctl enable storageproxyd
    $ systemctl start storageproxyd

<a name="FAQ"></a>

# FAQ

Q1 : Why do the npm EAI_AGAIN error occur in the bitbake process and how to solve it?

     | npm ERR! code EAI_AGAIN
     | npm ERR! errno EAI_AGAIN
     | npm ERR! request to https://registry.npmjs.org/angular failed, reason:reason: getaddrinfo EAI_AGAIN registry.cnpmjs.org registry.cnpmjs.org:80

A : This npm EAI_AGAIN error occurs when otbr is compiled more than once. The otbr intermediate compilation files should be cleared before executing the bitbake command again.

     $ bitbake -c cleanall otbr
     $ bitbake imx-image-multimedia

Q2 : why "zstd -d imx-image-multimedia-imx8mmevk.wic.zst" command cannot be executed in the folder ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/ ?

A : Because imx-image-multimedia-imx8mmevk.wic.zst is a link file; please zstd the link target file or copy imx-image-multimedia-imx8mmevk.wic.zst to another folder, then uncompress it using zstd.

    $ ls -al
    imx-image-multimedia-imx8mmevk.wic.zst -> imx-image-multimedia-imx8mmevk-20220721181418.rootfs.wic.zst

Q3 : How to solve the issue bellow?

    FAILED: src/web/web-service/frontend/CMakeFiles/otbr-web-frontend /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/build/otbr/src/web/web-service/frontend/CMakeFiles/otbr-web-frontend
    cd /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/build/otbr/src/web/web-service/frontend && cp /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/src/web/web-service/frontend/package.json . && npm install
    ERROR: npm is known not to run on Node.js v10.19.0
    You'll need to upgrade to a newer Node.js version in order to use this
    version of npm. You can find the latest version at https://nodejs.org/

A : Update the node to the latest version.

    $ curl  https://raw.githubusercontent.com/creationix/nvm/master/install.sh | bash
    $ source ~/.profiles       or     source ~/.bashrc        #You can choose one of them as prompted by the previous command
    $ nvm install 18
    $ node --version  # make sure that it was successfully installed.

Q4 : What if the Yocto SDK Python3 is exported into the shell environment and makes the Matter bootstrap/active process fail?

A : Open a new shell, then remove the Yocto SDK environment and initialise the applications build enviroment.

    $ cd ${MY_Matter_Apps}
    $ rm -rf .environment
    $ source scripts/activate.sh

