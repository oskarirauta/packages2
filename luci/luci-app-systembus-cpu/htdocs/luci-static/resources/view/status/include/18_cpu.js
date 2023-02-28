'use strict';
'require baseclass';
'require rpc';

var callCpuInfo = rpc.declare({
	object: 'system.cpu',
	method: 'list'
});

var callCpuTemp = rpc.declare({
	object: 'system.cpu',
	method: 'temp'
});

var callInfo = rpc.declare({
	object: 'system.info',
	method: 'list'
});

function cpu_progressbar(value) {
	var vn = parseInt(value) || 0;

	return E('div', {
		'class': 'cbi-progressbar',
		'title': '%s%'.format(vn)
	}, E('div', { 'style': 'width:%.2f%%'.format(vn) }));
}

function cpu_temperaturebar(value) {
        var vn = parseInt(value) || 0;

        return E('div', {
                'class': 'cbi-progressbar',
                'title': '+%s C'.format(vn)
        }, E('div', { 'style': 'width:%.2f%%'.format(vn) }));
}

return baseclass.extend({
	title: _('CPU Load'),

	load: function() {
		return Promise.all([
			L.resolveDefault(callCpuInfo(), {}),
			L.resolveDefault(callCpuTemp(), {}),
			L.resolveDefault(callInfo(), {})
		]);
	},

	render: function(data) {

		var cpuinfo	= data[0],
		    cputemp     = data[1],
		    systeminfo2	= data[2];

		if ( !systeminfo2.systembus_loaded )
			return E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '100%' }, [
						_('Cannot retrieve cpu data. Systembus is not loaded.')
					])
				])]);

		var fields = [
			_('Temperature'), cputemp.temperature,
			_('Total'), ( cpuinfo.cpu != null && cpuinfo.cpu1 != null ) ? cpuinfo.cpu : null,
			'cpu0', cpuinfo.cpu0,
			'cpu1', cpuinfo.cpu1,
			'cpu2', cpuinfo.cpu2,
			'cpu3', cpuinfo.cpu3,
			'cpu4', cpuinfo.cpu4,
			'cpu5', cpuinfo.cpu5,
			'cpu6', cpuinfo.cpu6,
			'cpu7', cpuinfo.cpu7,
			'cpu8', cpuinfo.cpu8,
			'cpu9', cpuinfo.cpu9,
			'cpu10', cpuinfo.cpu10,
			'cpu11', cpuinfo.cpu11,
			'cpu12', cpuinfo.cpu12,
			'cpu13', cpuinfo.cpu13,
			'cpu14', cpuinfo.cpu14,
			'cpu15', cpuinfo.cpu15
		];

		var table = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {

			if ( fields[i + 1 ] == null ) continue;

			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ fields[i] == _('Temperature') ? cpu_temperaturebar(fields[i + 1]) : cpu_progressbar(fields[i + 1]) ])
			]));
		}

		return table;
	}

});
