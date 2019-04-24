<?php

use OMV\Config\Database;
use OMV\System\Filesystem\Backend\Manager;
use OMV\System\Filesystem\Backend\Rclone;

$database = Database::getInstance();
$filesystemBackendManager = Manager::getInstance();
$filesystemBackendManager->registerBackend(new Rclone($database));
