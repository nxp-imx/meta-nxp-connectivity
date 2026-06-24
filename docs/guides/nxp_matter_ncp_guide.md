# Running Matter MATTER-NCP demos on i.MX MPU platforms

This document describes how to use the MATTER-NCP demos on the i.MX MPU platforms. It shows how to run MATTER-NCP demos on the i.MX MPU platform and how to do wifi-bt commissioning between chip-tool and chip-all-clusters-app-ncp apps.

## Release Version Information

To ensure proper functionality, the MATTER‑NCP must be used in conjunction with the ncp‑device. Therefore, the MATTER‑NCP version released in this cycle is required to match the corresponding SDK release version.
The SDK release link associated with this MATTER‑NCP version is provided below. You may download the appropriate SDK source code from GitHub and use it to compile the ncp‑device.

MATTER-NCP RELEASE: 2026Q1
SDK RELEASE:26.03.00 Preview2
https://github.com/nxp-mcuxpresso/mcuxsdk-manifests

## Hardware requirements

- Two i.MX 8M Mini EVK boards (one board acts as the Matter controller, running chip-tool, and the other will be the end device, running the chip-all-clusters-app-ncp apps.)

The i.MX 8M Mini EVK, which running the chip-all-clusters-app-ncp apps connect with RDRW612 as the connection of ncp-host and ncp-device.

- RDRW612 A2 BGA board

The RDRW612 acts as ncp-device, and ensure that the BLE antenna is connected to RDRW612 board to prevent the BLE disconnection during BLE pairing.

The four interfaces USB, UART, SPI, and SDIO are supported between all-clusters-app-ncp and ncp-device, you can refer to this ncp user manual below to setup:
https://www.nxp.com/webapp/Download?colCode=UM12095

- Compilation of ncp device

Refer to the user manual: https://www.nxp.com/webapp/Download?colCode=UM12095

Go to sdk-next/mcuxsdk/examples/ncp_examples/ncp_device/app_config.cmake, and change as below:

    set(CONFIG_NCP_WIFI           1)
    set(CONFIG_NCP_BLE            1)
    set(CONFIG_NCP_OT             0)

Choose the interface that you want to verify.

    set(CONFIG_NCP_UART           0)
    set(CONFIG_NCP_SPI            0)
    set(CONFIG_NCP_USB            1)
    set(CONFIG_NCP_SDIO           0)

Close mDNS
Go to sdk-next/mcuxsdk/examples/_boards/rdrw612bga/ncp_examples/ncp_device/app_config.h and change as below:

    #define CONFIG_NCP_MDNS_ENABLE      0

## Verification about ble-wifi commissioning

Set up BT and connect to a wifi AP on controller device (DUT running chip-tool)
- step1. Save your Wi-Fi SSID and Password to a file.

    $ wpa_passphrase ${SSID} ${PASSWORD} > wifiap.conf

- step2. Setup BT and connected to a WiFi AP.

    $modprobe moal mod_para=nxp/wifi_mod_para.conf
    $wpa_supplicant -d -B -i mlan0 -c ./wifiap.conf
    $sleep 5
    $udhcpc -i mlan0
    $modprobe btnxpuart
    $hciconfig hci0 up

Run the example application on the end device, the following example uses a USB interface.

    $ export NCP_PORT=/dev/ttyUSB0, and for sdio: export NCP_PORT=/dev/mcu-sdio
    $ ./chip-all-clusters-app-ncp-usb --wifi --ble-device 0

For other interfaces (SPI, SDIO, and UART):

    $ ./chip-all-clusters-app-ncp-spi --wifi --ble-device 0
    $ ./chip-all-clusters-app-ncp-sdio --wifi --ble-device 0
    $ ./chip-all-clusters-app-ncp-uart --wifi --ble-device 0

Do commissioning and control the end devices on the controller device.

    $ ./chip-tool pairing ble-wifi 8888 ${SSID} ${PASSWORD} 20202021 3840

If you can see that the IP address is obtained on the device (all-cluster-app) side, it proves that the device has already connected to the external AP.

    [1748353915.770112][551:551] CHIP:DL: Got IP address on interface: ncp_wlan IP: 192.168.0.188

Then the controller and device will setup IP communication by case handshake, after that if you can see below log, it means that the commissioning process is successfully completed.

    CHIP:TOO: Device commissioning completed with success

## Matter chip-tool control chip-all-clusters-app-ncp apps after matter commission successfully

Read onoff status

    $ chip-tool onoff read on-off 8888 1

You can see this log in the chip-tool console log. OnOff reflects on onoff status. The value should be FALSE before enabling OnOff.

    [2025-05-22 14:25:04.603] [1747865814.155617][647:649] CHIP:TOO:   OnOff: FALSE

Toggle to enable onoff

    $ chip-tool onoff toggle 8888 1

Read onoff status again
    $ chip-tool onoff read on-off 8888 1

You can see this log in the chip-tool console log. OnOff reflects on onoff status. The value should be TRUE after enabling OnOff.

    [2025-05-22 14:25:04.603] [1747865814.155617][647:649] CHIP:TOO:   OnOff: TRUE
