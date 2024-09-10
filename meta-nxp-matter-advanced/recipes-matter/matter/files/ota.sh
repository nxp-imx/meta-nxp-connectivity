#!/bin/sh

set -e

# Usage function to display help
usage() {
    echo "Usage: sudo $0 -o [8987|IW612|nxp-thermostat-app] -d 18"
    exit 1
}

# Parse command line arguments
while getopts ":o:d:" opt; do
    case $opt in
        o)
            option=$OPTARG
            ;;
        d)
            discriminator=$OPTARG
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            ;;
    esac
done

# Check if -o option is provided and not empty
if [ -z "$option" ]; then
    echo "Error: -o option is required."
    usage
fi

# Check if -d option is provided and not empty
if [ -z "$discriminator" ]; then
    echo "Error: -d option is required."
    usage
fi

# Run ota_requestor app to start a requestor
run_ota_requestor(){
    ota_requestor_cmd="chip-ota-requestor-app \
        --discriminator $discriminator \
        --secured-device-port 5560 \
        --KVS /tmp/chip_kvs_requestor \
        --periodicQueryTimeout 60 \
        --otaDownloadPath /root/$1"
    echo "Executing: $ota_requestor_cmd"

    $ota_requestor_cmd | {
        result=false
        while IFS= read -r line; do
            echo "$line"
            if [[ "$line" == *"CHIP:ATM: BlockAckEOF"* ]]; then
                sleep 2
                pid=$(ps -aux | grep 'chip-ota-requestor-app' | grep -v 'grep'| awk '{print $2}')
                if [[ -n "$pid" ]]; then
                    kill "$pid"
                fi
                result=true
                break
            fi
        done
        $result && return 0 || return 1
    }
}

# Update the Firmware with the new ota file
update_FW(){
    ifconfig mlan0 down
    ifconfig wfd0 down
    killall wpa_supplicant
    modprobe -r moal
    fw_name=$1
    mv /usr/lib/firmware/nxp/$fw_name /usr/lib/firmware/nxp/${fw_name}_backup
    chmod a+x /root/$fw_name
    mv /root/$fw_name /usr/lib/firmware/nxp/$fw_name
    if [ $? -eq 0 ]; then
        return 0
    else
        return 1
    fi
}

# Rollback the Firmware
rollback_FW(){
    fw_name=$1
    rm /usr/lib/firmware/nxp/${fw_name}
    mv /usr/lib/firmware/nxp/${fw_name}_backup /usr/lib/firmware/nxp/${fw_name}
    if [ $? -eq 0 ]; then
        return 0
    else
        return 1
    fi
}

# Kill the runnning nxp-thermostat-app process
kill_thermostat_process(){
    set +e
    pid=$(pgrep -f '^nxp-thermostat-app')
    set -e
    if [ -n "$pid" ]; then
        echo "Found running nxp-thermostat-app process with PID: $pid. Attempting to kill..."
        kill $pid
        if [ $? -eq 0 ]; then
            echo "Process nxp-thermostat-app pid: $pid has been terminated."
        else
            echo "Failed to kill process $pid."
        fi
    else
        echo "No running process named nxp-thermostat-app found."
    fi
}

# Run the nxp-thermostat-app to check if it run normal
run_thermostat_app(){
    TIMEOUT=5
    start_time=$(date +%s)
    nxp-thermostat-app | {
        result=false
        while IFS= read -r line; do
            echo "$line"
            if [[ "$line" == *"Updating services using commissioning mode 1"* ]]; then
                result=true
                kill_thermostat_process
                break
            fi

            if [[ $(date +%s) -ge $((start_time + TIMEOUT)) ]]; then
                echo "Running nxp-thermostat-app fail"
                kill_thermostat_process
                break
            fi
        done
        $result && return 0 || return 1
    }
}

# Update the nxp-thermostat-app with the new ota file
update_nxp_thermostat_app(){
    mv /usr/bin/nxp-thermostat-app /usr/bin/nxp-thermostat-app_backup
    chmod a+x /root/nxp-thermostat-app
    mv /root/nxp-thermostat-app /usr/bin/nxp-thermostat-app
    if run_thermostat_app; then
        return 0
    else
        return 1
    fi
}

# Rollback the nxp-thermostat-app if update ota file fail
rollback_nxp_thermostat_app(){
    if [ -f /usr/bin/nxp-thermostat-app ]; then
        mv /usr/bin/nxp-thermostat-app /usr/bin/nxp-thermostat-app_ota
    fi
    if [ -f /usr/bin/nxp-thermostat-app_backup ]; then
        mv /usr/bin/nxp-thermostat-app_backup /usr/bin/nxp-thermostat-app
        return 0
    else
        echo "Rollback nxp-thermostat-app failed because the backup binary doesn't exist"
        return 1
    fi
}

run_FW_ota(){
    download=$1
    if run_ota_requestor "$download"; then
        echo "Get OTA file succeeded."
        if update_FW "$download"; then
            echo "OTA update FW succeeded, Please reboot the i.MX device and re-setup the Wi-Fi and BT."
        else
            echo "OTA update FW failed, will rollback FW."
            if rollback_FW "$download"; then
                echo "Rollback update FW succeeded, please check the ota file and retry OTA."
            else
                echo "Rollback update FW failed, please manual check."
            fi
        fi
    else
        echo "Run chip-ota-requestor-app to get OTA file failed."
    fi
}

# Run ota based on the value of -o
case $option in
    8987)
        download="sdiouart8987_combo_v0.bin"
        echo "OTA download file: $download"
        run_FW_ota "$download"
        ;;
    IW612)
        download="sduart_nw61x_v1.bin.se"
        echo "OTA download file: $download"
        run_FW_ota "$download"
        ;;
    nxp-thermostat-app)
        download="nxp-thermostat-app"
        echo "OTA download file: $download"
        kill_thermostat_process
        if run_ota_requestor "$download"; then
            echo "Get OTA file succeeded."
            if update_nxp_thermostat_app; then
                echo "OTA update nxp-thermostat-app succeeded."
            else
                echo "OTA update nxp-thermostat-app failed, will rollback."
                if rollback_nxp_thermostat_app; then
                    echo "Rollback nxp-thermostat-app succeeded."
                else
                    echo "Rollback nxp-thermostat-app failed, please manual check."
                fi
            fi
        else
             echo "Get OTA file failed, please check OTA file and OTA provider."
        fi
        ;;
    *)
        echo "Invalid option: $option" >&2
        usage
        ;;
esac
