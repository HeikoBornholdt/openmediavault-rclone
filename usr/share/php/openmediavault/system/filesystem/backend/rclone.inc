<?php

namespace OMV\System\Filesystem\Backend;

use OMV\Config\Database;
use OMV\Exception;
use OMV\System\Filesystem\RcloneImpl;
use SplFileObject;

class Rclone extends BackendAbstract
{
    /**
     * @var Database
     */
    protected $database;

    /**
     * @var string
     */
    protected $dataModelPath = 'conf.service.rclone.filesystem';

    /**
     * @var string
     */
    private $mountPointDataModelPath = 'conf.system.filesystem.mountpoint';

    public function __construct(Database $database) {
        $this->database = $database;
        $this->type = 'fuse.rclone';
        $this->properties = self::PROP_POSIX_ACL;
    }

    /**
     * Get a list of filesystems of the given filesystem backend.
     *
     * @return array
     */
    public function enumerate() {
        $filesystems = [];

        $objects = $this->database->get($this->dataModelPath);
        foreach ($objects as $object) {
            $filesystem = $this->getImpl($object->get('uuid'));

            $filesystems[$filesystem->getDeviceFile()] = [
                'devicefile' => $filesystem->getDeviceFile(),
                'uuid' => $filesystem->getUuid(),
                'label' => $filesystem->getLabel(),
                'type' => $filesystem->getType(),
            ];
        }

        return $filesystems;
    }

    /**
     * Check whether the filesystem implemented by this backend is identified by
     * the block device identification library. If this is not the case, then
     * the backend must override the enumerate method.
     *
     * @return bool
     */
    public function isBlkidEnumerated() {
        return false;
    }

    /**
     * Check whether the given filesystem identifier is represented by this
     * filesystem backend.
     *
     * @param string $id The filesystem identifier (e.g. UUID or device path)
     *
     * @return bool
     */
    public function isTypeOf($id) {
        throw Exception('Unimplemented method. Implement this in the derived class.');
    }

    /**
     * Does the filesystem have a device file? E.g. union mount or overlay
     * filesystems like Mergerfs don't have a device file.
     *
     * @return bool
     */
    public function hasDeviceFile() {
        throw Exception('Unimplemented method. Implement this in the derived class.');
    }

    public function getImpl($args) {
        $uuid = $args;
        
        if (!is_uuid($args)) {
            // Assume we got the mount directory.
            $mountPoint = $args;

            // Check if it's the real fsname and handle it (the real fsname
            // contains ':' to separate the branches).
            if (strpos($args, ':') !== false) {
                $mountPoint = self::fetchMountPointFromFstabByFsnameAndType($args, $this->type);
            }

            $objects = $this->database->get($this->mountPointDataModelPath);
            foreach ($objects as $object) {
                if ($object->get('dir') == $mountPoint) {
                    $uuid = $object->get('fsname');
                    break;
                }
            }
        }

        $object = $this->database->get($this->dataModelPath, $uuid);
        $mountPointObject = $this->database->get($this->mountPointDataModelPath, $object->get('self-mntentref'));

        return new RcloneImpl($object->get('uuid'), $mountPointObject->get('dir'), $object->get('name'), $this);
    }

    /**
     * Fetch the mount point from either '/etc/fstab' or '/proc/mounts'.
     *
     * @param string $fsname
     * @param string $type
     *
     * @return string
     *
     * @throws Exception
     */
    private static function fetchMountPointFromFstabByFsnameAndType($fsname, $type) {
        $mountsFile = strpos($fsname, DIRECTORY_SEPARATOR) === 0 ? '/etc/fstab' : '/proc/mounts';

        foreach (new SplFileObject($mountsFile) as $content) {
            $splitContent = preg_split('/\s+/', $content);

            if ($fsname === $splitContent[0] && strpos($splitContent[1], '/sharedfolders') === false && $type === $splitContent[2]) {
                return $splitContent[1];
            }
        }

        throw new Exception(sprintf(
            'Could not fetch a matching mount point from the provided fsname: \'%s\'.',
            $fsname
        ));
    }
}
