'use strict';
'require baseclass';
'require fs';
'require rpc';
'require uci';

var callGetUnixtime = rpc.declare({
	object: 'luci',
	method: 'getUnixtime',
	expect: { result: 0 }
});

var callLuciVersion = rpc.declare({
	object: 'luci',
	method: 'getVersion'
});

var callSystemBoard = rpc.declare({
	object: 'system',
	method: 'board'
});

var callSystemInfo = rpc.declare({
	object: 'system',
	method: 'info'
});

var callSecurityInfo = rpc.declare({
	object: 'system.security',
	method: 'list'
});

var callSysInfo = rpc.declare({
	object: 'system.info',
	method: 'list'
});

function parse_uptime(value) {

	var val = parseInt(value) || 0,
		days = 0, hours = 0, minutes = 0;

	if (val < 3600) {
		if (val >= 60) {
			minutes = Math.floor(val / 60);
			val = val - (minutes * 60);
		}

		return minutes + ' ' + (minutes == 1 ? _('minute') : _('minutes')) + ' ' + val + ' ' + (val == 1 ? _('second') : _('seconds'));

	} else val = val < 60 ? 0 : Math.floor(val / 60);

	if (val >= 1440) {
		days = Math.floor(val / 1440);
		val = val - (days * 1440);
	}

	if (val >= 60) {
		hours = Math.floor(val / 60);
		val = val - (hours * 60);
	}

	var result = '';
	minutes = Math.floor(val);

	if (days > 0)
		result += days + ' ' + (days == 1 ? _('day') : _('days')) + ' ';

	if (days > 0 || hours > 0)
		result += hours + ' ' + (hours == 1 ? _('hour') : _('hours')) + (minutes > 0 ? ' ' : '');

	if (minutes > 0 || (hours == 0 && days == 0))
		result += minutes + ' ' + (minutes == 1 ? _('minute') : _('minutes'));

	return result;
}

function feature_element(title, state) {

	var state_style = state
		? 'background-color: #17b; color: #fff; width: 17px; background-position: 3px center; background-image: url("/luci-static/resources/svg/feature-enabled.svg");'
		: 'background-color: #888; color: #000; width: 16px; background-position: 2px center; background-image: url("/luci-static/resources/svg/feature-disabled.svg");';

	return E('li', { 'style': 'display: inline-block; margin-right: 6px;' }, [
		E('ul', { 'style': 'display: flex; flex-direction: row; flex-wrap: wrap; list-style-type: none; font-family: DejaVu Sans, Verdana, Geneva, sans-serif; font-size: 10px; margin: 0; padding: 0; overflow: hidden;' }, [
			E('li', { 'style': 'display: inline-block; height: 19px; border-radius: 4px 0 0 4px; background-color: #4c4c4c; padding: 0px 4px 0px 6px; color: #fff;' }, title),
			E('li', { 'style': 'display: inline-block; height: 19px; border-radius: 0 4px 4px 0; background-repeat: no-repeat; ' + state_style })
		])
	]);
}

return baseclass.extend({
	title: _('System'),

	load: function() {
		return Promise.all([
			L.resolveDefault(callSystemBoard(), {}),
			L.resolveDefault(callSystemInfo(), {}),
			L.resolveDefault(callLuciVersion(), { revision: _('unknown version'), branch: 'LuCI' }),
			L.resolveDefault(callGetUnixtime(), 0),
			L.resolveDefault(callSecurityInfo(), {}),
			L.resolveDefault(callSysInfo(), {}),
			uci.load('system')
		]);
	},

	render: function(data) {
		var boardinfo    = data[0],
		    systeminfo   = data[1],
		    luciversion  = data[2],
		    unixtime     = data[3],
		    securityinfo = data[4],
		    sysinfo      = data[5];

		luciversion = luciversion.branch + ' ' + luciversion.revision;

		var datestr = null;

		if (unixtime) {
			var date = new Date(unixtime * 1000),
				zn = uci.get('system', '@system[0]', 'zonename')?.replaceAll(' ', '_') || 'UTC',
				ts = uci.get('system', '@system[0]', 'clock_timestyle') || 0,
				hc = uci.get('system', '@system[0]', 'clock_hourcycle') || 0;

			datestr = new Intl.DateTimeFormat(undefined, {
				dateStyle: 'medium',
				timeStyle: (ts == 0) ? 'long' : 'full',
				hourCycle: (hc == 0) ? undefined : hc,
				timeZone: zn
			}).format(date);
		}

		var fields = [
			_('Hostname'),          boardinfo.hostname,
			_('Model'),             boardinfo.model || null,
			_('Architecture'),      boardinfo.system,
			_('Target Platform'),   (L.isObject(boardinfo.release) && boardinfo.release.target) ? boardinfo.release.target : null,
			_('Firmware'),          sysinfo.release_name || null,
			_('Firmware Version'),  (L.isObject(boardinfo.release) ? boardinfo.release.description + ' / ' : '') + (luciversion || ''),
			_('Security Features'), E('ul', { 'style': 'margin: 0; padding: 0; display: flex; flex-direction: row; flex-wrap: wrap;' }, [
				feature_element(_('SELinux'),  L.isObject(securityinfo.selinux)  && securityinfo.selinux.enabled),
				feature_element(_('AppArmor'), L.isObject(securityinfo.apparmor) && securityinfo.apparmor.enabled),
				feature_element(_('Seccomp'),  L.isObject(securityinfo.seccomp)  && securityinfo.seccomp.enabled)
			]),
			_('Kernel Version'),    boardinfo.kernel,
			_('Boot variant'),      sysinfo.boot_variant || null,
			_('Local Time'),        datestr,
			_('Uptime'),            parse_uptime(systeminfo.uptime)
		];

		var table = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {
			if (fields[i + 1] == null || fields[i + 1] === '')
				continue;

			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ fields[i + 1] ])
			]));
		}

		return table;
	}
});
