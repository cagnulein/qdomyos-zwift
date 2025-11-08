#!/usr/bin/env bash
# =============================================================================
# QDomyos-Zwift: Test Plan Cleanup Script
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# AI analysis tools (Claude, Gemini) were used to assist coding and debugging
# Licensed under GPL-3.0 - see project repository for full license
#
# This script resets a system to a clean baseline state for testing the ANT+
# feature installation process. It removes all components installed during
# testing to prepare for the next test iteration.
#
# WARNING: This script performs destructive actions, including removing
# packages and deleting configuration files. Use with caution.
# =============================================================================

set -eo pipefail

# --- Color Definitions ---
C_RED="\033[0;31m"
C_YELLOW="\033[0;33m"
C_CYAN="\033[0;36m"
C_GREEN="\033[0;32m"
C_RESET="\033[0m"

# --- Helper Functions ---
info() { echo -e "${C_CYAN}>>> $*${C_RESET}"; }
warn() { echo -e "${C_YELLOW}⚠ WARNING: $*${C_RESET}"; }
success() { echo -e "${C_GREEN}✓ $*${C_RESET}"; }
prompt() { read -p "$1 (y/N) " -n 1 -r; echo; }

# --- Main Cleanup Logic ---
main() {
    clear
    echo -e "${C_CYAN}========================================================"
    echo -e "      QDomyos-Zwift Test Plan Cleanup Script"
    echo -e "========================================================${C_RESET}"
    echo
    echo "This script resets your system to a clean baseline state for testing."
    echo
    warn "This script will perform the following IRREVERSIBLE actions:"
    echo "  - Stop and disable the 'qz.service'"
    echo "  - Delete the 'qdomyos-zwift' binary and related scripts"
    echo "  - DELETE the Python virtual environment (~/ant_venv)"
    echo "  - DELETE the pyenv installation (~/.pyenv) if present"
    echo "  - Remove pyenv configuration from shell profile (~/.bashrc)"
    echo "  - Remove the USB permissions file for ANT+ dongles"
    echo "  - Remove user from the 'plugdev' group"
    echo "  - BACKUP and delete root user's QZ configuration file"
    echo
    warn "This is designed for testers following the Test Plan procedure."
    warn "Back up any important configuration files before proceeding."
    echo
    prompt "Are you absolutely sure you want to proceed?"
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted by user."
        exit 1
    fi

    # --- Step 1: Stop and Disable Systemd Service ---
    info "Step 1: Stopping and disabling the qz.service..."
    if sudo systemctl stop qz 2>/dev/null; then 
        success "Service stopped."
    else 
        warn "Service was not running."
    fi
    
    if sudo systemctl disable qz 2>/dev/null; then 
        success "Service disabled."
    else 
        warn "Service was not enabled."
    fi
    
    if [ -f "/etc/systemd/system/qz.service" ]; then
        sudo rm -f /etc/systemd/system/qz.service
        sudo systemctl daemon-reload
        success "Service file removed and systemd reloaded."
    fi

    # --- Step 2: Remove Binary and Scripts ---
    info "Step 2: Removing application binary and scripts..."
    local QZ_BINARY="$HOME/qdomyos-zwift"
    local CHECK_SCRIPT="$HOME/runtime_check.sh"

    if [ -f "$QZ_BINARY" ]; then 
        rm -f "$QZ_BINARY"
        success "Removed binary: $QZ_BINARY"
    else
        warn "Binary not found: $QZ_BINARY"
    fi
    
    if [ -f "$CHECK_SCRIPT" ]; then 
        rm -f "$CHECK_SCRIPT"
        success "Removed runtime check script: $CHECK_SCRIPT"
    else
        warn "Runtime check script not found: $CHECK_SCRIPT"
    fi

    # --- Step 3: Remove Python Virtual Environment ---
    info "Step 3: Removing Python virtual environment..."
    local VENV_DIR="$HOME/ant_venv"
    
    if [ -d "$VENV_DIR" ]; then 
        rm -rf "$VENV_DIR"
        success "Removed Python venv: $VENV_DIR"
    else
        warn "Virtual environment not found: $VENV_DIR"
    fi

    # --- Step 4: Remove pyenv Installation (Optional) ---
    info "Step 4: Checking for pyenv installation..."
    local PYENV_DIR="$HOME/.pyenv"
    
    if [ -d "$PYENV_DIR" ]; then
        warn "Found pyenv installation at: $PYENV_DIR"
        prompt "Do you want to remove pyenv? (This is optional if you use it for other projects)"
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            rm -rf "$PYENV_DIR"
            success "Removed pyenv directory: $PYENV_DIR"
            
            # Clean shell configuration
            if [ -f "$HOME/.bashrc" ]; then
                info "Cleaning pyenv configuration from ~/.bashrc..."
                sed -i.bak '/export PYENV_ROOT/d' "$HOME/.bashrc"
                sed -i '/command -v pyenv/d' "$HOME/.bashrc"
                sed -i '/eval "$(pyenv init -)"/d' "$HOME/.bashrc"
                success "Removed pyenv configuration (backup: ~/.bashrc.bak)"
            fi
        else
            warn "Keeping pyenv installation."
        fi
    else
        success "No pyenv installation found (system Python 3.11 likely used)."
    fi

    # --- Step 5: Remove USB Permissions ---
    info "Step 5: Removing USB permissions and udev rules..."
    local UDEV_RULE="/etc/udev/rules.d/99-ant-usb.rules"
    
    if [ -f "$UDEV_RULE" ]; then
        sudo rm -f "$UDEV_RULE"
        sudo udevadm control --reload-rules && sudo udevadm trigger
        success "Removed udev rule and reloaded udev."
    else
        warn "Udev rule not found: $UDEV_RULE"
    fi

    # --- Step 6: Remove User from plugdev Group ---
    info "Step 6: Removing user from plugdev group..."
    if groups "$USER" | grep -q '\bplugdev\b'; then
        sudo gpasswd -d "$USER" plugdev
        success "User '$USER' removed from plugdev group."
        warn "Group change requires logout/login or reboot to take effect."
    else
        warn "User '$USER' is not in the plugdev group."
    fi

    # --- Step 7: Backup and Remove Root Configuration ---
    info "Step 7: Handling root configuration file..."
    local ROOT_CONFIG_DIR="/root/.config/Roberto Viola"
    
    if sudo [ -d "$ROOT_CONFIG_DIR" ]; then
        local BACKUP_PATH="/root/.config/Roberto Viola.backup-$(date +%s)"
        sudo cp -r "$ROOT_CONFIG_DIR" "$BACKUP_PATH" 2>/dev/null || true
        sudo rm -rf "$ROOT_CONFIG_DIR"
        success "Root configuration backed up to: $BACKUP_PATH"
        success "Root configuration removed."
    else
        warn "No root configuration found at: $ROOT_CONFIG_DIR"
    fi
    
    # --- Step 8: Optional - Remove System Dependencies ---
    echo
    info "Step 8: System package cleanup (optional)..."
    warn "Removing system packages may affect other applications."
    warn "Only remove these if you're sure they're not needed elsewhere."
    prompt "Do you want to see the command to remove all system dependencies?"
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo
        info "To remove system dependencies, run this command manually:"
        echo
        echo -e "${C_YELLOW}# Remove Qt5 libraries${C_RESET}"
        echo -e "${C_YELLOW}sudo apt-get remove -y libqt5bluetooth5 libqt5charts5 libqt5multimedia5"
        echo -e "  libqt5networkauth5 libqt5positioning5 libqt5sql5 libqt5texttospeech5"
        echo -e "  libqt5websockets5 libqt5xml5${C_RESET}"
        echo
        echo -e "${C_YELLOW}# Remove Python 3.11 (only if not needed by other software)${C_RESET}"
        echo -e "${C_YELLOW}sudo apt-get remove -y python3.11 python3.11-venv${C_RESET}"
        echo
        echo -e "${C_YELLOW}# Remove USB utilities${C_RESET}"
        echo -e "${C_YELLOW}sudo apt-get remove -y libusb-1.0-0 usbutils${C_RESET}"
        echo
        echo -e "${C_YELLOW}# Remove pyenv build dependencies (if pyenv was used)${C_RESET}"
        echo -e "${C_YELLOW}sudo apt-get remove -y build-essential libssl-dev zlib1g-dev libbz2-dev"
        echo -e "  libreadline-dev libsqlite3-dev wget curl llvm libncurses5-dev"
        echo -e "  libncursesw5-dev xz-utils tk-dev libffi-dev liblzma-dev${C_RESET}"
        echo
        echo -e "${C_YELLOW}# Clean up unused packages${C_RESET}"
        echo -e "${C_YELLOW}sudo apt-get autoremove -y${C_RESET}"
        echo
    fi

    # --- Step 9: Verification Summary ---
    echo
    echo -e "${C_CYAN}========================================================"
    echo -e "                  Cleanup Summary"
    echo -e "========================================================${C_RESET}"
    echo
    success "Test Plan cleanup process complete!"
    echo
    echo "Your system has been reset to a clean baseline state."
    echo
    echo "Verification checklist (run these commands to confirm):"
    echo -e "  ${C_YELLOW}ls ~/qdomyos-zwift${C_RESET}          # Should show 'No such file'"
    echo -e "  ${C_YELLOW}ls ~/ant_venv${C_RESET}               # Should show 'No such file'"
    echo -e "  ${C_YELLOW}python3.11 --version${C_RESET}        # May still exist (system package)"
    echo -e "  ${C_YELLOW}ls /etc/udev/rules.d/99-ant-usb.rules${C_RESET}  # Should show 'No such file'"
    echo -e "  ${C_YELLOW}groups | grep plugdev${C_RESET}       # Should NOT show plugdev"
    echo
    warn "IMPORTANT: Log out and log back in (or reboot) for group changes to take effect."
    warn "After logging back in, run: ${C_YELLOW}su - \$USER${C_RESET} to verify group removal."
    echo
    info "You can now proceed with a fresh installation test following the Test Plan."
    echo
}

# ---
# Run the main function
# ---
main "$@"