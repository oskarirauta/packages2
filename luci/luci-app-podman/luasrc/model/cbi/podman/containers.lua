--[[
LuCI - Lua Configuration Interface
Copyright 2021 Oskari Rauta <https://github.com/oskarirauta/lucipodman>
]]--

local http = require "luci.http"
local podman = require "luci.model.podman"

local m, s, o
local pods, images, networks, containers, res

local dk = podman.new()
res = dk.images:list()
if res.code <300 then
	images = res.body
else
	return
end

res = dk.networks:list()
if res.code <300 then
	networks = res.body
else
	return
end

res = dk.pods:list()
if res.code <300 then
	pods = res.body
else
	return
end

res = dk.containers:list({
	query = {
		all=true
	}
})
if res.code <300 then
	containers = res.body
else
	return
end

local urlencode = luci.http.protocol and luci.http.protocol.urlencode or luci.util.urlencode

function get_containers()
	local data = {}

	if type(containers) ~= "table" then
		return nil
	end

	for i, v in ipairs(containers) do
		local index = v.Id

		data[index]={}
		data[index]["_selected"] = 0
		data[index]["_id"] = v.Id:sub(1,12)
		data[index]["_name"] = v.Names[1]
		data[index]["_status"] = v.Status
		data[index]["_state"] = v.State

		if v.Status:find("^Up") then
			data[index]["_status"] = '<font color="green">'.. data[index]["_status"] .. "</font>"
		elseif v.Status == "" then
			data[index]["_status"] = '<font color="red">' .. data[index]["_state"] .. '</font>'
		else
			data[index]["_status"] = '<font color="red">'.. data[index]["_status"] .. "</font>"
		end

		data[index]["_network"] = ""
		if type(v.Networks) == "table" then
			for _, networkname in ipairs(v.Networks) do
				data[index]["_network"] = (data[index]["_network"] ~= "" and ( data[index]["_network"] .. ", " ) or "" ) .. networkname
			end
		end

		data[index]["_ports"] = ""
		if v.Ports and next(v.Ports) ~= nil then
			data[index]["_ports"] = nil
			for _,v2 in ipairs(v.Ports) do
				data[index]["_ports"] = (data[index]["_ports"] ~= "" and (data[index]["_ports"] .. ", ") or "")
					.. ((v2.PublicPort and v2.Type and v2.Type == "tcp") and ('<a href="javascript:void(0);" onclick="window.open((window.location.origin.match(/^(.+):\\d+$/) && window.location.origin.match(/^(.+):\\d+$/)[1] || window.location.origin) + \':\' + '.. v2.PublicPort ..', \'_blank\');">') or "")
					.. (v2.PublicPort and (v2.PublicPort .. ":") or "")  .. (v2.PrivatePort and (v2.PrivatePort .."/") or "") .. (v2.Type and v2.Type or "")
					.. ((v2.PublicPort and v2.Type and v2.Type == "tcp")and "</a>" or "")
			end
		end

		data[index]["_command"] = ""
		if v.Command and type(v.Command) == "table" then
			for _, cmd in ipairs(v.Command) do
				data[index]["_command"] = (data[index]["_command"] ~= "" and (data[index]["_command"] .. " ") or "") .. cmd
			end
		elseif v.Command then
			data[index]["_command"] = v.Command
		end

		data[index]["_image"] = v.Image
		data[index]["_image_id"] = v.ImageID
		data[index]["_pod"] = v.PodName and v.PodName or ""
		data[index]["_pod_id"] = v.Pod and v.Pod:sub(8,20) or ""
		data[index]["_image_id"] = v.ImageID:sub(8,20)
		data[index]["_infra"] = v.IsInfra and v.IsInfra or false

		data[index]["__container"] = '<span style="white-space: nowrap;">' .. data[index]["_name"] .. '<br/>' .. data[index]["_id"] .. '</span>';
		data[index]["__image"] = '<span style="white-space: nowrap;">' .. data[index]["_image"] .. '<br/>' .. data[index]["_image_id"] .. '</span>'
		data[index]["__pod"] = '<span style="white-space: nowrap;">' .. data[index]["_pod"] .. '<br/>&nbsp;</span>'
		data[index]["__network"] = '<span style="white-space: nowrap;">' .. data[index]["_network"] .. '<br/>&nbsp;</span>'
		data[index]["__ports"] = '<span style="white-space: nowrap;">' .. (data[index]["_ports"] == "" and "none" or data[index]["_ports"]) .. '<br/>&nbsp;</span>'
		data[index]["__infra"] = '<span style="white-space: nowrap;">' .. (v.IsInfra and "true" or "false") .. '<br/>&nbsp;</span>'
		data[index]["__command"] = '<span style="white-space: nowrap;">' .. data[index]["_command"] .. '<br/>&nbsp;</span>'
		data[index]["__status"] = '<span style="white-space: nowrap;">' .. data[index]["_status"] .. '<br/>&nbsp;</span'
		
	end

	return data
