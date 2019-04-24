// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/form/field/plugin/FieldInfo.js")

Ext.define('OMV.module.admin.storage.rclone.Mount', {
    extend: 'OMV.workspace.window.Form',
    requires: [
        'OMV.form.field.plugin.FieldInfo',
        'OMV.workspace.window.plugin.ConfigObject'
    ],

    plugins: [{
        ptype: 'configobject'
    }],

    hideResetButton: true,

    rpcService: 'Rclone',
    rpcGetMethod: 'get',
    rpcSetMethod: 'set',

    getFormItems: function() {
        return [{
            xtype: 'textfield',
            name: 'name',
            fieldLabel: _('Name'),
            allowBlank: false,
            readOnly: this.uuid !== OMV.UUID_UNDEFINED,
            plugins: [{
                ptype: 'fieldinfo',
                text: _('Name of systemd service that will do the rclone mount. Must be created manually before.')
            }]
        }, {
            xtype: 'hiddenfield',
            name: 'self-mntentref',
            value: OMV.UUID_UNDEFINED
        }];
    }
});
