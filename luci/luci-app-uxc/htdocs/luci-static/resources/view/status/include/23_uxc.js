'use strict';
'require baseclass';
'require rpc';
'require ui';

var callContainerList = rpc.declare({
	object: 'container',
	method: 'list',
	expect: { '': {} }
});

return baseclass.extend({
	title: _('Containers'),

	load() {
		return L.resolveDefault(callContainerList(), {});
	},

	render: function(containers) {

		var table = E('table', { 'class': 'table', 'style': 'width=100%' });

		if (!L.isObject(containers))
			return E('div', { 'class': 'left', 'style': 'margin-bottom: 16px;' }, _('Error while retrieving list of containers'));

		table.appendChild(E('tr', { 'class': 'tr' }, [
			E('td',  { 'class': 'td left', 'style' : 'width: 20%' }, '<b>' + _('Name') + '</b>'),
			E('td',  { 'class': 'td left', 'style': 'width: 20%;' }, '<b>' + _('Bundle') + '</b>'),
			E('td',  { 'class': 'td center', 'style': 'width: 10;' }, '<b>' + _('PID') + '</b>'),
			E('td',  { 'class': 'td left', 'style': 'width: 100%;' }, '&nbsp;'),
			E('td',  { 'class': 'td left', 'style': 'width: 15%;'  }, '<b>' + _('Running') + '</b>')
		]));

		var cnt = 0;

		Object.keys(containers).forEach(function(key) {

			let _key = key;
			if (L.isObject(containers[_key]['instances'])) {

				var shown = false;
				Object.keys(containers[_key]['instances']).forEach(function(instance) {

					let _instance = instance;

					if (instance != 'netifd' && instance != 'ubus' && !shown) {

						let container = containers[_key]['instances'][_instance];

						table.appendChild(E('tr', { 'class': 'tr' }, [
							E('td', { 'class': 'td left', 'style': 'width: 20%;'}, key),
							E('td', { 'class': 'td left', 'style': 'width: 20%;'}, container['bundle']),
							E('td', { 'class': 'td center', 'style': 'width: 10%;'}, container['pid']),
							E('td',  { 'class': 'td left', 'style': 'width: 100%;' }, '&nbsp;'),
							E('td', { 'class': 'td center', 'style': 'width: 15%;'}, container['running']),
						]));

						shown = true;
						cnt = cnt + 1;
					}
				});
			}
		});

		if ( cnt != 0 )
			return table;

		return E('div', { 'class': 'left', 'style': 'margin-bottom: 16px;' }, _('System does not have containers'));
	},

});
