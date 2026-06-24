# Contents

[**Introduction**](#introduction)

[**i.MX MPU Matter platform**](#imx-mpu-platform)

[**What's new**](#new-feature)

[**How to build the Yocto image with an integrated OpenThread Border Router**](#how-to-build-the-yocto-image-with-an-integrated-openthread-border-router)

[**How to build OpenThread Border Router and OpenThread Daemon with Yocto SDK**](#how-to-build-openthread)

[**How to setup OpenThread Border Router and OpenThread Daemon environment within the Yocto**](#how-to-set-up-openthread)

[**How to build Matter application**](#how-to-build-matter-application)

[**Security configuration for Matter**](#security-configuration-for-matter)

[**Known issue**](#known-issue)

[**FAQ**](#faq)

<a name="introduction"></a>

# Introduction
This repository contains the i.MX MPU project Matter related Yocto recipes. The following modules are built with this meta-nxp-connectivity layer.
 - **Matter** (CHIP) : https://github.com/nxp/matter.git
 - **OpenThread** Daemon: https://github.com/openthread/openthread
 - **OpenThread Border Router**: https://github.com/openthread/ot-br-posix
 - **Zigbee** Stack and application examples: [meta-nxp-zigbee-rcp](./docs/guides/meta-nxp-connectivity-iwxxx-dualpan.md) and [zbcoord](./meta-nxp-connectivity-examples/recipes-zbcoord/zbcoord)
 - **Matter-NCP** [nxp-matter-ncp](./docs/guides/nxp_matter_ncp_guide.md)

All software component revisions are based on [Matter v1.6-branch 2026](https://github.com/project-chip/connectedhomeip/tree/v1.6-branch).

The following Matter-related binaries are installed into the Yocto image root file system by this Yocto layer recipes:
 - chip-lighting-app: Matter lighting app demo
 - chip-lighting-app-trusty: Matter lighting app with enhanced security on i.MX 8M Mini
 - chip-all-clusters-app: Matter all-clusters demo
 - thermostat-app: Matter thermostat demo
 - nxp-thermostat-app: NXP customized thermostat application, which is used for Matter Certification
 - nxp-thermostat-app-trusty: NXP customized thermostat application with enhanced security on i.MX 8M Mini
 - chip-bridge-app: Matter bridge demo
 - imx-chip-bridge-app: NXP customized Zigbee bridge application
 - nxp-media-app: NXP customized media application
 - nxp-media-app-trusty: NXP customized media application with enhanced security on i.MX 8M Mini
 - imx-thread-br-app: NXP customized Thread Border Router Management application
 - chip-evse-app: Matter energy management app demo
 - chip-tool: Matter Controller tool
 - chip-tool-trusty: Matter Controller tool with enhanced security for i.MX 8M Mini
 - chip-tool-web2: Matter Web Controller tool (version 2) with Angular Material UI
 - chip-ota-provider-app: Matter ota provider app demo
 - chip-ota-requestor-app: Matter ota requestor app demo
 - chip-rvc-app: Matter Robotic Vacuum Cleaner demo
 - ot-daemon: OpenThread Daemon for OpenThread client
 - ot-client-ctl: OpenThread Control tool for OpenThread client
 - otbr-agent: OpenThread Border Router agent
 - ot-ctl: OpenThread Border Router Control tool
 - ot-daemon-iwxxx: OpenThread Daemon for OpenThread client of the IW612 chipset and IW610 chipset
 - ot-client-iwxxx: OpenThread Control tool for OpenThread client of the IW612 chipset and IW610 chipset
 - otbr-agent-iwxxx: OpenThread Border Router agent of the IW612 chipset and IW610 chipset
 - ot-ctl-iwxxx: OpenThread Border Router Control tool of the IW612 chipset and IW610 chipset
 - otbr-web: OpenThread Border Router web management daemon

<a name="imx-mpu-platform"></a>

# i.MX MPU Matter platform

Support is provided for 11 i.MX MPU platforms, including i.MX 93 FRDM and EVK, i.MX 8M Mini EVK, i.MX 6ULL EVK, i.MX 8ULP EVK, i.MX 91 EVK, QSB, and FRDM, i.MX 8M Plus FRDM, as well as the i.MX 95 15×15 EVK, i.MX 95 FRDM, and i.MX 95 FRDM PRO. For more details, refer to the [NXP i.MX MPU Matter Platform](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/mpu-linux-hosted-matter-development-platform:MPU-LINUX-MATTER-DEV-PLATFORM).

<a name="new-feature"></a>

# What's new in i.MX Matter 2026 Q2 release

- Integrated with Linux L6.18.20 and Yocto Wrynose.
- Upgraded Matter application source code to v1.6-branch 2026 version.
- Support Matter controller application and Matter demo applications on i.MX 95 FRDM Platform, support OpenThread Border Router and OpenThread on i.MX 95 FRDM Platform.
- Support Matter controller application and Matter demo applications on i.MX 95 FRDM PRO Platform, support OpenThread Border Router and OpenThread on i.MX 95 FRDM PRO Platform.
- Support Matter controller application and Matter demo applications on i.MX 8M Plus FRDM Platform, support OpenThread Border Router and OpenThread on i.MX 8M Plus FRDM Platform.
- Support MSR (mobile service robot) control on chip-tool-web2

<a name="how-to-build-the-yocto-image-with-an-integrated-openthread-border-router"></a>

# How to build the Yocto image with an integrated OpenThread Border Router

The following packages are required to build the Yocto Project:

    $ sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
    build-essential chrpath socat cpio python3 python3-pip python3-pexpect cmake \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libegl1 libsdl1.2-dev \
    pylint xterm npm zstd build-essential libpython3-dev libdbus-1-dev python3.8-venv lz4 \
    git git-lfs gcc g++ pkg-config libssl-dev libglib2.0-dev libavahi-client-dev ninja-build \
    python3-venv python3-dev libgirepository1.0-dev libcairo2-dev libreadline-dev default-jre

Make sure that your default Python3 version is 3.11:

    $ python3 --version
      Python 3.11.x

Then, Yocto build environment must be set up.

The Yocto source code and meta-nxp-connectivity recipes are maintained with a manifest file, used by the repo tool to download the corresponding source code.
This document is tested with the i.MX Yocto 6.18.20_2.0.0 release. The platforms tested are: i.MX 93 FRDM, i.MX 93 EVK, i.MX 8M Mini EVK, i.MX 6ULL EVK, i.MX 8ULP EVK. i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM, i.MX 95 15×15 EVK.
Run the commands below to download this release:

    $ mkdir ~/bin
    $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
    $ chmod a+x ~/bin/repo
    $ export PATH=${PATH}:~/bin

    $ mkdir ${MY_YOCTO} # this directory is the top directory of the Yocto source code
    $ cd ${MY_YOCTO}
    $ repo init -u https://github.com/nxp-imx/imx-manifest -b imx-linux-wrynose -m imx-6.18.20-2.0.0.xml
    $ repo sync
    $ cd ${MY_YOCTO}/sources/meta-nxp-connectivity
    $ git remote update
    $ git checkout imx_matter_2026_q2

More information about the downloaded Yocto release can be found in the corresponding i.MX Yocto Project User’s Guide, which can be found at [NXP official website](http://www.nxp.com/imxlinux).

Change the current directory to the top directory of the Yocto source code and execute the command below:

    # For i.MX 93 FRDM and i.MX 93 EVK:
    $ MACHINE=imx93evk-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx93

    # For i.MX 8M Mini EVK
    $ MACHINE=imx8mmevk-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8mm

    # For i.MX 6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx6ull

    # For i.MX 8ULP EVK:
    $ MACHINE=imx8ulpevk-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8ulp

    # For i.MX 91 EVK:
    $ MACHINE=imx91evk-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx91

    # For i.MX 91 QSB:
    $ MACHINE=imx91qsb-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx91qsb

    # For i.MX 91 FRDM:
    $ MACHINE=imx91frdm-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx91frdm

    # For i.MX 95 15×15 EVK:
    $ MACHINE=imx95-15x15-evk-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx95

    # For i.MX 95 FRDM:
    $ MACHINE=imx95-frdm-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx95frdm

    # For i.MX 95 FRDM PRO:
    $ MACHINE=imx95-frdm-pro-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx95frdm-pro
    
    # For i.MX 8M Plus FRDM:
    $ MACHINE=imx8mp-frdm-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8mpfrdm

This creates a Python virtual environment for the Matter build. To exit the Python virtual environment, run "$ deactivate". You can also run "$ source matter_venv/bin/activate" at the top directory of the Yocto source code to reenter the Python virtual environment for the Matter build.

This will also create a build directory (namely bld-xwayland-imx93/ for i.MX 93 FRDM and i.MX 93 EVK, bld-xwayland-imx8mm/ for i.MX 8M Mini EVK, bld-xwayland-imx6ull/ for i.MX 6ULL EVK, bld-xwayland-imx8ulp/ for i.MX 8ULP EVK, bld-xwayland-imx91/ for i.MX 91 EVK, bld-xwayland-imx91qsb/ for i.MX 91 QSB, bld-xwayland-imx91frdm/ for i.MX 91 FRDM, bld-xwayland-imx95 for i.MX 95 15×15 EVK, bld-xwayland-imx95-frdm for i.MX 95 FRDM, bld-xwayland-imx95-frdm-pro for i.MX 95 FRDM PRO or bld-xwayland-imx8mp-frdm for i.MX 8M Plus FRDM), and enter this directory automatically. Execute the command below to generate the Yocto images:

    $ bitbake imx-image-multimedia

After execution of the previous commands, the Yocto images will be generated:
- ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk-iwxxx-matter/imx-image-multimedia-imx93evk-iwxxx-matter.rootfs.wic.zst for i.MX 93 FRDM and i.MX 93 EVK.
- ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk-matter.wic.zst for i.MX 8M Mini EVK.
- ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst for i.MX 6ULL EVK.
- ${MY_YOCTO}/bld-xwayland-imx8ulp/tmp/deploy/images/imx8ulpevk/imx-image-multimedia-imx8ulpevk-matter.wic.zst for i.MX 8ULP EVK.
- ${MY_YOCTO}/bld-xwayland-imx91/tmp/deploy/images/imx91evk-iwxxx-matter/imx-image-multimedia-imx91evk-iwxxx-matter.wic.zst for i.MX 91 EVK.
- ${MY_YOCTO}/bld-xwayland-imx91qsb/tmp/deploy/images/imx91qsb-iwxxx-matter/imx-image-multimedia-imx91qsb-iwxxx-matter.wic.zst for i.MX 91 QSB.
- ${MY_YOCTO}/bld-xwayland-imx91frdm/tmp/deploy/images/imx91frdm-iwxxx-matter/imx-image-multimedia-imx91frdm-iwxxx-matter.wic.zst for i.MX 91 FRDM.
- ${MY_YOCTO}/bld-xwayland-imx95/tmp/deploy/images/imx95-iwxxx-matter/imx-image-multimedia-imx95-15x15-evk-iwxxx-matter.wic.zst for i.MX 95 15×15 EVK.
- ${MY_YOCTO}/bld-xwayland-imx95-frdm/tmp/deploy/images/imx95-frdm-iwxxx-matter/imx-image-multimedia-imx95-frdm-iwxxx-matter.rootfs.wic.zst for i.MX 95 FRDM.
- ${MY_YOCTO}/bld-xwayland-imx95-frdm-pro/tmp/deploy/images/imx95-frdm-pro-iwxxx-matter/imx-image-multimedia-imx95-frdm-pro-iwxxx-matter.rootfs.wic.zst for i.MX 95 FRDM PRO.
- ${MY_YOCTO}/bld-xwayland-imx8mp-frdm/tmp/deploy/images/imx8mp-frdm-iwxxx-matter/imx-image-multimedia-imx8mp-frdm-iwxxx-matter.rootfs.wic.zst for i.MX 8M Plus FRDM.

The zst images are symbolic link files, so you should copy them to a dedicated folder ${MY_images} before unzipping them.

    # For i.MX 93 FRDM and i.MX 93 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx93/tmp/deploy/images/imx93evk-iwxxx-matter/imx-image-multimedia-imx93evk-iwxxx-matter.rootfs.wic.zst ${MY_images}

    # For i.MX 8M Mini EVK
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk-matter/imx-image-multimedia-imx8mmevk-matter.wic.zst ${MY_images}

    # For i.MX 6ULL EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx6ull/tmp/deploy/images/imx6ullevk/imx-image-multimedia-imx6ullevk.wic.zst ${MY_images}

    # For i.MX 8ULP EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8ulp/tmp/deploy/images/imx8ulpevk/imx-image-multimedia-imx8ulpevk-matter.wic.zst ${MY_images}

    # For i.MX 91 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx91/tmp/deploy/images/imx91evk-iwxxx-matter/imx-image-multimedia-imx91evk-iwxxx-matter.wic.zst ${MY_images}

    # For i.MX 91 QSB:
    $ cp ${MY_YOCTO}/bld-xwayland-imx91qsb/tmp/deploy/images/imx91qsb-iwxxx-matter/imx-image-multimedia-imx91qsb-iwxxx-matter.wic.zst ${MY_images}

    # For i.MX 91 FRDM:
    $ cp ${MY_YOCTO}/bld-xwayland-imx91frdm/tmp/deploy/images/imx91frdm-iwxxx-matter/imx-image-multimedia-imx91frdm-iwxxx-matter.wic.zst ${MY_images}

    # For i.MX 95 15×15 EVK:
    $ cp ${MY_YOCTO}/bld-xwayland-imx95/tmp/deploy/images/imx95-iwxxx-matter/imx-image-multimedia-imx95-15x15-evk-iwxxx-matter.wic.zst ${MY_images}

    # For i.MX 95 FRDM:
    $ cp ${MY_YOCTO}/bld-xwayland-imx95-frdm/tmp/deploy/images/imx95-frdm-iwxxx-matter/imx-image-multimedia-imx95-frdm-iwxxx-matter.rootfs.wic.zst ${MY_images}
    
    # For i.MX 95 FRDM PRO:
    $ cp ${MY_YOCTO}/bld-xwayland-imx95-frdm-pro/tmp/deploy/images/imx95-frdm-pro-iwxxx-matter/imx-image-multimedia-imx95-frdm-pro-iwxxx-matter.rootfs.wic.zst ${MY_images}

    # For i.MX 8M Plus FRDM:
    $ cp ${MY_YOCTO}/bld-xwayland-imx8mp-frdm/tmp/deploy/images/imx8mp-frdm-iwxxx-matter/imx-image-multimedia-imx8mp-frdm-iwxxx-matter.rootfs.wic.zst ${MY_images}

You can use the zstd and dd commands to flash the images to a microSD card for i.MX 93 FRDM, i.MX 93 EVK, i.MX 8M Mini EVK, i.MX 6ULL EVK, i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM, i.MX 95 15×15 EVK, i.MX 95 FRDM, i.MX 95 FRDM PRO and i.MX 8M Plus FRDM. You can also use the [Universal Update Utility](https://github.com/nxp-imx/mfgtools) to flash the images for all 8 boards. The i.MX 8ULP EVK supports only booting from EMMC, not from microSD. Other platforms support both booting from EMMC and microSD images.

For use with the zstd and dd command method, use the zstd command to unzip the .zst archive, and then use the dd command to program the output file to a microSD card.

___Be cautious when executing the dd command below, making sure that the output ("of" parameter) represents the microSD card device! /dev/sdc in the below command represents a microSD card connected to the host machine with a USB adapter; however the output device name may vary. Use the "ls /dev/sd*" command to verify the name of the SD card device.___

    $ cd ${MY_images}

    # For i.MX 93 FRDM and i.MX 93 EVK:
    $ zstd -d imx-image-multimedia-imx93evk-iwxxx-matter.rootfs.wic.zst
    $ sudo dd if=imx-image-multimedia-imx93evk-iwxxx-matter.rootfs.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 8M Mini EVK
    $ zstd -d imx-image-multimedia-imx8mmevk-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx8mmevk-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 6ULL EVK:
    $ zstd -d imx-image-multimedia-imx6ullevk.wic.zst
    $ sudo dd if=imx-image-multimedia-imx6ullevk.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 91 EVK:
    $ zstd -d imx-image-multimedia-imx91evk-iwxxx-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx91evk-iwxxx-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 91 QSB:
    $ zstd -d imx-image-multimedia-imx91qsb-iwxxx-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx91qsb-iwxxx-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 91 FRDM:
    $ zstd -d imx-image-multimedia-imx91frdm-iwxxx-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx91frdm-iwxxx-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 95 15×15 EVK:
    $ zstd -d imx-image-multimedia-imx95-15x15-evk-iwxxx-matter.wic.zst
    $ sudo dd if=imx-image-multimedia-imx95-15x15-evk-iwxxx-matter.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 95 FRDM:
    $ zstd -d imx-image-multimedia-imx95-frdm-iwxxx-matter.rootfs.wic.zst
    $ sudo dd if=imx-image-multimedia-imx95-frdm-iwxxx-matter.rootfs.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 95 FRDM PRO:
    $ zstd -d imx-image-multimedia-imx95-frdm-pro-iwxxx-matter.rootfs.wic.zst
    $ sudo dd if=imx-image-multimedia-imx95-frdm-pro-iwxxx-matter.rootfs.wic of=/dev/sdc bs=4M conv=fsync

    # For i.MX 8M Plus FRDM:
    $ zstd -d imx-image-multimedia-imx8mp-frdm-iwxxx-matter.rootfs.wic.zst
    $ sudo dd if=imx-image-multimedia-imx8mp-frdm-iwxxx-matter.rootfs.wic of=/dev/sdc bs=4M conv=fsync

For use with the uuu method, install [uuu](https://github.com/nxp-imx/mfgtools/releases/tag/uuu_1.5.201) on your host and make sure it is at least version 1.5.201.

    $ uuu -version
    uuu (Universal Update Utility) for nxp imx chips -- libuuu_1.5.201-0-g727fc2b

___Before flashing the image, follow the prompts on the board to put the board into serial download mode. After flashing the image, keep i.MX 93 FRDM, i.MX 93 EVK, i.MX 8M Mini EVK, i.MX 6ULL EVK, i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM, i.MX 95 15×15 EVK, i.MX 95 FRDM, i.MX 95 FRDM PRO and i.MX 8M Plus FRDM into MicroSD boot mode to boot the image from the MicroSD card. Place i.MX 8ULP EVK into EMMC boot mode to boot the image from the EMMC.___

    $ cd ${MY_images}

    # For i.MX 93 FRDM and i.MX 93 EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx93evk-iwxxx-matter.rootfs.wic.zst

    # For i.MX 8M Mini EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx8mmevk-matter.wic.zst

    # For i.MX 6ULL EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx6ullevk.wic.zst

    # For i.MX 8ULP EVK:
    $ sudo uuu -b emmc_all imx-image-multimedia-imx8ulpevk-matter.wic.zst

    # For i.MX 91 EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx91evk-iwxxx-matter.wic.zst

    # For i.MX 91 QSB:
    $ sudo uuu -b sd_all imx-image-multimedia-imx91qsb-iwxxx-matter.wic.zst

    # For i.MX 91 FRDM:
    $ sudo uuu -b sd_all imx-image-multimedia-imx91frdm-iwxxx-matter.wic.zst

    # For i.MX 95 15×15 EVK:
    $ sudo uuu -b sd_all imx-image-multimedia-imx95-15x15-evk-iwxxx-matter.wic.zst

    # For i.MX 95 FRDM:
    $ sudo uuu -b sd_all imx-image-multimedia-imx95-frdm-iwxxx-matter.rootfs.wic.zst

    # For i.MX 95 FRDM PRO:
    $ sudo uuu -b sd_all imx-image-multimedia-imx95-frdm-pro-iwxxx-matter.rootfs.wic.zst

    # For i.MX 8M Plus FRDM:
    $ sudo uuu -b sd_all imx-image-multimedia-imx8mp-frdm-iwxxx-matter.rootfs.wic.zst

The prebuilt images for i.MX 93 FRDM, i.MX 93 EVK, i.MX 8M Mini EVK, i.MX 6ULL EVK, i.MX 8ULP EVK, i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM and i.MX 95 15×15 EVK can be downloaded from [NXP i.MX MPU Matter Platform](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/mpu-linux-hosted-matter-development-platform:MPU-LINUX-MATTER-DEV-PLATFORM).

__Note: For i.MX 93 FRDM, it is essential to modify the fdtfile for it to work properly.__
Enter the U-Boot mode and run the following commands to set the fdtfile, save the fdtfile setting, and boot the board.

    u-boot=> print fdtfile
    fdtfile=imx93-11x11-evk-ffu_gpio_irq.dtb
    u-boot=> fatls mmc 1
    u-boot=> fatls mmc 1
    35183104   Image
        64421   imx93-11x11-evk.dtb
        50672   imx93-11x11-evk-ffu_gpio_irq.dtb
        45915   imx93-11x11-frdm.dtb
        ......

    u-boot=> setenv fdtfile imx93-11x11-frdm.dtb
    u-boot=> saveenv
    Saving Environment to MMC... Writing to MMC(1)... OK
    u-boot=> print fdtfile
    fdtfile=imx93-11x11-frdm.dtb
    u-boot=> boot

<a name="how-to-build-openthread"></a>

# How to build OpenThread Border Router and OpenThread Daemon with Yocto SDK

There are 2 modules for OpenThread Border Router (OTBR): otbr-agent, and ot-ctl.
There are 2 modules for OpenThread: ot-daemon, ot-client-ctl.

To build these binaries, you must use the Yocto SDK toolchain with meta-nxp-connectivity included.
This SDK can be generated with the following commands:

    # For i.MX 9 series platforms and i.MX 8 series platforms:
    $ MACHINE=imx93evk-iwxxx-matter DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx8n9sdk
    $ cd ${MY_YOCTO}/bld-xwayland-imx8n9sdk

    # For i.MX 6ULL EVK:
    $ MACHINE=imx6ullevk DISTRO=fsl-imx-xwayland source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh bld-xwayland-imx6ull
    $ cd ${MY_YOCTO}/bld-xwayland-imx6ull

    $ bitbake imx-image-sdk -c populate_sdk
    # In this step, you may need install one more time dependency by running:
    $ sudo apt-get install git-lfs

Then, install the Yocto SDK, by running the SDK installation script with root permission:

    # For i.MX 9 series platforms and i.MX 8 series platforms:
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-sdk-armv8a-imx93evk-iwxxx-matter-toolchain-6.18-wrynose.sh

    # For i.MX 6ULL EVK
    $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-sdk-cortexa7t2hf-neon-imx6ullevk-toolchain-6.18-wrynose.sh

The SDK installation directory is prompted during the SDK installation. You can specify the installation directory, or keep the default one: /opt/fsl-imx-xwayland/6.18-wrynose.
___Use board-specific paths if you need to build the SDK for several EVK boards; for example, you can use /opt/fsl-imx-xwayland/6.18-wrynose-imx8n9 for i.MX 9 series platforms and i.MX 8 series platforms, /opt/fsl-imx-xwayland/6.18-wrynose-imx6ull for i.MX 6ULL EVK.___

    NXP i.MX Release Distro SDK installer version 6.18-wrynose
    ============================================================
    Enter target directory for SDK (default: /opt/fsl-imx-xwayland/6.18-wrynose):

Enter the "/opt/fsl-imx-xwayland/6.18-wrynose-imx8n9" or "/opt/fsl-imx-xwayland/6.18-wrynose-imx6ull" when the above prompt displays.

After the Yocto SDK is installed on the host machine, an SDK environment setup script is also generated.
The user must import Yocto build environment, by sourcing this script each time the SDK is used in a new shell; for example:

    # For i.MX 9 series platforms and i.MX 8 series platforms:
    $ . /opt/fsl-imx-xwayland/6.18-wrynose-imx8n9/environment-setup-armv8a-poky-linux

    # For i.MX 6ULL EVK
    $ . /opt/fsl-imx-xwayland/6.18-wrynose-imx6ull/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi

Fetch the latest otbr source code and execute the build for OTBR:

    $ mkdir ${MY_OTBR}  # this directory is the top directory of the OTBR source code
    $ cd ${MY_OTBR}
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git checkout 45c847a6b47cef00c9e3d46786127ef87475437d
    $ git submodule update --init
    $ git cherry-pick ecd9519e469ff4addeb4c5287a6c9445acfba299

    # For i.MX 8M Mini EVK and i.MX 8ULP EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=OFF -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/aarch64.cmake \
      -DPROTOC_DIR=${OECORE_NATIVE_SYSROOT}/usr/ -DCMAKE_CXX_STANDARD=17 -DCMAKE_POLICY_VERSION_MINIMUM=3.5

    # For i.MX 6ULL EVK
    $ ./script/cmake-build -DOTBR_BORDER_ROUTING=ON -DOTBR_REST=ON -DOTBR_WEB=OFF -DBUILD_TESTING=OFF -DOTBR_DBUS=ON \
      -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.3 -DOTBR_INFRA_IF_NAME=mlan0 \
      -DOTBR_BACKBONE_ROUTER=ON -DOT_BACKBONE_ROUTER_MULTICAST_ROUTING=ON -DOTBR_MDNS=mDNSResponder \
      -DCMAKE_TOOLCHAIN_FILE=./examples/platforms/nxp/linux-imx/arm.cmake \
      -DPROTOC_DIR=${OECORE_NATIVE_SYSROOT}/usr/ -DCMAKE_CXX_STANDARD=17 -DCMAKE_POLICY_VERSION_MINIMUM=3.5

The otbr-agent is built in \${MY_OTBR}/build/otbr/src/agent/otbr-agent.

The ot-ctl is built in \${MY_OTBR}/build/otbr/third_party/openthread/repo/src/posix/ot-ctl.

Copy them into the target /usr/sbin/ directory.

__The OTBR does not support incremental compilation. If an error occurs during compilation, or if you need to recompile, delete ${MY_OTBR}/build before recompiling.__

    $ cd ${MY_OTBR}
    $ rm -rf build/

Fetch the latest OpenThread source code and execute the build for OpenThread:

    $ mkdir ${MY_OPENTHREAD}  # this directory is the top directory of the Open Thread source code
    $ cd ${MY_OPENTHREAD}
    $ git clone https://github.com/openthread/openthread
    $ cd openthread
    $ git checkout 9681690fab100590566e4937cbf2d072de031ff3

    # For i.MX 8M Mini EVK, i.MX 8ULP EVK and i.MX 6ULL EVK
    $ cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOT_COMPILE_WARNING_AS_ERROR=OFF -DOT_PLATFORM=posix -DOT_SLAAC=ON \
      -DOT_BORDER_AGENT=ON -DOT_BORDER_ROUTER=ON -DOT_COAP=ON -DOT_COAP_BLOCK=ON -DOT_COAP_OBSERVE=ON -DOT_COAPS=ON -DOT_COMMISSIONER=ON \
      -DOT_CHANNEL_MANAGER=ON -DOT_CHANNEL_MONITOR=ON -DOT_CHILD_SUPERVISION=ON -DOT_DATASET_UPDATER=ON -DOT_DHCP6_CLIENT=ON \
      -DOT_DHCP6_SERVER=ON -DOT_DIAGNOSTIC=ON -DOT_DNS_CLIENT=ON -DOT_ECDSA=ON -DOT_IP6_FRAGM=ON -DOT_JAM_DETECTION=ON -DOT_JOINER=ON \
      -DOT_LEGACY=ON -DOT_MAC_FILTER=ON -DOT_NETDIAG_CLIENT=ON -DOT_NEIGHBOR_DISCOVERY_AGENT=ON -DOT_PING_SENDER=ON \
      -DOT_REFERENCE_DEVICE=ON -DOT_SERVICE=ON -DOT_SNTP_CLIENT=ON -DOT_SRP_CLIENT=ON -DOT_COVERAGE=OFF -DOT_LOG_LEVEL_DYNAMIC=ON \
      -DOT_RCP_RESTORATION_MAX_COUNT=2 -DOT_LOG_OUTPUT=PLATFORM_DEFINED -DOT_POSIX_MAX_POWER_TABLE=ON -DOT_DAEMON=ON \
      -DOT_THREAD_VERSION=1.3 -DCMAKE_BUILD_TYPE=Release -DOT_RCP_RESTORATION_MAX_COUNT=10 -DOT_POSIX_RCP_HDLC_BUS=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    $ ninja

The ot-daemon is built in \${MY_OPENTHREAD}/src/posix/ot-daemon.

The ot-ctl for ot-daemon is built in \${MY_OPENTHREAD}/src/posix/ot-ctl.

Rename the ot-ctl to ot-client-ctl and then copy ot-daemon and ot-client-ctl into the target /usr/sbin/ directory.

<a name="how-to-set-up-openthread"></a>

# How to set up OpenThread Border Router and OpenThread Daemon on the target

Use the commands below to connect the OTBR to the Wi-Fi access point:

    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ wpa_passphrase ${SSID} ${PASSWORD} > wifiap.conf
    $ wpa_supplicant -d -B -i mlan0 -c ./wifiap.conf
    $ systemctl start otbr_fwcfg  #if no systemd installed, use /usr/bin/otbr_fwcfg.sh instead

Then configure the Thread device:

On __i.MX 93 FRDM, i.MX 93 EVK, i.MX 95 15×15 EVK, i.MX 95 FRDM, i.MX 95 FRDM PRO, i.MX 8M Plus FRDM__, use IW612 as Thread device, on __i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM__, use IW610 as Thread device, execute the following commands to start the OTBR:

    # For i.MX 93 FRDM:
    $ otbr-agent-iwxxx -I wpan0 -B mlan0 'spinel+spi:///dev/spidev2.0?gpio-reset-device=/dev/gpiochip4&gpio-int-device=/dev/gpiochip5&gpio-int-line=10&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    # For i.MX 93 EVK, i.MX 95 15×15 EVK, i.MX 91 EVK, i.MX 91 QSB and i.MX 91 FRDM:
    $ otbr-agent-iwxxx -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip4&gpio-int-device=/dev/gpiochip5&gpio-int-line=10&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    # For i.MX 95 FRDM:
    otbr-agent-iwxxx -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip5&gpio-int-device=/dev/gpiochip3&gpio-int-line=8&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    # For i.MX 95 FRDM PRO:
    otbr-agent-iwxxx -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip6&gpio-int-device=/dev/gpiochip3&gpio-int-line=11&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &
    # For i.MX 8M Plus FRDM:
    otbr-agent-iwxxx -I wpan0 -B mlan0 'spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip6&gpio-int-device=/dev/gpiochip6&gpio-int-line=13&gpio-reset-line=12&spi-mode=0&spi-speed=1000000&spi-reset-delay=0' &

    $ iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $ iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT

**Note: To identify the SPI device name, [Check SPI device](./docs/guides/nxp_mpu_matter_demos.md#check-spi-dev). To determine gpio-reset-device and gpio-int-device, [check GPIO device](./docs/guides/nxp_mpu_matter_demos.md#check-gpio-device).**

On __i.MX 8M Mini EVK__, __i.MX 6ULL EVK__ or __i.MX 8ULP EVK__, use a dedicated Thread device (NXP K32W or any third party RCP).

Plugin the Thread module into the USB OTG port of __i.MX 8M Mini EVK__, __i.MX 6ULL EVK__ or __i.MX 8ULP EVK__. A USB device must be visible as _/dev/ttyUSB_ or _/dev/ttyACM_.
Once the USB device is detected, start the OTBR-related services.

When using the RCP module, programmed with OpenThread Spinel firmware image, execute the following commands:

    # If you are using third-party reference RCP
    $ otbr-agent -I wpan0 -B mlan0 spinel+hdlc+uart:///dev/ttyACM0 &

    # If you are using K32W RCP
    $ otbr-agent -I wpan0 -B mlan0 'spinel+hdlc+uart:///dev/ttyUSB0?uart-baudrate=1000000' &
    $ iptables -A FORWARD -i mlan0 -o wpan0 -j ACCEPT
    $ iptables -A FORWARD -i wpan0 -o mlan0 -j ACCEPT
    $ otbr-web &

A document explaining how to use Matter with OTBR and OpenThread on the i.MX MPU platform can be found in the [NXP Matter demos guide](docs/guides/nxp_mpu_matter_demos.md).

<a name="how-to-build-matter-application"></a>

# How to build Matter application

The Matter application has been installed into the Yocto image by default. If you want to build it separately, run the below commands to download the Matter application source code and switch to the v1.6 branch:

    $ mkdir ${MY_Matter_Apps}     # this is top-level directory of this project
    $ cd ${MY_Matter_Apps}
    $ git clone https://github.com/NXP/matter.git
    $ cd matter
    $ git checkout origin/v1.6-branch-imx_matter_2026_q2
    $ ./scripts/checkout_submodules.py --shallow --platform linux

 ___Make sure that the shell is not in the Yocto SDK environment___. Then, export a shell environment variable named IMX_SDK_ROOT to specify the path of the SDK.

    # For i.MX 93 FRDM, i.MX 93 EVK, i.MX 8M Mini EVK, i.MX 8ULP EVK, i.MX 91 EVK, i.MX 91 QSB, i.MX 91 FRDM and i.MX 95 15×15 EVK:
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.18-wrynose-imx8n9

    # For i.MX 6ULL EVK:
    $ export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/6.18-wrynose-imx6ull

User can build Matter applications (with the Yocto SDK specified by the IMX_SDK_ROOT) with the imxlinux_example.sh script. Refer to the below examples.

Assuming that the working directory is changed to the top-level directory of this project.

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

    # Build the imx-thread-br-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/thread-br-app/linux/ -o out/imx-thread-br-app -d

    # Build the chip-evse-app example with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/evse-app/linux/ -o out/evse-app -d

    # Build the Matter Controller tool with enhanced security using imxlinux_example.sh, by adding "-t" to the target. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/imx-chip-tool-trusty -t

    # Build the Matter lighting app with enhanced security using imxlinux_example.sh, by adding "-t" to the target. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/lighting-app/linux -o out/imx-lighting-app-trusty -t

    # Build the NXP customized thermostat application with enhanced security using imxlinux_example.sh, by adding "-t" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-thermostat/linux -o out/nxp-thermostat-trusty -t

    # Build the NXP customized media application with enhanced security using imxlinux_example.sh, by adding "-t" to the command. For example:
    $ ./scripts/examples/imxlinux_example.sh -s examples/nxp-media-app/linux/ -o out/nxp-media-trusty -t

    # Build the chip-tool-web2 application using imxlinux_example.sh, by adding "NXP_CHIPTOOL_WITH_WEB2=1" to the command. For example:
    $ NXP_CHIPTOOL_WITH_WEB2=1 ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/chip-tool-web2 -d

    # Build the NXP customized Zigbee bridge application with below command
    $ ./scripts/examples/imxlinux_example.sh -s examples/bridge-app/nxp/linux-imx -o out/zigbee-bridge/

The applications are built in out/ subdirectories; the subdirectory name is specified with the -o option, when building the examples. For example, the chip-all-clusters-app executable files can be found in \${MY_Matter_Apps}/connectedhomeip/out/all-clusters/.

___Make sure that the subdirectories do not exist before building an application with the same name.___
If an application must be built for several boards, the user can specify a board dedicated directory with the -o option; for example:

    $ ./scripts/examples/imxlinux_example.sh -s examples/chip-tool/ -o out/imx8mm-chip-tool -d

After executing the above command, the chip-tool executable files will be found in ${MY_Matter_Apps}/out/imx8mm-chip-tool/.

An official Matter document explaining how to use the chip-tool as a Matter controller can be found [here](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md).

A document explaining how to use Matter applications on the i.MX MPU platform can be found in the [NXP Matter demos guide](docs/guides/nxp_mpu_matter_demos.md). A document explaining how to use chip-tool-web2 application can be found in the [NXP chip-tool-web2 guide](docs/guides/nxp_chip_tool_web2_guide.md). A document explaining how to use NXP customized Zigbee bridge application imx-chip-bridge-app application can be found in the [NXP imx-chip-bridge-app guide](https://github.com/NXP/matter/blob/v1.6-branch-imx_matter_2026_q2/examples/bridge-app/nxp/linux-imx/README.md). A document explaining how to run Matter Commissioning in Home Assistant application based on i.MX MPU platforms can be found in the [NXP Matter HA guide](docs/guides/nxp_mpu_matter_Home_Assistant.md)

<a name="security-configuration-for-matter"></a>

# Security configuration for Matter

The i.MX Matter 2023 Q1 release enables hardware security on __i.MX 8M Mini__ to strengthen Matter security. Certification attestation and P256Keypair keys are protected by the ARM Trustzone and stored in secure storage using the [Trusty OS](https://source.android.com/docs/security/features/trusty) Trusted Execution Environment (TEE), in accordance with the CSA Matter Attestation of Security Requirements.

The i.MX Matter secure storage uses eMMC RPMB and is initialized, along with credential provisioning, using _fastboot_. Download _fastboot_ from [SDK Platform-Tools](https://developer.android.com/studio/releases/platform-tools) and then add it to your _${PATH}_. Follow the instructions below to initialize the secure storage.

    # Connect the OTG port of the i.MX 8M Mini to the host PC.
    # Boot the i.MX 8M Mini EVK board, during the U-Boot bootloader procedure, press any key on the target console to stop boot process and input U-Boot commands.
    u-boot=> fastboot 0

    # On host side, use fastboot command to initialise the RPMB partition as secure storage. Note that this is a one time programmable partition and cannot be revoked.
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

NXP maintains the Trusty OS, which contains the Trusted Application (TA) for i.MX Matter, and releases it as open source. Follow the instructions below to fetch and build the Trusty OS source code.

    $ repo init -u https://github.com/nxp-imx/imx-manifest.git -b imx-trusty-matter -m imx_trusty_matter_2026_q2.xml
    $ repo sync -c

    # Setup the build environment. This will only configure the current terminal.
    $ source trusty/vendor/google/aosp/scripts/envsetup.sh

    # Build the i.MX 8M Mini Trusty OS binary:
    $ ./trusty/vendor/google/aosp/scripts/build.py imx8mm --dynamic_param BUILD_MATTER=true
    # The target binary will be located at: build-root/build-imx8mm/lk.bin

    # Enable the secure storage service on first boot on i.MX 8M Mini Linux shell
    $ systemctl enable storageproxyd
    $ systemctl start storageproxyd

The i.MX Matter 2023 Q3 release integrates the built-in [ELE (EdgeLock Secure Enclave)](https://www.nxp.com/products/nxp-product-information/nxp-product-programs/edgelock-secure-enclave:EDGELOCK-SECURE-ENCLAVE) to enhance Matter security on __i.MX 93__, __i.MX 91__ and __i.MX 95 15×15__. To enable ELE, start the nvm_daemon service on the i.MX 93 and i.MX 91 Linux shell after each power cycle.

    $ systemctl start nvm_daemon

<a name="known-issue"></a>

## Known issue in i.MX Matter 2026 Q2 release

- Due to a known issue in BlueZ, when performing ble-wifi pairing between two devices using the i.MX 8M Mini platform (as commissioner or commissionee), the other i.MX device must also run the current release image. Using an older i.MX Matter release image on the peer device may result in BLE pairing failures.

<a name="faq"></a>

# FAQ

Q1 : Why the "zstd -d imx-image-multimedia-imx8mmevk.wic.zst" command cannot be executed in the folder ${MY_YOCTO}/bld-xwayland-imx8mm/tmp/deploy/images/imx8mmevk/?

A : Because imx-image-multimedia-imx8mmevk.wic.zst is a symbolic link file; zstd the link target file or copy imx-image-multimedia-imx8mmevk.wic.zst to another folder, then uncompress it using zstd.

    $ ls -al
    imx-image-multimedia-imx8mmevk.wic.zst -> imx-image-multimedia-imx8mmevk-20220721181418.rootfs.wic.zst

Q2 : What if the Yocto SDK Python3 is exported into the shell environment and makes the Matter bootstrap/active process fail?

A : Open a new shell, then remove the Yocto SDK environment and initialize the applications build environment.

    $ cd ${MY_Matter_Apps}
    $ rm -rf .environment
    $ source scripts/activate.sh

Q3 : How to download the official PAA files from CSA and how to use the official PAA files?

A : Connect the i.MX Matter device to a network that can access CSA resources, and then execute the following command. This stores the PAA files in the "/etc/dcl_paas" directory.

    $ dcldownloader

You can configure the macro with the following command, and then directly execute the chip-tool command to use it.

    $ export CHIPTOOL_PAA_TRUST_STORE_PATH=/etc/dcl_paas
    $ ${chip-tool command}

Another way is to add a suffix when executing the command, as shown below:

    $ ${chip-tool command} --paa-trust-store-path /etc/dcl_paas

>  **Note:** If you are using the official PAA files, the end Matter device must have the official DAC and PAI installed.

Q4 : How to save the commissioning information so that the board does not need to go through the commissioning process after a reboot?

A : Save the commissioning information using the following command:

    $ mkdir -p /etc/matter && export TMPDIR=/etc/matter

    After rebooting the device, re-export the `TMPDIR` environment using the following command:

    $ export TMPDIR=/etc/matter

Q5 : What should I do if I encounter insufficient storage space when downloading Docker containers on the i.MX 91 FRDM?

A  : Due to the limited eMMC storage capacity of the i.MX 91 FRDM, an SD card with a minimum capacity of 16 GB is required to run Home Assistant. Flash the i.MX 91 FRDM Matter yocto image to the SD card and boot it from the SD card. For instructions on how to run HA on the i.MX 91 FRDM, refer to the [Home Assistant guide](./docs/guides/nxp_mpu_matter_Home_Assistant.md).
