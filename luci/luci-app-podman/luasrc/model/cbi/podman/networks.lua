--[[
LuCI - Lua Configuration Interface
Copyright 2021 Oskari Rauta <https://github.com/oskarirauta/lucipodman>
]]--

local podman = require "luci.model.podman"

local m, s, o
local networks, dk, res

dk = podman.new()
res = dk.networks:list()
if res.code < 300 then
	networks = res.body
else
	return
end

local get_networks = function ()
	local data = {}

	if type(networks) ~= "table" then
		return nil
	end

	for i, v in ipairs(networks) do
		local index = v.Created .. v.Id

		data[index]={}
		data[index]["_selected"] = 0
		data[index]["_id"] = v.Id:sub(1,12)
		data[index]["_name"] = v.Name
		data[index]["_driver"] = v.Driver

		if v.Driver == "bridge" then
			data[index]["_interface"] = v.Options["com.docker.network.bridge.name"]
		elseif v.Driver == "macvlan" then
			data[index]["_interface"] = v.Options.parent
		end

		data[index]["_subnet"] = v.IPAM and v.IPAM.Config[1] and v.IPAM.Config[1].Subnet or nil
		data[index]["_gateway"] = v.IPAM and v.IPAM.Config[1] and v.IPAM.Config[1].Gateway or nil
	end

	return data
end

local network_list = get_networks()

m = SimpleForm("podman", translate("Podman"))
m.submit=false
m.reset=false

s = m:section(Table, network_list, translate("Networks"))
s.nodescr=true

o = s:option(Flag, "_selected","")
o.template = "podman/cbi/xfvalue"
o.disabled = 0
o.enabled = 1
o.default = 0
o.render = function(self, section, scope)
	self.disable = 0
	if network_list[section]["_name"] == "bridge" or network_list[section]["_name"] == "none" or network_list[section]["_name"] == "host" then
		self.disable = 1
	end
	Flag.render(self, section, scope)
end
o.write = function(self, section, value)
	network_list[section]._selected = value
end

o = s:option(DummyValue, "_id", translate("ID"))

o = s:option(DummyValue, "_name", translate("Network Name"))

o = s:option(DummyValue, "_driver", translate("Driver"))

o = s:option(DummyValue, "_interface", translate("Parent Interface"))

o = s:option(DummyValue, "_subnet", translate("Subnet"))

o = s:option(DummyValue, "_gateway", translate("Gateway"))

s = m:section(SimpleSection)
s.template = "podman/apply_widget"
s.err = podman:read_status()
s.err = s.err and s.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if s.err then
	podman:clear_status()
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template="cbi/nullsection"

o = s:option(Button, "_new")
o.inputtitle= translate("New")
o.template = "podman/cbi/inlinebutton"
o.notitle=true
o.inputstyle = "add"
o.forcewrite = true
o.write = function(self, section)
	luci.http.redirect(luci.dispatcher.build_url("admin/podman/newnetwork"))
end

o = s:option(Button, "_remove")
o.inputtitle= translate("Remove")
o.template = "podman/cbi/inlinebutton"
o.inputstyle = "remove"
o.forcewrite = true
o.write = function(self, section)
	local network_selected = {}
	local network_name_selected = {}
	local network_driver_selected = {}

	for k in pairs(network_list) do
		if network_list[k]._selected == 1 then
			network_selected[#network_selected + 1] = network_list[k]._id
			network_name_selected[#network_name_selected + 1] = network_list[k]._name
			network_driver_selected[#network_driver_selected + 1] = network_list[k]._driver
		end
	end

	if next(network_selected) ~= nil then
		local success = true
		podman:clear_status()

		for ii, net in ipairs(network_selected) do
			podman:append_status("Networks: " .. "remove" .. " " .. net .. "...")
			local res = dk.networks["remove"](dk, {id = net})

			if res and res.code >= 300 then
				podman:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
				success = false
			else
				podman:append_status("done\n")
				--[[
				if network_driver_selected[ii] == "macvlan" then
					docker.remove_macvlan_interface(network_name_selected[ii])
				end
				]]--
			end
		end

		if success then
			docker:clear_status()
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/docker/networks"))
	end
end

return m
