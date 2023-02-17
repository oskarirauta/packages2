'use strict';
'require ui';
'require uci';
'require rpc';
'require form';
'require network';

network.registerPatternVirtual(/^cni-.+$/);

return network.registerProtocol('cni', {
	getI18n: function () {
		return _('CNI');
	},

	getIfname: function () {
		return this._ubus('interface') || this.sid;
	},

	getOpkgPackage: function () {
		return 'cni-plugins';
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

		o = s.taboption('general', form.Value, 'device', _('dev'), _('Required. CNI device.'));
		o.optional = false;
		o.rmempty = false;
	},

	deleteConfiguration: function () {
		uci.sections('network', 'cni-%s'.format(this.sid), function (s) {
			uci.remove('network', s['.name']);
		});
	}
});