end

local container_list = get_containers()

m = SimpleForm("podman", translate("Podman"))
m.submit=false
m.reset=false

s = m:section(SimpleSection)
s.template = "podman/apply_widget"
s.err=podman:read_status()
s.err=s.err and s.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if s.err then
	podman:clear_status()
end

s = m:section(Table, container_list, translate("Containers"))
s.addremove = false
s.sectionhead = translate("Containers")
s.sortable = false
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin", "podman", "container","%s")

o = s:option(Flag, "_selected","")
o.disabled = 0
o.enabled = 1
o.default = 0
o.write=function(self, section, value)
	container_list[section]._selected = value
end

o = s:option(DummyValue, "__container", translate("Container"))
o.width="20%"
o.rawhtml = true

o = s:option(DummyValue, "__image", translate("Image"))
o.rawhtml = true

o = s:option(DummyValue, "__pod", translate("Pod"))
o.rawhtml = true

o = s:option(DummyValue, "__network", translate("Network"))
o.rawhtml = true

o = s:option(DummyValue, "__ports", translate("Ports"))
o.width="10%"
o.rawhtml = true

o = s:option(DummyValue, "__infra", translate("Infra"))
o.width="5%"
o.rawhtml = true

o = s:option(DummyValue, "__command", translate("Command"))
o.width="20%"
o.rawhtml = true

o = s:option(DummyValue, "__status", translate("Status"))
o.width="12%"
o.rawhtml = true

local start_stop_remove = function(m,cmd)
	local container_selected = {}

	for k in pairs(container_list) do
		if container_list[k]._selected == 1 then
			container_selected[#container_selected + 1] = container_list[k]._name
		end
	end

	if #container_selected  > 0 then
		local success = true

		podman:clear_status()
		for _, cont in ipairs(container_selected) do
			podman:append_status("Containers: " .. cmd .. " " .. cont .. "...")
			local res = dk.containers[cmd](dk, {id = cont})
			if res and res.code >= 300 then
				success = false
				podman:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
			else
				podman:append_status("done\n")
			end
		end

		if success then
			podman:clear_status()
		end

		luci.http.redirect(luci.dispatcher.build_url("admin/podman/containers"))
	end
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template="cbi/nullsection"

o = s:option(Button, "_new")
o.inputtitle= translate("Add")
o.template = "podman/cbi/inlinebutton"
o.inputstyle = "add"
o.forcewrite = true
o.write = function(self, section)
	luci.http.redirect(luci.dispatcher.build_url("admin/podman/newcontainer"))
end

o = s:option(Button, "_start")
o.template = "podman/cbi/inlinebutton"
o.inputtitle=translate("Start")
o.inputstyle = "apply"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"start")
end

o = s:option(Button, "_restart")
o.template = "podman/cbi/inlinebutton"
o.inputtitle=translate("Restart")
o.inputstyle = "reload"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"restart")
end

o = s:option(Button, "_stop")
o.template = "podman/cbi/inlinebutton"
o.inputtitle=translate("Stop")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"stop")
end

o = s:option(Button, "_kill")
o.template = "podman/cbi/inlinebutton"
o.inputtitle=translate("Kill")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"kill")
end

o = s:option(Button, "_remove")
o.template = "podman/cbi/inlinebutton"
o.inputtitle=translate("Remove")
o.inputstyle = "remove"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"remove")
end

return m
