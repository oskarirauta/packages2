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

		var cpuinfo    = data[0],
		    cputemp    = data[1],
		    systeminfo = data[2];

		if (!systeminfo.systembus_loaded)
			return E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '100%' }, [
						_('Cannot retrieve cpu data. Systembus is not loaded.')
					])
				])]);

		var fields = [
			_('Temperature'), { value: cputemp.temperature, isTemp: true },
			_('Total'), ( cpuinfo.cpu != null && cpuinfo.cpu1 != null ) ? { value: cpuinfo.cpu } : null,
			'cpu0',  cpuinfo.cpu0  != null ? { value: cpuinfo.cpu0  } : null,
			'cpu1',  cpuinfo.cpu1  != null ? { value: cpuinfo.cpu1  } : null,
			'cpu2',  cpuinfo.cpu2  != null ? { value: cpuinfo.cpu2  } : null,
			'cpu3',  cpuinfo.cpu3  != null ? { value: cpuinfo.cpu3  } : null,
			'cpu4',  cpuinfo.cpu4  != null ? { value: cpuinfo.cpu4  } : null,
			'cpu5',  cpuinfo.cpu5  != null ? { value: cpuinfo.cpu5  } : null,
			'cpu6',  cpuinfo.cpu6  != null ? { value: cpuinfo.cpu6  } : null,
			'cpu7',  cpuinfo.cpu7  != null ? { value: cpuinfo.cpu7  } : null,
			'cpu8',  cpuinfo.cpu8  != null ? { value: cpuinfo.cpu8  } : null,
			'cpu9',  cpuinfo.cpu9  != null ? { value: cpuinfo.cpu9  } : null,
			'cpu10', cpuinfo.cpu10 != null ? { value: cpuinfo.cpu10 } : null,
			'cpu11', cpuinfo.cpu11 != null ? { value: cpuinfo.cpu11 } : null,
			'cpu12', cpuinfo.cpu12 != null ? { value: cpuinfo.cpu12 } : null,
			'cpu13', cpuinfo.cpu13 != null ? { value: cpuinfo.cpu13 } : null,
			'cpu14', cpuinfo.cpu14 != null ? { value: cpuinfo.cpu14 } : null,
			'cpu15', cpuinfo.cpu15 != null ? { value: cpuinfo.cpu15 } : null
		];

		var table = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {

			if (fields[i + 1] == null) continue;

			var entry = fields[i + 1];

			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td left', 'width': '33%' }, [ fields[i] ]),
				E('td', { 'class': 'td left' }, [ entry.isTemp ? cpu_temperaturebar(entry.value) : cpu_progressbar(entry.value) ])
			]));
		}

		return table;
	}

});
