# Introduction
This repo contains the i.MX MPU project Matter related Yocto recipes. Below is a list of modules that will be built with the meta-matter repo integrated.
 - Matter (CHIP) : https://github.com/project-chip/connectedhomeip
 - OpenThread Daemon: https://github.com/openthread/openthread
 - OpenThread Border Router: https://github.com/openthread/ot-br-posix

All the software components revision are based on [project Matter TE9](https://github.com/project-chip/connectedhomeip/commits/TE9).

The Following Matter related binaries will be installed into the Yocto image root filesystem with this recipe repo:
 - chip-lighting-app: Matter lighting app demo
 - chip-all-clusters-app: Matter all-clusters demo
 - thermostat-app: Matter thermostat demo
 - chip-tool: Matter Controller tools
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

    $ wget https://raw.githubusercontent.com/project-chip/connectedhomeip/master/scripts/constraints.txt
    $ pip3 install -r constraints.txt
    $ pip3 install build mypy==0.910 types-setuptools pylint==2.9.3 
    $ pip install dbus-python

Then, Yocto build environment must be setup.

The Yocto source code is maintained with a repo manifest, the tool repo is used to download the source code.
This document is tested with the i.MX Yocto 5.15.32_2.0.0 release. The hardware tested in this documentation is the i.MX 8M Mini EVK and i.MX6ULL EVK.
Run the commands below to download this release:

    $ mkdir ~/bin
    $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
    $ chmod a+x ~/bin/repo
    $ export PATH=${PATH}:~/bin
    
    $ mkdir ${MY_YOCTO} # this directory will be the top directory of the Yocto source code
    $ cd ${MY_YOCTO}
    $ repo init -u https://source.codeaurora.org/external/imx/imx-manifest -b imx-linux-kirkstone -m imx-5.15.32-2.0.0.xml
    $ repo sync
Then integrate the meta-matter recipe into the Yocto code base

    $ cd ${MY_YOCTO}/sources/
    $ git clone https://github.com/NXPmicro/meta-matter.git

More information about the downloaded Yocto release can be found in the corresponding i.MX Yocto Project User’s Guide which can be found at [NXP official website](http://www.nxp.com/imxlinux).

Make sure your default Python of the Linux host is Python2.

    $ python --version
      Python 2.7.18

Change the current directory to the top directory of the Yocto source code and execute the command below.

    #For i.MX8M Mini EVK:
    $ MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-iot-setup.sh bld-xwayland-imx8mm
    #For i.MX6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-matter/tools/imx-iot-setup.sh bld-xwayland-imx6ull

The system will create a directory bld-xwayland-imx8mm/ for i.MX8M Mini EVK or bld-xwayland-imx6ull/ for i.MX6ULL EVK and enter this directory automatically, execute the command below under these directory to generate the Yocto images.

    $ bitbake imx-image-multimedia

After execution of previous commands, the Yocto images will be generated under ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/imx-image-multimedia-imx8mmevk.wic.bz2 for i.MX8M Mini EVK.

And ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.bz2 for i.MX6ULL EVK. 

The bz2 images are symbolic link files, so you should copy it to another fold ${MY_images} before unzip it.
    
    #For i.MX8M Mini EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/imx-image-multimedia-imx8mmevk.wic.bz2 ${MY_images}
    #For i.MX6ULL EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.bz2 ${MY_images}

The bzip2 command should be used to unzip this file then the dd command should be used to program the output file to a microSD card by running the commands below. Then a microSD card can be used to boot the image of an i.MX 8M Mini EVK or i.MX6ULL EVK.

___Be cautious when executing the dd command below, make sure the of represents the microSD card device!, /dev/sdc in the command below represents a microSD card connected to the host machine with a USB adapter, however the output device name may vary. Use the command "ls /dev/sd*" to verify the name of the SD card device.___

    $ cd ${MY_images}
    #For i.MX8M Mini EVK:
    $ bzip2 -d imx-image-multimedia-imx8mmevk.wic.bz2
    $ sudo dd if=imx-image-multimedia-imx8mmevk.wic of=/dev/sdc bs=4M conv=fsync
    #For i.MX6ULL EVK:
    $ bzip2 -d imx-image-multimedia-imx6ullevk.wic.bz2
    $ sudo dd if=imx-image-multimedia-imx6ullevk.wic of=/dev/sdc bs=4M conv=fsync

# How to build OpenThread Border Router with Yocto SDK
There are 3 module for OpenThread Border Router (OTBR): otbr-agent, ot-ctl and otbr-web. The otbr-web need liboost static and jsoncpp modules which are not included into default built Yocto images.

To build these binaries, the Yocto SDK with meta-matter generated must be used. This SDK can be generated by below command:

    #For i.MX8M Mini EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx8mm
    #For i.MX6ULL EVK:
    $ cd ${MY_YOCTO}/bld-xwayland-imx6ull

    $bitbake imx-image-multimedia -c populate_sdk

Install the NXP Yocto SDK and set toolchain environment variables.
Run the SDK installation script with root permission.

    #For i.MX8M Mini EVK:
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-armv8a-imx8mmevk-toolchain-5.15-kirkstone.sh
    #For i.MX6ULL EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-multimedia-cortexa7t2hf-neon-imx6ullevk-toolchain-5.15-kirkstone.sh

The default target directory for SDK will be prompted during SDK installation, as shown below，enter a new target directory while the path \${/opt/fsl-imx-xwayland/} is required .
___If you need build SDK both for i.MX8M Mini EVK and i.MX6ULL EVK on a host machine, it is necessary to distinguish the paths. As a reference, you can use /opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm for i.MX8M Mini EVK SDK and /opt/fsl-imx-xwayland/5.15-kirkstone-imx6ull for i.MX6ULL EVK.___

    NXP i.MX Release Distro SDK installer version 5.15-kirkstone
    ============================================================
    Enter target directory for SDK (default: /opt/fsl-imx-xwayland/5.15-kirkstone):

After the Yocto SDK is installed on the host machine, an environment setup script is also generated, and there are prompt lines telling the user to source the script each time when using the SDK in a new shell, for example:

    #For i.MX8M Mini EVK
    $ . /opt/fsl-imx-xwayland/5.15-kirkstone-imx8mm/environment-setup-armv8a-poky-linux
    $For i.MX6ULL EVK
    $ . /opt/fsl-imx-wayland/5.15-kirkstone-imx6ull/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi

After the SDK package installed in the build machine, import the Yocto build environment using the command:

    #For i.MX8M Mini EVK
    $source ${iMX8MM_SDK_INSTALLED_PATH}/environment-setup-armv8a-poky-linux
    #For i.MX6ULL EVK
    $source ${iMX6ULL_SDK_INSTALLED_PATH}/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi

Fetch latest otbr source codes and execute the build:

    $ mkdir ${MY_OTBR}  # this directory will be the top directory of the OTBR source code
    $ cd ${MY_OTBR} 
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git checkout -t origin/main
    $ git submodule update --init
    #For i.MX8M Mini EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DOTBR_BACKBONE_ROUTER=ON \
     -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON \
     -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.2 -DOTBR_INFRA_IF_NAME=mlan0 \
     -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake
    #For i.MX6ULL EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=ON -DOTBR_BACKBONE_ROUTER=ON \
     -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DBUILD_TESTING=OFF -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON \
     -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.2 -DOTBR_INFRA_IF_NAME=mlan0 \
     -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/arm.cmake

The otbr-agent is built in \${MY_OTBR}/build/otbr/src/agent/otbr-agent. 

The otbr-web is built in \${MY_OTBR}/build/otbr/src/web/otbr-web.

The ot-ctl is built in \${MY_OTBR}/build/otbr/third_party/openthread/repo/src/posix/ot-ctl.

Please copy them into Yocto's /usr/sbin/.

# How to setup OpenThread Border Router environment within the Yocto

After the OTBR boot, the __i.MX8M Mini EVK__ or __i.MX6ULL EVK__ must connect the OTBR to the target Wi-Fi AP network.

    $modprobe moal mod_para=nxp/wifi_mod_para.conf
    $wpa_passphrase ${SSID} ${PASSWORD} > imxrouter.conf
    $wpa_supplicant -d -B -i mlan0 -c ./imxrouter.conf
    $udhcpc -i mlan0
    $echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
    $echo 1 > /proc/sys/net/ipv4/ip_forward
    $echo 2 > /proc/sys/net/ipv6/conf/all/accept_ra

Plugin the Thread module into the USB OTG port of __i.MX8M Mini EVK__ or __i.MX6ULL EVK__. A USB device should be visible as _/dev/ttyUSB_ or _/dev/ttyACM_.
Once the USB device is detected, start te OTBR related services.

When using the RCP module, programmed with OpenThread Spinel firmware image, execute the following commands:

    $otbr-agent -I wpan0 -B mlan0 spinel+hdlc+uart:///dev/ttyACM0 &
    $iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $otbr-web &

# How to build Matter application

The Matter application has be installed into the Yocto image defaultly, but the software are based on project Matter TE9. If you want use the latest SVE versions, you shoule build it separately. Run the commands below to download the Matter application code And switch to SVE branch:

    $ mkdir ${MY_Matter_Apps}     # this is top level directory of this project
    $ cd ${MY_Matter_Apps}  
    $ git clone https://github.com/project-chip/connectedhomeip.git
    $ cd connectedhomeip
    $ git checkout -b sve origin/sve
    $ git submodule update --init

 ___Make sure the shell isn't in Yocto SDK environment___. Then, export a shell environment variable named IMX_SDK_ROOT to specify the path of the SDK.

    #For i.MX8M Mini EVK  #/opt/fsl-imx-xwayland/5.15-hardknott-imx8mm is ${IMX8MM_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-hardknott-imx8mm     
    #For i.MX6ULL EVK     #/opt/fsl-imx-xwayland/5.15-hardknott-imx6ull is ${IMX6ULL_SDK_INSTALLED_PATH}
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.15-hardknott-imx6ull

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

The apps are built in the subdirectories under out/, the subdirectory name is the same as the argument specified after the option --target when build the examples. For example, the imx-all-clusters-app executable files can found in \${MY_Matter_Apps}/connectedhomeip/out/imx-all-clusters-app/.

___Make sure the subdirectories isn't exist before build the same name app.___
If an app needs to be built both for iMX8M Mini EVK and iMX6ULL EVK, you can use the option --out-prefix to specify a subdirectory, for example:  

    $./scripts/build/build_examples.py  --target imx-chip-tool --out-prefix ./out/imx8mm build

After executing the above command, the chip-tool executable files will be found in ${MY_Matter_Apps}/out/imx8mm/imx-chip-tool/.

A Matter official document about how to use chip-tool as Matter controller can be found in [here](https://github.com/project-chip/connectedhomeip/blob/TE8/rc3/examples/chip-tool/README.md).


# FAQ

Q1 : How to solve the following error occur while install python dependency packages?
     
     ERROR: launchpadlib 1.10.13 requires testresources, which is not installed.ERROR: pip-tools 6.1.0 has requirement pip>=20.3, but you'll have pip 20.0.2 which is incompatible.

A : Run the commands below to install python3-testresources. It has no effect for now if the python3-testresources is not installed.
     
     $ sudo apt install python3-testresources

Q2 : Why do the npm EAI_AGAIN error occur in the bitbake process and how to solve it?
     
     | npm ERR! code EAI_AGAIN
     | npm ERR! errno EAI_AGAIN
     | npm ERR! request to https://registry.npmjs.org/angular failed, reason:reason: getaddrinfo EAI_AGAIN registry.cnpmjs.org registry.cnpmjs.org:80 

A : This npm EAI_AGAIN error occurs when otbr is compiled more than once. The otbr intermediate compilation files should be cleared before executing the bitbake command again.
    
     $ bitbake -c cleanall otbr
     $ bitbake imx-image-multimedia

Q3 : why can't "bzip2 -d imx-image-multimedia-imx8mmevk.wic.bz2" be executed in the floder ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/ ?

A : Because imx-image-multimedia-imx8mmevk.wic.bz2 is a link file, you can bzip2 the linked file or cp imx-image-multimedia-imx8mmevk.wic.bz2 to another floder then bizp2.

    $ ls -al
    imx-image-multimedia-imx8mmevk.wic.bz2 -> imx-image-multimedia-imx8mmevk-20220721181418.rootfs.wic.bz2

Q4 : How to solve the issue bellow?

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

Q5 : What if the Yocto SDK Python3 is exported into the shell environment and makes the Matter bootstrap/active process fail?

A : open a new shell, then remove the Yocto SDK envrinment and resource the apps build enviroment. 
   
    $ cd ${MY_Matter_Apps}
    $ rm -rf .environment
    $ source scripts/activate.sh
