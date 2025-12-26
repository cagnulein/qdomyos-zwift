#!/bin/bash
set -euo pipefail
bin=$(command -v qdomyos-zwift || echo '/usr/bin/qdomyos-zwift')
user=${SUDO_USER:-$USER}
flags='-no-gui -no-log -no-console -poll-device-time 200'
tmp=$(mktemp /tmp/qz.service.XXXX)
cat > "$tmp" <<EOF
[Unit]
Description=qdomyos-zwift service
After=multi-user.target

[Service]
User=root
Group=plugdev
Environment="QZ_USER=${user}"
WorkingDirectory=$(dirname "$bin")
ExecStart=${bin} ${flags}
KillSignal=SIGINT

[Install]
WantedBy=multi-user.target
EOF

echo 'TEMP_SERVICE_FILE:' "$tmp"
ls -l "$tmp"
# Install
sudo mv -f "$tmp" /etc/systemd/system/qz.service
sudo systemctl daemon-reload
echo 'INSTALL_OK'
# Start
sudo systemctl start qz.service || true
sleep 1
sudo systemctl status qz.service --no-pager || true
# Journal
sudo journalctl -u qz.service -n 20 --no-pager || true
# Stop
sudo systemctl stop qz.service || true
sleep 1
sudo systemctl status qz.service --no-pager || true
# Enable
sudo systemctl enable qz.service || true
sudo systemctl is-enabled qz.service || true
# Disable
sudo systemctl disable qz.service || true
sudo systemctl is-enabled qz.service || echo 'DISABLED'
