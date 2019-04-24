<?php

namespace OMV\Engined\Rpc;

use Exception;
use OMV\Config\ConfigObject;
use OMV\Config\Database;
use OMV\Engine\Module\Manager as ModuleManager;
use OMV\Rpc\Rpc;
use OMV\Rpc\ServiceAbstract;
use OMV\System\Filesystem\Filesystem;
use OMV\System\MountPoint;
use OMV\Uuid;

class Rclone extends ServiceAbstract
{
    /**
     * @var Database
     */
    private $database;

    /**
     * @var string
     */
    private $dataModelPath = 'conf.service.rclone.filesystem';

    public function __construct(Database $database = null) {
        $this->database = $database ?: Database::getInstance();
    }

    /**
     * Get the name of the RPC service.
     *
     * @return string
     */
    public function getName() {
        return 'Rclone';
    }

    /**
     * Initialize the RPC service.
     *
     * @return void
     */
    public function initialize() {
        $this->registerMethod('getList');
        $this->registerMethod('get');
        $this->registerMethod('set');
        $this->registerMethod('delete');
    }

    /**
     * Get a list of the existing union filesystems.
     *
     * @param $params  The method parameters
     * @param $context The context of the caller
     *
     * @return array
     */
    public function getList($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.common.getlist');

        $filesystems = array_map(function ($object) use ($context) {
            return $object->getAssoc();
        }, $this->database->get($this->dataModelPath));

        return $this->applyFilter(
            $filesystems,
            $params['start'],
            $params['limit'],
            $params['sortfield'],
            $params['sortdir']
        );
    }

    /**
     * Get the configuration for a single existing union filesystem.
     *
     * @param $params  The method parameters
     * @param $context The context of the caller
     *
     * @return array
     */
    public function get($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.common.objectuuid');

        $object = $this->database->get($this->dataModelPath, $params['uuid']);

        return $object->getAssoc();
    }

    /**
     * Set the configuration for a single new or existing union filesystem.
     *
     * @param $params  The method parameters
     * @param $context The context of the caller
     *
     * @return array
     */
    public function set($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.rclone.set');

        $object = new ConfigObject($this->dataModelPath);
        $object->setAssoc($params);

        $isNewObject = $object->isNew();

        if ($isNewObject) {
            $this->database->assertIsUnique($object, 'name');
        }

        // Save the object before generating a fstab entry to generate a valid
        // UUID.
        $this->database->set($object);

        // If it's a new entry we'll create a new mntent.
        if ($isNewObject) {
            $selfMntent = $this->createMntentEntry($object);

            try {
                $object->set('self-mntentref', $selfMntent['uuid']);

                $this->database->set($object);
            } catch (Exception $e) {
                $this->database->delete($object);

                throw $e;
            }
        }

        // The fstab module does not mark itself as dirty when a mntent
        // configuration is modified, only when created or deleted.
        if (!$isNewObject) {
            $moduleManager = ModuleManager::getInstance();
            $moduleManager->setModuleDirty('fstab');
        }

        return $object->getAssoc();
    }

    /**
     * Delete the configuration of a single existing union filesystem.
     *
     * @param $params  The method parameters
     * @param $context The context of the caller
     *
     * @return void
     */
    public function delete($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.common.objectuuid');

        // FIXME: Check that the filesystem is not used before deleting it.
        $object = $this->database->get($this->dataModelPath, $params['uuid']);
        $this->database->delete($object);

        Rpc::call('FsTab', 'delete', ['uuid' => $object->get('self-mntentref')], $context);
    }

    /**
     * Create a mntent configuration object from a configuration object.
     *
     * NOTE: Currently this should only be used when creating a new file system
     * as it will calculate the mount point every time. This can in turn break
     * existing setups if the default mount point directory has changed.
     *
     * @param ConfigObject $object The object to create a mntent entry for
     *
     * @return string Returns the mntentref for the mntent entry
     */
    private function createMntentEntry(ConfigObject $object) {
        $mountPoint = '/tank2/mnt/' . $object->get('name');

        $mntentEntry = [
            'uuid' => $object->get('self-mntentref'),
            'fsname' => $object->get('uuid'),
            'dir' => $mountPoint,
            'type' => 'fuse.rclone',
            'opts' => '',
            'freq' => 0,
            'passno' => 0,
        ];

        $mntent = Rpc::call('FsTab', 'set', $mntentEntry, $this->getAdminContext());

        return $mntent;
    }
}
