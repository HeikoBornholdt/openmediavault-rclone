openmediavault-rclone
=====================

1. You have to install and configure rclone by your own.
1. You have to create systemd services for rclone mounts by your own.

Example systemd service:
------------------------

```bash
[Unit]
Description=Google Drive Mount
AssertPathIsDirectory=/mnt/gdrive
After=network-online.target

[Service]
Type=notify
ExecStart=/usr/bin/rclone mount gdrive: /mnt/gdrive \
	--verbose \
	--config /root/.rclone.conf \
	--allow-other \
	--uid 1000 \
	--gid 100 \
	--umask=002 \
	--buffer-size 256M \
	--dir-cache-time 72h \
	--drive-chunk-size 32M \
	--timeout 1h \
	--vfs-read-chunk-size 128M \
	--vfs-read-chunk-size-limit off \
	--rc-addr=127.0.0.1:5572 \
	--rc
ExecStop=/bin/fusermount -uz /mnt/gdrive
Restart=always

[Install]
WantedBy=default.target
```
