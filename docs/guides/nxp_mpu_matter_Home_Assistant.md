# Running Matter Commissioning in Home Assistant application on i.MX MPU platforms

The [Home Assistant](https://www.home-assistant.io/) (HA) application runs on both Android and iOS systems. By deploying the Home Assistant and Matter Server Docker containers on the i.MX MPU platform, you can use the HA application on the phone to manage Matter devices intuitively and easily. This document shows how to deploy the Home Assistant and Matter Server Docker containers on i.MX MPU platforms and then shows how to commission i.MX Matter devices in the HA application.

 [**Overview**](#overview)

 [**Deploying the Docker containers on the i.MX MPU platform**](#deploy-docker)

 [**Running the Home Assistant application on the Phone**](#running-app)

 [**Commissioning the i.MX Matter device on Home Assistant application**](#commissioning-with-phone)

 [**Known issue**](#known-issue)

 [**FAQ**](#faq)

<a name="overview"></a>

## Overview

The i.MX MPU platforms support deploying the HA and matter-server Docker containers. As shown in the figure below, by deploying Docker containers on an i.MX device, the i.MX device becomes the HA server, and the HA application can access the HA server via URL. You can then perform the commissioning.

 <img src="../images/home_assistant_demo/home-assistant.png" width = "500"/>

Figure. HA schematic diagram for i.MX MPU platform

<a name="deploy-docker"></a>

## Deploying the Docker containers on the i.MX MPU platform

### i.MX 91 FRDM pre-deployment steps

To run HA on the i.MX 91 FRDM, please flash the i.MX 91 FRDM Matter Yocto image to an SD card with a capacity of at least 16 GB and boot the board from the SD card. Before downloading and deploying the Home Assistant and Matter Server Docker images on the i.MX 91 FRDM, please perform the following two steps. ___If you are using other platforms, you can skip this section and directly move on to deploying the Docker containers in the next section.___

Step 1. Create a new partition to store the Docker images.

    root@imx91frdm-iwxxx-matter:~# fdisk /dev/mmcblk1

        Welcome to fdisk (util-linux 2.41.1).
        Changes will remain in memory only, until you decide to write them.
        Be careful before using the write command.

        This disk is currently in use - repartitioning is probably a bad idea.
        It's recommended to umount all file systems, and swapoff all swap
        partitions on this disk.

        Command (m for help): p

        Disk /dev/mmcblk1: 59.48 GiB, 63864569856 bytes, 124735488 sectors
        Units: sectors of 1 * 512 = 512 bytes
        Sector size (logical/physical): 512 bytes / 512 bytes
        I/O size (minimum/optimal): 512 bytes / 512 bytes
        Disklabel type: dos
        Disk identifier: 0x076c4a2a

        Device         Boot  Start      End  Sectors  Size Id Type
        /dev/mmcblk1p1 *     16384   540671   524288  256M  c W95 FAT32 (LBA)
        /dev/mmcblk1p2      540672 14172159 13631488  6.5G 83 Linux

        Command (m for help): n

        Partition type
        p   primary (2 primary, 0 extended, 2 free)
        e   extended (container for logical partitions)
        Select (default p): p
        Partition number (3,4, default 3): 3
        First sector (2048-124735487, default 2048): 14180000
        Last sector, +/-sectors or +/-size{K,M,G,T,P} (14180000-124735487, default 124735487):

        Created a new partition 3 of type 'Linux' and of size 52.7 GiB.

        Command (m for help): w
        The partition table has been altered.
        Syncing disks.

    root@imx91frdm-iwxxx-matter:~# mkfs.ext4 /dev/mmcblk1p3
        mke2fs 1.47.3 (8-Jul-2025)
        Discarding device blocks: done
        Creating filesystem with 13819436 4k blocks and 3457024 inodes
        Filesystem UUID: 77309a3c-1448-47fe-ba73-78cb933720cd
        Superblock backups stored on blocks:
                32768, 98304, 163840, 229376, 294912, 819200, 884736, 1605632, 2654208,
                4096000, 7962624, 11239424

        Allocating group tables: done
        Writing inode tables: done
        Creating journal (65536 blocks): done
        Writing superblocks and filesystem accounting information: done

Step 2. Mount the new partition to the "~/image" folder and restart the docker service using the commands below.

    root@imx91frdm-iwxxx-matter:~# mkdir image
    root@imx91frdm-iwxxx-matter:~# mount /dev/mmcblk1p3 image
    root@imx91frdm-iwxxx-matter:~# mkdir ~/image/docker
    root@imx91frdm-iwxxx-matter:~# mkdir ~/image/containerd
    root@imx91frdm-iwxxx-matter:~# systemctl stop docker
    root@imx91frdm-iwxxx-matter:~# cd /var/lib/
    root@imx91frdm-iwxxx-matter:/var/lib# mv docker docker_old
    root@imx91frdm-iwxxx-matter:/var/lib# mv containerd containerd_old
    root@imx91frdm-iwxxx-matter:/var/lib# ln -s ~/image/docker/ ./
    root@imx91frdm-iwxxx-matter:/var/lib# ln -s ~/image/containerd/ ./
    root@imx91frdm-iwxxx-matter:/var/lib# systemctl start docker
    root@imx91frdm-iwxxx-matter:/var/lib# systemctl restart containerd

### Deploying the Docker containers on all supported i.MX MPU Platforms

Download and deploy the homeassistant and matter-server Docker images.

    $ docker run -d --name homeassistant --privileged --restart=unless-stopped -e TZ=MY_TIME_ZONE -v $(pwd)/config:/config -v /run/dbus:/run/dbus:ro --network=host ghcr.io/home-assistant/home-assistant:2026.3
    $ docker run -d --name matter-server --restart=unless-stopped --security-opt apparmor=unconfined -v $(pwd)/data:/data --network=host ghcr.io/home-assistant-libs/python-matter-server:8.1.0 --storage-path /data --paa-root-cert-dir /data/credentials

It takes a few minutes to download and deploy the images. You can check the images by running "$ docker image" after the deployment is complete.

    root@imx91frdm-iwxxx-matter:~# docker images
                                                                                                        i Info    U  In Use
    IMAGE                                                    ID             DISK USAGE   CONTENT SIZE   EXTRA
    ghcr.io/home-assistant-libs/python-matter-server:8.1.0   170aa093ce91        644MB          144MB    U
    ghcr.io/home-assistant/home-assistant:2026.3             916682086154        3.3GB          597MB    U

<a name="running-app"></a>

## Running the Home Assistant application on the Phone

Before running the HA application, you need to perform some environmental configuration.
First, you can use the commands below to connect to the Wi-Fi AP on the i.MX Docker device and check its IP address.

    $ wpa_passphrase ${SSID} ${PASSWORD} > wifiap.conf

    $ ifconfig eth0 down
    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ wpa_supplicant -d -B -i mlan0 -c ./wifiap.conf
    $ sleep 5
    $ modprobe btnxpuart
    $ hciconfig hci0 up

    $ ifconfig mlan0       # check IP address

If you need to commission a Thread device in HA, [setup OTBR on the i.MX Docker device](./nxp_mpu_matter_demos.md#running-matter-demos-with-otbr-and-openthread-on-the-imx-mpu-platform).

Then, connect to the same Wi-Fi AP, enable Bluetooth on your phone, and open the HA app on your phone and configure the HA server URL.

If you are a new user of this application, you can set the URL when you log in to this application. Enter the URL and click the "Connect" button. The URL should be in the format  `http://<IP_ADDRESS>:8123 `, where <IP_ADDRESS> is the IP address of your i.MX device.

 <img src="../images/home_assistant_demo/config-url_newuser.png" width = "200"/>

Figure. Configuring the Home Assistant server URL when logging in

Or you can configure the URL in the settings according to the below steps, click the "SETTINGS", "Home", "Home Assistant URL" buttons in sequence on the following page to update the URL.

<img src="../images/home_assistant_demo/config-url_1.png" width = "200"/> <img src="../images/home_assistant_demo/config-url_2.png" width = "200"/> <img src="../images/home_assistant_demo/config-url_3.png" width = "200"/>

Figure. Configuring the Home Assistant server URL in Settings

Once you have connected the Home Assistant server, you should integrate the Python Matter server to communicate with the i.MX Matter device. You will also need to integrate the Thread and OpenThread Border Router server if you need to commission the Matter Thread device.

Integrate the Python Matter server into the Phone application:

Click "Setting" – "Devices & services" – "+ Add integration" at the bottom-right corner – search for "Matter" – select the "Matter" – enter URL "ws://localhost:5580/ws" – "Submit" to integrate Python Matter Server.

<img src="../images/home_assistant_demo/config-matter_server.png" width = "200"/><img src="../images/home_assistant_demo/config-matter_server-1.png" width = "200"/>

Figure. Integrate the Python Matter server

Integrate the Thread service into the HA instance:

Click "Setting" – "Devices & services" – "+ Add integration" at the bottom-right corner – search for "Thread" - select the "Thread". The Thread service is added to HA instance as shown below.

<img src="../images/home_assistant_demo/thread_1.png" width = "200"/><img src="../images/home_assistant_demo/thread_2.png" width = "200"/>

Figure. Integrate the Thread service

Integrate the Open Thread Border Router REST API into HA instance:

Click "Setting" – "Devices & services" – "+ Add integration" at the bottom-right corner - search for "Open Thread Border Router" - select the "Open Thread Border Router" – Enter URL "http://ip:8081" (IP is the otbr-agent device's IP, it uses 8081 port for REST API by default) – Submit, the Open Thread Border Router REST API will add into HA instance.

<img src="../images/home_assistant_demo/config-otbr_1.png" width = "200"/><img src="../images/home_assistant_demo/config-otbr_2.png" width = "200"/>

Figure. Integrate the Thread Border Router

Set the preferred network and sync Thread credentials.

Go to "Setting" - "Devices & services" - "Thread" - click the configuration logo - set or check the right border router as the preferred network.
Then, go to "Setting" – "Companion app" – "Troubleshooting" – "Sync Thread credentials" to Sync the credentials.

<img src="../images/home_assistant_demo/config-otbr_3.png" width = "200"/><img src="../images/home_assistant_demo/config-otbr_4.png" width = "200"/>

Figure. Setting the preferred network and sync the Thread credentials.

<a name="commissioning-with-phone"></a>

## Commissioning the i.MX Matter device on Home Assistant application

This chapter shows how to commission an i.MX Matter device on HA application.

First, set up the i.MX Matter device. There are two ways to run the Matter application. Take the chip-lighting-app as an example. You can also setup the Matter application on a Thread device.

Run the commands below if the i.MX Matter device is i.MX 93 or i.MX 91 to enable the ELE. For other boards, proceed to Option 1 or Option 2.

    $ systemctl enable nvm_daemon
    $ systemctl start nvm_daemon
    $ systemctl status nvm_daemon # make sure the daemon active and running.

Option 1

    $ wpa_passphrase ${SSID} ${PASSWORD} > wifiap.conf
    $ ifconfig eth0 down
    $ ifconfig eth1 down
    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ wpa_supplicant -d -B -i mlan0 -c ./wifiap.conf
    $ sleep 5
    $ modprobe btnxpuart
    $ hciconfig hci0 up
    $ chip-lighting-app

Option 2

    $ ifconfig eth0 down
    $ ifconfig eth1 down
    $ modprobe moal mod_para=nxp/wifi_mod_para.conf
    $ modprobe btnxpuart
    $ hciconfig hci0 up
    $ chip-lighting-app --wifi --ble-device 0

After running the chip-lighting-app, you will find a log line similar to the one below, copy the URL and open it in a browser, you can see the QR code of this Matter application.

    CHIP:SVR: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J042C00KA0648G00

Then, you can commission i.MX Matter application as shown in the following pictures. Click the "Settings", "Devices & services", "+ Add integration", "Add Matter device", "No, it's new.", "Open camera here" buttons in sequence on the following pages to start scanning the QR code of the i.MX Matter application.

<img src="../images/home_assistant_demo/app_1.png" width = "200"/> <img src="../images/home_assistant_demo/app_2.png" width = "200"/> <img src="../images/home_assistant_demo/app_3.png" width = "200"/> <img src="../images/home_assistant_demo/app_4.png" width = "200"/> <img src="../images/home_assistant_demo/app_5.png" width = "200"/> <img src="../images/home_assistant_demo/app_6.png" width = "200"/>

Figure. How to start the commissioning on the phone app

Once the QR code is scanned, it goes through the following processes to connect to the i.MX Matter application chip-lighting-app.

<img src="../images/home_assistant_demo/connect_1.png" width = "200"/> <img src="../images/home_assistant_demo/connect_2.png" width = "200"/> <img src="../images/home_assistant_demo/connect_3.png" width = "200"/> <img src="../images/home_assistant_demo/connect_4.png" width = "200"/>

Figure. Device connection procedure

Once the device has been successfully connected, you are able to control it.

<a name="known-issue"></a>

## Known issue

- When commissioning with the chip-all-cluster-app, you may encounter a "Something went wrong" error in the final steps of the commissioning process on the Home Assistant app. This is a known issue related to the Home Assistant Docker environment.

<a name="faq"></a>

## FAQ

### What to do if the download fails or the download speed of the Docker image is slow?

The download failure or slow download speed may be caused by network issues. Use the following commands to set the proxies for the Docker service. Then retry to download and deploy the Docker image again.

    $ mkdir /etc/systemd/system/docker.service.d
    $ vi /etc/systemd/system/docker.service.d/http-proxy.conf
        [Service]
        Environment="HTTP_PROXY=${YOUR_HTTP_PROXY_URL}:${YOUR_HTTP_PROXY_PORT}"
        Environment="HTTPS_PROXY=${YOUR_HTTPS_PROXY_URL}:${YOUR_HTTPS_PROXY_PORT}"
        Environment="NO_PROXY=localhost,127.0.0.*,${YOUR_HOST_network_prefix}.*"

    $ systemctl daemon-reload
    $ systemctl restart docker

### What to do if the commissioning fails?

Check the status of the Docker service, and the network status and the status of the Bluetooth interface.

    $ systemctl status docker    # check the Docker service status
    $ systemctl start docker     # start the Docker service
    $ ifconfig mlan0             # check the Network status
    $ hciconfig hci0             # check the Bluetooth status

You need to make sure that the Docker service is active, an IP has been assigned to the mlan0 interface, and that hci0 is up and running.

        root@imx93evk-iwxxx-matter:~# systemctl status docker
        * docker.service - Docker Application Container Engine
            Loaded: loaded (8;;file://imx93evk-iwxxx-matter/usr/lib/systemd/system/docker.service/usr/lib/systemd/system/docker.service8;;; disabled; preset: disabled)8;;
            Active: active (running) since Tue 2024-10-22 07:09:36 UTC; 36s ago
        TriggeredBy: * docker.socket
            Docs: 8;;https://docs.docker.comhttps://docs.docker.com8;;8;;
        Main PID: 681 (dockerd)
            Tasks: 12
            Memory: 98.1M (peak: 100.0M)
                CPU: 1.345s
            CGroup: /system.slice/docker.service
                `-681 /usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock

        root@imx93evk-iwxxx-matter:~# ifconfig mlan0

        mlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.3.119  netmask 255.255.254.0  broadcast 192.168.3.255
        inet6 2001:470:11e:f:a2cd:f3ff:fe77:e72e  prefixlen 64  scopeid 0x0<global>
        inet6 fe80::a2cd:f3ff:fe77:e72e  prefixlen 64  scopeid 0x20<link>
        ether a0:cd:f3:77:e7:2e  txqueuelen 1000  (Ethernet)
        RX packets 2766  bytes 184053 (179.7 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 643  bytes 71774 (70.0 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

        root@imx93evk-iwxxx-matter:~# hciconfig hci0
        hci0:   Type: Primary  Bus: UART
                BD Address: A0:CD:F3:77:E7:2F  ACL MTU: 1021:7  SCO MTU: 120:6
                UP RUNNING
                RX bytes:849 acl:0 sco:0 events:57 errors:0
                TX bytes:1078 acl:0 sco:0 commands:57 errors:0

If the device is connected to a network cable, use the following command to turn off the Ethernet interface.

        $ ifconfig eth0 down

### What to do if the commissioning fails with matter-server logs "CHIP_ERROR [chip.native.DIS] Timeout waiting for mDNS resolution."?

When commissioning device over BLE or IP failures occur, especially when OTBR is set up. You need to check the matter-server container's logs by "$ docker logs matter-server", if the logs include "CHIP_ERROR [chip.native.DIS] Timeout waiting for mDNS resolution", you can try to resolve this issue by running below commands to clear avahi cache then retry the commissioning process.

	$ sudo systemctl stop avahi-daemon
	$ sudo rm -rf /var/run/avahi-daemon/*
	$ sudo systemd-resolve --flush-caches
	$ sudo systemctl start avahi-daemon

### What should be done when an attempt to add the OpenThread Border Router's REST API fails with a 'Failed to connect' error?

Add the option "--rest-listen-address 0.0.0.0" when starting otbr-agent or otbr-agent-iwxxx, for example:"

	$ otbr-agent-iwxxx -I wpan0 -B mlan0 --rest-listen-address 0.0.0.0 spinel+spi:///dev/spidev0.0?gpio-reset-device=/dev/gpiochip5&gpio-int-device=/dev/gpiochip4&gpio-int-line=10&gpio-reset-line=1&spi-mode=0&spi-speed=1000000&spi-reset-delay=0
