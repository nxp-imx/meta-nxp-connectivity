# Introduction
This repo contains the i.MX MPU project Matter related Yocto recipes. Below is a list of modules that will be built with the meta-matter repo integrated.
 - Matter (CHIP) : https://github.com/nxp-imx/matter.git
 - OpenThread Daemon: https://github.com/openthread/openthread
 - OpenThread Border Router: https://github.com/openthread/ot-br-posix

All the software components revision are based on [project Matter v1.0-branch](https://github.com/nxp-imx/matter.git).

The Following Matter related binaries will be installed into the Yocto image root filesystem with this recipe repo:
 - chip-lighting-app: Matter lighting app demo
 - chip-lighting-app-trusty: Matter lighting app with security enhance on i.MX8M Mini
 - chip-all-clusters-app: Matter all-clusters demo
 - thermostat-app: Matter thermostat demo
 - nxp-thermostat-app: NXP customized thermostat application which used for Matter Certification
 - nxp-thermostat-app-trusty: NXP customized thermostat application with security enhance on i.MX8M Mini
 - chip-bridge-app: Matter bridge demo
 - chip-tool: Matter Controller tools
 - chip-tool-trusty: Matter Controller tools with security enhance on i.MX8M Mini
 - ot-daemon: OpenThread Daemon for OpenThread client
 - ot-client-ctl: OpenThread ctrl tool for OpenThread client
 - otbr-agent: OpenThread Border Router agent
 - ot-ctl: OpenThread Border Router ctrl tool
 - otbr-web: OpenThread Border Router web management daemon

# How to build the Yocto image with integrated OpenThread Border Router

To build the Yocto Project, some packages need to be installed. The list of packages required are:

    $ sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
    build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev \
    pylint3 xterm npm zstd build-essential libpython3-dev libdbus-1-dev python3.8-venv

To build the Yocto Project, some python dependency packages need to be installed.

    $ pip3 install testresources build mypy==0.910 types-setuptools pylint==2.9.3
    $ wget https://raw.githubusercontent.com/project-chip/connectedhomeip/v1.0.0/scripts/constraints.txt
    $ pip3 install -r constraints.txt
    $ pip install dbus-python

Then, Yocto build environment must be setup.

The Yocto source code is maintained with a repo manifest, the tool repo is used to download the source code.
This document is tested with the i.MX Yocto 5.15.71_2.2.0 release. The hardware tested in this documentation is the i.MX 8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK.
Run the commands below to download this release:

    $ mkdir ~/bin
    $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
    $ chmod a+x ~/bin/repo
    $ export PATH=${PATH}:~/bin
    
    $ mkdir ${MY_YOCTO} # this directory will be the top directory of the Yocto source code
    $ cd ${MY_YOCTO}
    $ repo init -u https://github.com/nxp-imx/imx-manifest -b imx-linux-kirkstone -m imx-5.15.71-2.2.0.xml
    $ repo sync
Then integrate the meta-matter recipe into the Yocto code base

    $ cd ${MY_YOCTO}/sources/
    $ git clone https://github.com/nxp-imx/meta-matter.git

More information about the downloaded Yocto release can be found in the corresponding i.MX Yocto Project User’s Guide which can be found at [NXP official website](http://www.nxp.com/imxlinux).

Make sure your default Python of the Linux host is Python2.

    $ python --version
      Python 2.7.18

Change the current directory to the top directory of the Yocto source code and execute the command below.

    #For i.MX8M Mini EVK:
    $ MACHINE=imx8mmevk-matter DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx8mm
    #For i.MX8M Mini EVK which use Firecrest (IW612) module (Note this operation will switch meta-imx and meta-matter repo to specific revision):
    $ TARGET_15_4_CHIP=IW612 OT_RCP_BUS=SPI MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx8mm
    #For i.MX6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx6ull
    #For i.MX93 EVK:
    $ MACHINE=imx93evk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-matter-setup.sh bld-xwayland-imx93

The system will create a directory bld-xwayland-imx8mm/ for i.MX8M Mini EVK, bld-xwayland-imx6ull/ for i.MX6ULL EVK or bld-xwayland-imx93/ for i.MX93 EVK and enter this directory automatically, execute the command below under these directory to generate the Yocto images.

    $ bitbake imx-image-multimedia

After execution of previous commands, the Yocto images will be generated under ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/imx-image-multimedia-imx8mmevk.wic.zst for i.MX8M Mini EVK.

And ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst for i.MX6ULL EVK.

And ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk/imx-image-multimedia-imx93evk.wic.zst for i.MX93 EVK.

The zst images are symbolic link files, so you should copy it to another fold ${MY_images} before unzip it.

    #For i.MX8M Mini EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk.wic.zst ${MY_images}
    #For i.MX6ULL EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst ${MY_images}
    #For i.MX93 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk/imx-image-multimedia-imx93evk.wic.zst ${MY_images}

The zstd command should be used to unzip this file then the dd command should be used to program the output file to a microSD card by running the commands below. Then a microSD card can be used to boot the image of an i.MX 8M Mini EVK, i.MX6ULL EVK or i.MX93 EVK.

___Be cautious when executing the dd command below, make sure the of represents the microSD card device!, /dev/sdc in the command below represents a microSD card connected to the host machine with a USB adapter, however the output device name may vary. Use the command "ls /dev/sd*" to verify the name of the SD card device.___

    $ cd ${MY_images}
    #For i.MX8M Mini EVK:
    $ zstd -d imx-image-multimedia-imx8mmevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx8mmevk.wic of=/dev/sdc bs=4M conv=fsync
    #For i.MX6ULL EVK:
    $ zstd -d imx-image-multimedia-imx6ullevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx6ullevk.wic of=/dev/sdc bs=4M conv=fsync
    #For i.MX93 EVK:
    $ zstd -d imx-image-multimedia-imx93evk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx93evk.wic of=/dev/sdc bs=4M conv=fsync

# How to build OpenThread Border Router with Yocto SDK
There are 3 module for OpenThread Border Router (OTBR): otbr-agent, ot-ctl and otbr-web. The otbr-web need liboost static and jsoncpp modules which are not included into default built Yocto images.

To build these binaries, the Yocto SDK with meta-matter generated must be used. This SDK can be generated by below command:

    #For i.MX8M Mini EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx8mm
    #For i.MX6ULL EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx6ull
    #For i.MX93 EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx93

    $bitbake imx-image-multimedia -c populate_sdk

Install the NXP Yocto SDK and set toolchain environment variables.
Run the SDK installation script with root permission.

    #For i.MX8M Mini EVK:
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh
    #For i.MX6ULL EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-cortexa7t2hf-neon-imx6ullevk-toolchain-5.15-kirkstone.sh
    #For i.MX93 EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx93evk-toolchain-5.15-kirkstone.sh

The default target directory for SDK will be prompted during SDK installation, as shown below，enter a new target directory while the path \${/opt/fsl-imx-xwayland/} is required.
___If you need build SDK both for i.MX8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK on a host machine, it is necessary to distinguish the paths. As a reference, you can use /opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm for i.MX8M Mini EVK SDK, /opt/fsl-imx-xwayland/5.15-kirkstone-imx6ull for i.MX6ULL EVK and /opt/fsl-imx-xwayland/5.15-kirkstone-imx93 for i.MX93 EVK.___

    NXP i.MX Release Distro SDK installer version 5.15-kirkstone
    ============================================================
    Enter target directory for SDK (default: /opt/fsl-imx-xwayland/5.15-kirkstone):

After the Yocto SDK is installed on the host machine, an environment setup script is also generated, and there are prompt lines telling the user to source the script each time when using the SDK in a new shell, for example:

    #For i.MX8M Mini EVK
    $ . /opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm/environment-setup-armv8a-poky-linux
    $For i.MX6ULL EVK
    $ . /opt/fsl-imx-wayland/5.15-kirkstone-imx6ull/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
    #For i.MX93 EVK
    $ . /opt/fsl-imx-wayland/5.15-kirkstone-imx93/environment-setup-armv8a-poky-linux

After the SDK package installed in the build machine, import the Yocto build environment using the command:

    #For i.MX8M Mini EVK
    $source ${iMX8MM_SDK_INSTALLED_PATH}/environment-setup-armv8a-poky-linux
    #For i.MX6ULL EVK
    $source ${iMX6ULL_SDK_INSTALLED_PATH}/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
    #For i.MX93 EVK
    $source ${iMX93_SDK_INSTALLED_PATH}/environment-setup-armv8a-poky-linux

Fetch latest otbr source codes and execute the build:

    $ mkdir ${MY_OTBR}  # this directory will be the top directory of the OTBR source code
    $ cd ${MY_OTBR}
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git checkout origin/main
    $ git submodule update --init
    #For i.MX8M Mini EVK and i.MX93 EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake
    #For i.MX6ULL EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
     -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/arm.cmake

The otbr-agent is built in \${MY_OTBR}/build/otbr/src/agent/otbr-agent. 

The otbr-web is built in \${MY_OTBR}/build/otbr/src/web/otbr-web.

The ot-ctl is built in \${MY_OTBR}/build/otbr/third_party/openthread/repo/src/posix/ot-ctl.

Please copy them into Yocto's /usr/sbin/.

__The OTBR does not support incremental compilation. If there is an error occur in the compilation process or you need to recompile, please delete ${MY_OTBR}/build and recompile.__

    $ cd ${MY_OTBR}
    $ rm -rf build/

# How to setup OpenThread Border Router environment within the Yocto

After the OTBR boot, the __i.MX8M Mini EVK__, __i.MX6ULL EVK__ or __i.MX93 EVK__ must connect the OTBR to the target Wi-Fi AP network.

    # For i.MX8M Mini EVK and i.MX6ULL EVK with 88W8987 WiFi module
    $modprobe moal mod_para=nxp/wifi_mod_para.conf
    # For i.MX93 EVK with IW612 module:
    $modprobe sdxxx mod_para=nxp/wifi_mod_para.conf
    $wpa_passphrase ${SSID} ${PASSWORD} > imxrouter.conf
    $wpa_supplicant -d -B -i mlan0 -c ./imxrouter.conf
    $udhcpc -i mlan0
    $service otbr_fwcfg start  #if no systemd installed, please use /usr/bin/otbr_fwcfg.sh instead

Plugin the Thread module into the USB OTG port of __i.MX8M Mini EVK__ or __i.MX6ULL EVK__. A USB device should be visible as _/dev/ttyUSB_ or _/dev/ttyACM_.
Once the USB device is detected, start te OTBR related services.

When using the RCP module, programmed with OpenThread Spinel firmware image, execute the following commands:

    #If you are using third-party reference RCP
    $otbr-agent -I wpan0 -B mlan0 spinel+hdlc+uart:///dev/ttyACM0 &
    #If you are using K32W RCP
    $otbr-agent -I wpan0 -B mlan0 'spinel+hdlc+uart:///dev/ttyUSB0?uart-baudrate=1000000' &
    $iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $otbr-web &

For i.MX93 EVK, you do not need plugin the RCP and just need set gpio and then setup the otbr.

    $gpioset gpiochip6 0=1
    $otbr-agent -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip6&gpio-int-device=/dev/gpiochip4&gpio-int-line=10&gpio-reset-line=1&
    spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    $iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $otbr-web &

# How to build Matter application

The Matter application has be installed into the Yocto image defaultly. If you want build it separately. Run the commands below to download the Matter application code and switch to v1.0 branch:

    $ mkdir ${MY_Matter_Apps}     # this is top level directory of this project
    $ cd ${MY_Matter_Apps}
    $ git clone https://github.com/NXP/matter.git
    $ cd matter
    $ git checkout origin/v1.0-branch-nxp_imx_2023_q1
    $ git submodule update --init

 ___Make sure the shell isn't in Yocto SDK environment___. Then, export a shell environment variable named IMX_SDK_ROOT to specify the path of the SDK.

    #For i.MX8M Mini EVK  #/opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm is ${IMX8MM_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm
    #For i.MX6ULL EVK     #/opt/fsl-imx-xwayland/5.15-kirkstone-imx6ull is ${IMX6ULL_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-kirkstone-imx6ull
    #For i.MX93 EVK  #/opt/fsl-imx-xwayland/5.15-kirkstone-imx93 is ${IMX93_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-kirkstone-imx93

Building those apps with the Yocto SDK specified by the IMX_SDK_ROOT has been integrated into the tool build_examples.py. Use it to build the examples.

Assuming that the working directory is changed to the top level directory of this project.

    $ source scripts/activate.sh

    #If the all-clusters example is to be built
    $ ./scripts/build/build_examples.py --target imx-all-clusters-app build

    #If the lighting example is to be built
    $ ./scripts/build/build_examples.py --target imx-lighting-app  build

    #If the thermostat example is to be built
    $ ./scripts/build/build_examples.py  --target imx-thermostat build

    #If the chip-tool example is to be built
    $ ./scripts/build/build_examples.py  --target imx-chip-tool build

    #If the ota-provider example is to be built
    $ ./scripts/build/build_examples.py  --target imx-ota-provider-app build

    #If the nxp-thermostat-app is to be built for certification device reference
    $ ./scripts/examples/imxlinux_example.sh examples/nxp-thermostat/linux/ out/nxp-thermostat debug

    #If the chip-bridge-app example is to be built
    $ ./scripts/examples/imxlinux_example.sh examples/bridge-app/linux/ out/bridge-app debug

    #If the security enhanced with Trusty OS application to be built using build_examples.py, could simply add "-trusty" to te target. For example:
    $ ./scripts/build/build_examples.py  --target imx-chip-tool-trusty build

    #If the security enhanced with Trusty OS application to be built using imxlinux_example.sh, append "trusty" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh examples/nxp-thermostat/linux out/nxp-thermostat-trusty trusty


The apps are built in the subdirectories under out/, the subdirectory name is the same as the argument specified after the option --target when build the examples. For example, the imx-all-clusters-app executable files can found in \${MY_Matter_Apps}/connectedhomeip/out/imx-all-clusters-app/.

___Make sure the subdirectories isn't exist before build the same name app.___
If an app needs to be built both for i.MX8M Mini EVK, i.MX6ULL EVK and i.MX93 EVK, you can use the option --out-prefix to specify a subdirectory, for example:

    $./scripts/build/build_examples.py  --target imx-chip-tool --out-prefix ./out/imx8mm build

After executing the above command, the chip-tool executable files will be found in ${MY_Matter_Apps}/out/imx8mm/imx-chip-tool/.

A Matter official document about how to use chip-tool as Matter controller can be found in [here](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md).

# Security configuration for Matter

Since the i.MX Matter 2023 Q1 release, the hardware backed security function is enabled to enhance the security of Matter on __i.MX8M Mini__. Now the attestation and P256Keypair are protected by ARM Trustzone and secure storage based on the i.MX Matter security enhancement solution on i.MX8M Mini using [Trusty OS](https://source.android.com/docs/security/features/trusty) TEE. The design is based on the CSA Matter Attestation of Security Requirements document.

The i.MX Matter secure storage is based on eMMC RPMB. Initialising the secure storage and providing credentials is based on _fastboot_ you can get the _fastboot_ from [SDK Platform-Tools](https://developer.android.com/studio/releases/platform-tools) and add the _fastboot_ to your _${PATH}_. In order to initialise the secure storage, please follow the instructions below.

    #Connect the OTG port of the i.MX8M Mini to the host PC.
    #Boot the i.MX8M Mini EVK board, during the U-Boot procedure, press any key on the console to input U-Boot commands.
    u-boot=> fastboot 0

    #On your host, use fastboot command to initialise the RPMB partition as secure storage. Note that this is the one time program partation and cannot be revoked.
    $ fastboot oem set-rpmb-hardware-key

    #Then provision the PAI, DAC, CD and DAC private key via _fastboot_ instructions on your host.
    $ fastboot stage <path-to-PAI-CERT>
    $ fastboot oem set-matter-pai-cert
    $ fastboot stage <path-to-DAC-CERT>
    $ fastboot oem set-matter-dac-cert
    $ fastboot stage <path-to-CD-CERT>
    $ fastboot oem set-matter-cd-cert
    $ fastboot stage <path-to-DAC-PRIVATE_KEY>
    $ fastboot oem set-matter-dac-private-key

    #You will see the following output from the U-Boot console when it has been successfully provisioned:
    u-boot=> fastboot 0
    Starting download of 463 bytes downloading of 463 bytes finished
    Set matter pai cert successfully!
    Starting download of 491 bytes downloading of 491 bytes finished
    Set matter dac cert successfully!
    Starting download of 539 bytes downloading of 539 bytes finished
    Set matter cd cert successfully!
    Starting download of 32 bytes downloading of 32 bytes finished
    Set matter dac private key successfully!

For the test, you can find the test attestation binary in: _meta-matter/tools/test_attestation_

The Trusty OS which contained the Trusted Application(TA) for i.MX Matter are maintained by NXP and open source. Following below instruction you will be able to fetch the Trusty OS codes and build the Trusty OS binary.

    $ repo init -u https://github.com/nxp-imx/imx-manifest.git -b imx_matter_2023_q1 -m imx-trusty-matter-2023-q1.xml
    $ repo sync -c

    #Setup the build environment. This will only configure the current terminal.
    $ source trusty/vendor/google/aosp/scripts/envsetup.sh

    #Build the i.MX8M Mini Trusty OS binary:
    $ ./trusty/vendor/google/aosp/scripts/build.py imx8mm
    #And the target binary will be put on: build-root/build-imx8mm/lk.bin

    #When boot to the i.MX8M Mini Linux shell, first time need to trigger the secure storage service.
    $ systemctl enable storageproxyd
    $ systemctl start storageproxyd

# FAQ

Q1 : Why do the npm EAI_AGAIN error occur in the bitbake process and how to solve it?

     | npm ERR! code EAI_AGAIN
     | npm ERR! errno EAI_AGAIN
     | npm ERR! request to https://registry.npmjs.org/angular failed, reason:reason: getaddrinfo EAI_AGAIN registry.cnpmjs.org registry.cnpmjs.org:80

A : This npm EAI_AGAIN error occurs when otbr is compiled more than once. The otbr intermediate compilation files should be cleared before executing the bitbake command again.

     $ bitbake -c cleanall otbr
     $ bitbake imx-image-multimedia

Q2 : why can't "zstd -d imx-image-multimedia-imx8mmevk.wic.zst" be executed in the floder ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/ ?

A : Because imx-image-multimedia-imx8mmevk.wic.zst is a link file, you can zstd the linked file or cp imx-image-multimedia-imx8mmevk.wic.zst to another floder then zstd.

    $ ls -al
    imx-image-multimedia-imx8mmevk.wic.zst -> imx-image-multimedia-imx8mmevk-20220721181418.rootfs.wic.zst

Q3 : How to solve the issue bellow?

    FAILED: src/web/web-service/frontend/CMakeFiles/otbr-web-frontend /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/build/otbr/src/web/web-service/frontend/CMakeFiles/otbr-web-frontend
    cd /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/build/otbr/src/web/web-service/frontend && cp /home/ssd-3/matter/yocto/bld-xwayland/ot-br-posix/src/web/web-service/frontend/package.json . && npm install
    ERROR: npm is known not to run on Node.js v10.19.0
    You'll need to upgrade to a newer Node.js version in order to use this
    version of npm. You can find the latest version at https://nodejs.org/

A : Update the node to latest version.

    $ curl  https://raw.githubusercontent.com/creationix/nvm/master/install.sh | bash
    $ source ~/.profiles       or     source ~/.bashrc        #You can choose one of them as prompted by the previous command
    $ nvm install 18
    $ node --version  # make sure that it was successfully installed.

Q4 : What if the Yocto SDK Python3 is exported into the shell environment and makes the Matter bootstrap/active process fail?

A : open a new shell, then remove the Yocto SDK envrinment and resource the apps build enviroment.

    $ cd ${MY_Matter_Apps}
    $ rm -rf .environment
    $ source scripts/activate.sh

