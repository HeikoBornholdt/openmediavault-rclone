// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/module/admin/storage/rclone/Mount.js")

Ext.define('OMV.module.admin.storage.rclone.Mounts', {
    extend: 'OMV.workspace.grid.Panel',
    requires: [
        'OMV.data.Store',
        'OMV.data.Model',
        'OMV.data.proxy.Rpc',
        'OMV.module.admin.storage.rclone.Mount'
    ],

    hidePagingToolbar: false,
    reloadOnActivate: true,

    columns: [{
        xtype: "textcolumn",
        header: _('UUID'),
        hidden: true,
        dataIndex: 'uuid'
    },{
        xtype: "textcolumn",
        header: _('Name'),
        flex: 1,
        sortable: true,
        dataIndex: 'name'
    }],

    store: Ext.create('OMV.data.Store', {
        autoLoad: true,
        model: OMV.data.Model.createImplicit({
            idProperty: 'uuid',
            fields: [{
                name: 'uuid',
                type: 'string'
            },{
                name: 'mounttype',
                type: 'string'
            },{
                name: 'name',
                type: 'string'
            },{
                name: 'server',
                type: 'string'
            },{
                name: 'sharename',
                type: 'string'
            }]
        }),
        proxy: {
            type: 'rpc',
            rpcData: {
                'service': 'Rclone',
                'method': 'getList'
            }
        },
        remoteSort: true,
        sorters: [{
            direction: 'ASC',
            property: 'name'
        }]
    }),

    onAddButton: function() {
        Ext.create('OMV.module.admin.storage.rclone.Mount', {
            title: _('Add mount'),
            uuid: OMV.UUID_UNDEFINED,
            listeners: {
                scope: this,
                submit: function() {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton: function() {
        var record = this.getSelected();

        Ext.create('OMV.module.admin.storage.rclone.Mount', {
            title: _('Edit mount'),
            uuid: record.get('uuid'),
            listeners: {
                scope: this,
                submit: function() {
                    this.doReload();
                    OMV.MessageBox.info(null, _('NOTE: The changes won\'t take effect until you\'ve restarted the system or manually remounted the mount.'));
                }
            }
        }).show();
    },

    doDeletion: function(record) {
        OMV.Rpc.request({
            scope: this,
            callback: this.onDeletion,
            rpcData: {
                service: 'Rclone',
                method: 'delete',
                params: {
                    uuid: record.get('uuid')
                }
            }
        });
    }
});

OMV.WorkspaceManager.registerPanel({
    id: 'mounts',
    path: '/storage/rclone',
    text: _('Mounts'),
    position: 30,
    className: 'OMV.module.admin.storage.rclone.Mounts'
});
