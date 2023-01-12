'use strict';
'require ui';
'require uci';
'require rpc';
'require form';
'require network';

network.registerPatternVirtual(/^zt.+$/);

return network.registerProtocol('zerotier', {
	getI18n: function () {
		return _('Zerotier');
	},

	getIfname: function () {
		return this._ubus('interface') || this.sid;
	},

	getOpkgPackage: function () {
		return 'zerotier';
	},

	isFloating: function () {
		return true;
	},

	isVirtual: function () {
		return true;
	},

	getDevices: function () {
		return null;
	},

	containsDevice: function (ifname) {
		return (network.getIfnameOf(ifname) == this.getIfname());
	},

	renderFormOptions: function (s) {
		var o;

		o = s.taboption('general', form.Value, 'network_id', _('Network id'), _('Required. Network id where to join.'));
		o.optional = false
		o.rmempty = false
		o = s.taboption('general', form.Value, 'secret', _('Secret'), _('Your network identifier.'));
		o.optional = true
		o = s.taboption('general', form.Value, 'port', _('Port'), _('Port for UDP and TCP/HTTP (default: 9993, leave empty for random)'));
		o.optional = true
		o = s.taboption('general', form.Value, 'config_path', _('Config path'), _('Path to your prepared connection configuration.'));
		o.optional = true
		o = s.taboption('general', form.Flag, 'copy_config_path', _('Copy config files'), _('Instead of linking to configuration path, copy it'));
                o.default = o.disabled;
		o.optional = true
	},

	deleteConfiguration: function () {
		uci.sections('network', 'zt%s'.format(this.sid), function (s) {
			uci.remove('network', s['.name']);
		});
	}
});
