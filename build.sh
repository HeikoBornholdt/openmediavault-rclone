#!/bin/bash
docker run --rm --tty --interactive \
	--volume "`pwd`":/openmediavault-rclone \
	--volume /Users/heiko/Downloads:/output \
	--workdir /openmediavault-rclone \
	ubuntu:bionic \
	/bin/bash -c 'apt-get update && \
		apt-get install -y dpkg-dev debhelper && \
		find . -name ".DS_Store" -exec rm {} \; && \
		dpkg-buildpackage -us -uc && \
		mv /openmediavault-rclone_* /output'
