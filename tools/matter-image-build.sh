#!/bin/bash

DEFAULT_MACHINES=(
    "imx93evk-iwxxx-matter"
    "imx91evk-iwxxx-matter"
    "imx91qsb-iwxxx-matter"
    "imx91frdm-iwxxx-matter"
    "imx8mmevk-matter"
    "imx8mpevk-matter"
    "imx8mnevk-matter"
    "imx8mnddr3levk-matter"
    "imx8ulpevk-matter"
    "imx6ullevk"
    "imx95-15x15-evk-iwxxx-matter"
    "imx95-19x19-evk-iwxxx-matter"
    "imx95-frdm-pro-iwxxx-matter"
    "imx95-frdm-iwxxx-matter"
)

detect_yocto_root() {
    local current_dir="$1"
    local max_levels=5
    local level=0
    
    while [ $level -lt $max_levels ]; do
        if [ -d "${current_dir}/sources" ] && [ -d "${current_dir}/sources/meta-nxp-connectivity" ]; then
            echo "${current_dir}"
            return 0
        fi
        
        if [ "${current_dir}" = "/" ]; then
            break
        fi
        
        current_dir=$(dirname "${current_dir}")
        level=$((level + 1))
    done
    
    return 1
}

select_codebase() {
    # If CODEBASE is already set, skip selection
    if [ -n "${CODEBASE}" ]; then
        return 0
    fi
    
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local detected_root=""
    
    detected_root=$(detect_yocto_root "${script_dir}")
    
    if [ -n "${detected_root}" ]; then
        local choice
        choice=$(whiptail --title "CODEBASE Configuration" \
            --yesno "\nDetected Yocto root directory:\n${detected_root}\n\nUse this as CODEBASE?" \
            12 70 3>&1 1>&2 2>&3)
        
        if [ $? -eq 0 ]; then
            CODEBASE="${detected_root}"
            export CODEBASE
            return 0
        fi
    fi
    
    local codebase_input
    if [ -n "${detected_root}" ]; then
        local default_path="${detected_root}"
    else
        local default_path="$(dirname "$(dirname "${script_dir}")")"
    fi
    
    codebase_input=$(whiptail --title "Set CODEBASE" \
        --inputbox "\nPlease enter the Yocto project root directory:" \
        10 70 "${default_path}" 3>&1 1>&2 2>&3)
    
    if [ $? -ne 0 ]; then
        echo "User canceled the operation. Exiting program." >&2
        exit 1
    fi
    
    if [ ! -d "${codebase_input}" ]; then
        whiptail --title "Error" --msgbox "\nDirectory does not exist: ${codebase_input}" 8 70
        select_codebase
        return
    fi
    
    if [ ! -d "${codebase_input}/sources" ]; then
        whiptail --title "Warning" --yesno "\nWarning: 'sources' directory not found in:\n${codebase_input}\n\nContinue anyway?" 10 70
        if [ $? -ne 0 ]; then
            select_codebase
            return
        fi
    fi
    
    CODEBASE="${codebase_input}"
    export CODEBASE
}

show_scp_menu() {
    local MENU_HEIGHT=10
    local MENU_WIDTH=60
    local CHOICE_HEIGHT=3

    if [ -z "${SCP_TARGET_PATH}" ]; then
        local BUTTON="Start"
    else
        local BUTTON="Next"
    fi

    local choice
    choice=$(whiptail --title "SCP Configuration" \
        --menu "\nChoose an action:" \
        $MENU_HEIGHT $MENU_WIDTH $CHOICE_HEIGHT \
        "$BUTTON" "Configure SCP target path and start building" \
        "Skip_SCP" "Skip SCP copy process" \
        "Cancel" "Cancel operation" 3>&1 1>&2 2>&3)

    if [ $? -ne 0 ] || [ "$choice" = "Cancel" ]; then
        echo "User canceled the operation. Exiting program." >&2
        exit 1
    fi

    if [ "$choice" = "Skip_SCP" ]; then
        SCP_TARGET_PATH=""
        return 0
    fi

    local scp_input
    scp_input=$(whiptail --title "Set SCP target path" \
        --inputbox "\nPlease enter the SCP target path:" \
        10 60 "${SCP_TARGET_PATH}" 3>&1 1>&2 2>&3)

    if [ $? -ne 0 ]; then
        echo "User canceled the operation. Exiting program." >&2
        exit 1
    fi

    SCP_TARGET_PATH="$scp_input"
    export SCP_TARGET_PATH
}

