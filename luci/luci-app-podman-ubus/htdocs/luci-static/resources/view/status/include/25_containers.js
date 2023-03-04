'use strict';
'require baseclass';
'require rpc';
'require ui';

var callPodmanList = rpc.declare({
	object: 'podman',
	method: 'list'
});

var callPodmanExec = rpc.declare({
	object: 'podman',
	method: 'exec',
	params: [ 'action', 'group', 'name' ],
	expect: { result: [] }
});

function progressbar(percent, text) {
	return E('div', {
		'class': 'cbi-progressbar',
		'title': text
	}, E('div', { 'style': 'width:%.2f%%'.format(percent > 100 ? 100 : percent) }));
}

return baseclass.extend({
	title: _('Containers'),

	load: function() {
		return L.resolveDefault(callPodmanList(), {});
	},

	callContainerExec: function(action, name) {
		return L.resolveDefault(callPodmanExec(action, 'container', name));
	},

	renderContainers: function(data) {

		var pods = Array.isArray(data.pods) ? data.pods : [],
		    showInfra = false;

		if ( pods.length == 0 )
			return E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '100%' }, [
						_('This system does not have containers or ubus container management is offline.')
					])
				])
			]);

		var table = E('table', { 'class': 'table containers' }),
			topRowStyle = 'padding-bottom: 2px;',
			rowStyle = 'padding-top: 2px; padding-bottom: 2px; border-top: 0;',
			lastRowStyle = 'padding-top: 2px; border-top: 0;';

		for (var podI = 0; podI < pods.length; podI++) {

			var pod = pods[podI];

			if ( !( pods.length === 1 && pod.name === '' ))
				table.appendChild(E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'th', 'colspan': '6' }, [
						pod.name === '' && pods.length > 1 ? 'Podless' : pod.name
					])
				]));

			var infraRow1 = null;
			var infraRow2 = null;
			var infraRow3 = null;

			for (var containerI = 0; containerI < pod.containers.length; containerI++) {

				var container = pod.containers[containerI];
				var containerRow = E('tr', { 'class': 'tr' });

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': 'padding-left: 20px; ' + topRowStyle }, [
					E('b', { 'class': 'container-name' }, [ container.name ])
				]));

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': 'padding-left: 20px; ' + topRowStyle }, [
					container.infra ? _('INFRA') : (
						container.busy.state ?
						_(container.busy.reason.charAt(0).toUpperCase() + container.busy.reason.slice(1)) :
						_(container.state.charAt(0).toUpperCase() + container.state.slice(1))
					)
				]));

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': topRowStyle }, [
					container.running && !container.busy.state ? _('Uptime') : ''
				]));

				containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
					container.running && !container.busy.state ? ( _('RAM') + ":" ) : ''
				]));

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': topRowStyle }, [
					container.running && !container.busy.state ? progressbar(container.ram.percent, container.ram.used + " / " + container.ram.max + " (" + container.ram.percent + "%)") :
						''
				]));

				var restart_added = false;

				if ( container.infra )
					containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
						''
					]));
				else {
					if ( container.busy.state )
						containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
							E('div', {
								'class': 'spinning left',
								'style': 'display: inline;'
							}, _('Busy'))
						]));
					else if ( container.actions.start )
						containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
							E('button', {
								'class': 'btn cbi-button cbi-button-apply important',
								'click': ui.createHandlerFn(this, 'callContainerExec', 'start', container.name)
							}, _('Start'))
						]));
					else if ( container.actions.stop )
						containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
							E('button', {
								'class': 'btn cbi-button cbi-button-apply important',
								'click': ui.createHandlerFn(this, 'callContainerExec', 'stop', container.name)
							}, _('Stop'))
						]));
					else if ( container.actions.restart ) {
						containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
							E('button', {
								'class': 'btn cbi-button cbi-button-apply important',
								'click': ui.createHandlerFn(this, 'callContainerExec', 'restart', container.name)
							}, _('Restart'))
						]));
						restart_added = true;
					} else containerRow.appendChild(E('td', { 'class': 'td right', 'style': topRowStyle }, [
						'---'
					]));
				}

				if ( !container.infra ) table.appendChild(containerRow);
				else infraRow1 = containerRow;

				containerRow = E('tr', { 'class': 'tr' });

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': 'padding-left: 20px; ' + rowStyle, 'colspan': '2' }, [
					E('i', { 'class': 'container-image' }, [ container.image ])
				]));

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': rowStyle }, [
					container.running && !container.busy.state ? ( container.uptime.days + 'd ' + container.uptime.hours + 'h ' + container.uptime.minutes + 'm' ) :
						''
				]));

				containerRow.appendChild(E('td', { 'class': 'td right', 'style': rowStyle }, [
					container.running && !container.busy.state ? ( _('CPU') + ":" ) : ''
				]));

				containerRow.appendChild(E('td', { 'class': 'td left', 'style': rowStyle }, [
					container.running && !container.busy.state ? progressbar(container.cpu.percent, container.cpu.load) : ''
				]));

				if ( container.infra )
					containerRow.appendChild(E('td', { 'class': 'td right', 'style': rowStyle }, [
						''
					]));
				else if ( container.actions.restart && restart_added === false )
					containerRow.appendChild(E('td', { 'class': 'td right', 'style': rowStyle }, [
						E('button', {
							'class': 'btn cbi-button cbi-button-apply important',
							'click': ui.createHandlerFn(this, 'callContainerExec', 'restart', container.name)
						}, _('Restart'))
					]));
				else containerRow.appendChild(E('td', { 'class': 'td right', 'style': rowStyle }, [
						''
					]));

				if ( container.infra ) {
					infraRow2 = containerRow;
					infraRow3 = E('td', { 'class': 'td left', 'style': 'padding-left: 20px; ' + lastRowStyle, 'colspan': '6' }, [ '' ]);
					continue;
				}

				table.appendChild(containerRow);

				containerRow = E('tr', { 'class': 'tr' });
				containerRow.appendChild(E('td', { 'class': 'td left', 'style': 'padding-left: 20px; ' + lastRowStyle, 'colspan': '6' }, [
					container.cmd
				]));

				table.appendChild(containerRow);

			}

			if ( showInfra ) {
				if ( infraRow1 )
					table.appendChild(infraRow1);
				if ( infraRow2 )
					table.appendChild(infraRow2);
				if ( infraRow3 )
					table.appendChild(infraRow3);
			}

		}

		return table;
	},

	render: function(data) {
		return this.renderContainers(data);
	}
});
