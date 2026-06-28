'use strict';
'require form';
'require network';

// LuCI protocol handler for the 'netbird' protocol - DRAFT for the netbird package
// (install as /www/luci-static/resources/protocol/netbird.js in luci-proto-netbird).
//
// The 'netbird' netifd proto runs the netbird daemon under netifd and lets it own
// the wt0 tunnel + its routes. This handler just exposes the interface in
// Network -> Interfaces. Virtual/floating: there is no base device to assign -
// netbird creates wt0 itself.
return network.registerProtocol('netbird', {
	getI18n: function() {
		return _('NetBird VPN');
	},

	getIfname: function() {
		return this._ubus('l3_device') || 'wt0';
	},

	getOpkgPackage: function() {
		return 'netbird';
	},

	isFloating: function() {
		return true;
	},

	isVirtual: function() {
		return true;
	},

	getDevices: function() {
		return null;
	},

	containsDevice: function(ifname) {
		return (network.getIfnameOf(ifname) == this.getIfname());
	},

	renderFormOptions: function(s) {
		var o;

		o = s.taboption('general', form.Value, 'device', _('Tunnel device'),
			_('The interface netbird creates and manages. Default: wt0.'));
		o.placeholder = 'wt0';
		o.rmempty = true;

		o = s.taboption('general', form.Value, 'searchdomain', _('Search domain'),
			_('DNS search domain for the NetBird network (e.g. netbird.cloud).'));
		o.rmempty = true;

		o = s.taboption('general', form.Value, 'delay', _('Startup wait (s)'),
			_('Maximum seconds to wait for the tunnel after the daemon (re)starts.'));
		o.datatype = 'uinteger';
		o.placeholder = '25';
		o.rmempty = true;

		o = s.taboption('advanced', form.Value, 'state_dir', _('State directory'),
			_('netbird state directory (NB_STATE_DIR). Default: /root/.config/netbird.'));
		o.placeholder = '/root/.config/netbird';
		o.rmempty = true;
	}
});