show_machine_menu() {
    if [ "$FORCE_FULL_BUILD" = true ]; then
        echo "'-f' flag detected. Selecting all default machines for the build."
        SELECTED_MACHINES=("${DEFAULT_MACHINES[@]}")
    else
        local machine_list=()
        for machine in "${DEFAULT_MACHINES[@]}"; do
            machine_list+=("$machine" " " ON)
        done

        local selected
        selected=$(whiptail --title "Select build target" \
                            --checklist "\nUse space to select the machines you want to build, use arrow keys to navigate" 20 60 10 "${machine_list[@]}" 3>&1 1>&2 2>&3)

        if [ $? -ne 0 ]; then
            echo "User canceled selection. Exiting program." >&2
            exit 1
        fi

        readarray -t SELECTED_MACHINES <<< "$(echo "$selected" | tr -d '"' | tr ' ' '\n')"
    fi

    if [ ${#SELECTED_MACHINES[@]} -eq 0 ]; then
        echo "Error: Please select at least one build target." >&2
        exit 1
    fi

    MACHINE_LIST=$(printf " %s" "${SELECTED_MACHINES[@]}")
    export MACHINE_LIST

    show_scp_menu
}

FORCE_FULL_BUILD=false
if [[ "$1" == "-f" ]]; then
    FORCE_FULL_BUILD=true
fi

# Setup CODEBASE first (required for LOG_FILE path)
if [ -z "${CODEBASE}" ]; then
    select_codebase
fi

show_machine_menu

SCP_TARGET_PATH="${SCP_TARGET_PATH:-}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

LOG_FILE="${CODEBASE}/build_$(date '+%Y%m%d_%H%M%S').log"
SEPARATOR="================================================================"

format_duration() {
    local duration=$1
    local hours=$((duration / 3600))
    local minutes=$(((duration % 3600) / 60))
    local seconds=$((duration % 60))
    printf "%02d:%02d:%02d" $hours $minutes $seconds
}

show_progress() {
    local current=$1
    local total=$2
    local width=50
    local percentage=$((current * 100 / total))
    local filled=$((width * current / total))
    local unfilled=$((width - filled))

    printf "\rProgress: ["
    printf "%${filled}s" | tr ' ' '#'
    printf "%${unfilled}s" | tr ' ' '-'
    printf "] %d/%d (%d%%)" "$current" "$total" "$percentage"
}

log() {
    local level=$1
    local message=$2
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    local color_level=""

    case $level in
        "INFO")    color_level="${BLUE}INFO${NC}" ;;
        "SUCCESS") color_level="${GREEN}SUCCESS${NC}" ;;
        "WARNING") color_level="${YELLOW}WARNING${NC}" ;;
        "ERROR")   color_level="${RED}ERROR${NC}" ;;
    esac

    echo -e "${CYAN}${timestamp}${NC} [${color_level}] ${message}"

    echo "${timestamp} [${level}] ${message}" >> "${LOG_FILE}"
}

copy_images() {
    if [ -z "${SCP_TARGET_PATH}" ]; then
        log "INFO" "Skipping copying as SCP_TARGET_PATH is not set"
        return 0
    fi

    log "INFO" "Starting to copy all built images"
    local failed_copies=""

    local remote_user_host=""
    local remote_path=""

    if [[ "${SCP_TARGET_PATH}" =~ ^([^:]+)?:(.*)$ ]]; then
        remote_user_host="${BASH_REMATCH[1]}"
        remote_path="${BASH_REMATCH[2]}"
    else
        log "ERROR" "Invalid SCP_TARGET_PATH format: ${SCP_TARGET_PATH}"
        return 1
    fi

    if [ -n "${remote_user_host}" ]; then
        log "INFO" "Checking if remote directory ${YELLOW}${remote_path}${NC} exists on ${YELLOW}${remote_user_host}${NC}"
        if ! ssh "${remote_user_host}" "[ -d \"${remote_path}\" ]"; then
            log "INFO" "Remote directory ${YELLOW}${remote_path}${NC} does not exist. Creating it..."
            if ! ssh "${remote_user_host}" "mkdir -p \"${remote_path}\""; then
                log "ERROR" "Failed to create remote directory ${RED}${remote_path}${NC} on ${RED}${remote_user_host}${NC}"
                return 1
            else
                log "SUCCESS" "Remote directory ${GREEN}${remote_path}${NC} created successfully on ${GREEN}${remote_user_host}${NC}"
            fi
        else
            log "INFO" "Remote directory ${YELLOW}${remote_path}${NC} already exists."
        fi
    else
        log "INFO" "Checking if local directory ${YELLOW}${remote_path}${NC} exists."
        if [ ! -d "${remote_path}" ]; then
            log "INFO" "Local directory ${YELLOW}${remote_path}${NC} does not exist. Creating it..."
            if ! mkdir -p "${remote_path}"; then
                log "ERROR" "Failed to create local directory ${RED}${remote_path}${NC}"
                return 1
            else
                log "SUCCESS" "Local directory ${GREEN}${remote_path}${NC} created successfully."
            fi
        else
            log "INFO" "Local directory ${YELLOW}${remote_path}${NC} already exists."
        fi
    fi

    for machine_name in "${SELECTED_MACHINES[@]}"; do
        local image_path="${CODEBASE}/bld-xwayland-${machine_name}/tmp/deploy/images/${machine_name}/imx-image-multimedia-${machine_name}.rootfs.wic.zst"
        log "INFO" "Copying image to target location for ${RED}${machine_name}${NC}"

        if ! scp "${image_path}" "${SCP_TARGET_PATH}"; then
            log "ERROR" "Failed to copy image for ${RED}${machine_name}${NC} to target location"
            failed_copies="${failed_copies} ${machine_name}"
        else
            log "SUCCESS" "Image copied successfully for ${RED}${machine_name}${NC}"
        fi
    done

    if [ -n "${failed_copies}" ]; then
        log "WARNING" "Failed copies: ${RED}${failed_copies}${NC}"
        return 1
    fi

    log "SUCCESS" "All images copied successfully"
    return 0
}

build_machine() {
    local machine_name=$1
    local build_start_time=$(date '+%Y-%m-%d %H:%M:%S')
    local build_start_seconds=$(date +%s)

    log "INFO" "Starting build for ${RED}${machine_name}${NC} at ${build_start_time}"

    cd "${CODEBASE}" || {
        log "ERROR" "Failed to change directory to ${CODEBASE}"
        return 1
    }

    log "INFO" "Setting up build environment for ${RED}${machine_name}${NC}"
    EULA=1 MACHINE="${machine_name}" DISTRO=fsl-imx-xwayland \
    source sources/meta-nxp-connectivity/tools/imx-matter-setup.sh "bld-xwayland-${machine_name}" >> "${LOG_FILE}" 2>&1

    if [ $? -ne 0 ]; then
        log "ERROR" "Environment setup failed for ${machine_name}"
        return 1
    fi

    source ../sources/imx-build-bamboo/build/hook-in-internal-servers.sh >> "${LOG_FILE}" 2>&1

    log "INFO" "Building imx-image-multimedia for ${RED}${machine_name}${NC}"

    local temp_output=$(mktemp)

    bitbake imx-image-multimedia 2>&1 | tee "${temp_output}" | while read line; do
        if [[ $line =~ "Running task "([0-9]+)" of "([0-9]+) ]]; then
            current_task=${BASH_REMATCH[1]}
            total_tasks=${BASH_REMATCH[2]}
            show_progress "$current_task" "$total_tasks"
        fi
        echo "$line" >> "${LOG_FILE}"
    done

    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        echo
        log "ERROR" "Build failed for ${RED}${machine_name}${NC}"
        return 1
    fi

    echo

    local build_end_time=$(date '+%Y-%m-%d %H:%M:%S')
    local build_end_seconds=$(date +%s)
    local build_duration=$((build_end_seconds - build_start_seconds))
    local formatted_duration=$(format_duration $build_duration)

    log "SUCCESS" "Build completed for ${RED}${machine_name}${NC}"
    log "INFO" "Build duration for ${RED}${machine_name}${NC}: ${GREEN}${formatted_duration}${NC} (Start: ${build_start_time}, End: ${build_end_time})"

    return 0
}

total_machines=${#SELECTED_MACHINES[@]}
current_machine=0
failed_machines=""
script_start_time=$(date +%s)

for machine_name in "${SELECTED_MACHINES[@]}"; do
    current_machine=$((current_machine + 1))
    log "INFO" "Processing machine ${current_machine}/${total_machines}: ${RED}${machine_name}${NC}"
    echo "${SEPARATOR}" >> "${LOG_FILE}"

    if build_machine "${machine_name}"; then
        log "INFO" "Successfully processed ${RED}${machine_name}${NC}"
    else
        failed_machines="${failed_machines} ${machine_name}"
        log "WARNING" "Failed to process ${RED}${machine_name}${NC}"
    fi
    sleep 200

    echo "${SEPARATOR}" >> "${LOG_FILE}"
done

script_end_time=$(date +%s)
total_duration=$((script_end_time - script_start_time))
formatted_total_duration=$(format_duration $total_duration)

log "INFO" "Build process completed"
log "INFO" "Total build time: ${formatted_total_duration}"
if [ -n "${failed_machines}" ]; then
    log "WARNING" "Failed builds:${failed_machines}"
else
    log "SUCCESS" "All builds completed successfully"
fi

if [ -z "${failed_machines}" ]; then
    log "INFO" "Starting image copy phase"
    copy_images
else
    log "WARNING" "Skipping image copy phase due to build failures"
fi

echo "${SEPARATOR}" >> "${LOG_FILE}"
log "INFO" "Log file location: ${LOG_FILE}"
