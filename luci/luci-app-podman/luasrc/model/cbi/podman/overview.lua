--[[
LuCI - Lua Configuration Interface
Copyright 2021 Oskari Rauta <https://github.com/oskarirauta/lucipodman>
]]--

local podman = require "luci.model.podman"

local m, s, o
local dk = podman.new()

function byte_format(byte)
	if byte == nil then return nil end
	local suff = {"B", "KB", "MB", "GB", "TB"}
	for i=1, 5 do
		if byte > 1024 and i < 5 then
			byte = byte / 1024
		else
			return string.format("%.2f %s", byte, suff[i])
		end
	end
end

m = SimpleForm("podman", translate("Overview"))
m.submit=false
m.reset=false

local podman_info_table = {}
podman_info_table['3PodmanVersion'] = {_key=translate("Podman version"),_value='-'}
podman_info_table['4ApiVersion'] = {_key=translate("Api version"),_value='-'}
podman_info_table['5NCPU'] = {_key=translate("CPUs"),_value='-'}
podman_info_table['6Memory'] = {_key=translate("Memory"),_value='-'}
podman_info_table['7SocketPath'] = {_key=translate("Socket path"),_value='-'}
podman_info_table['81GraphRoot'] = {_key=translate("Graph root"),_value='-'}
podman_info_table['82RunRoot'] = {_key=translate("Run root"),_value='-'}
podman_info_table['9Registries'] = {_key=translate("Registries"),_value='-'}

s = m:section(Table, podman_info_table)

s:option(DummyValue, "_key", translate("Info"))
s:option(DummyValue, "_value")

s = m:section(SimpleSection)
s.template = "podman/overview"

s.containers_running = '-'
s.containers_total = '-'
s.pods_running = '-'
s.pods_total = '-'
s.images_used = '-'
s.images_total = '-'
s.networks_total = '-'
s.volumes_total = '-'

if dk:_ping().code == 200 then
	local containers_list = dk.containers:list({query = {all=true}}).body
	local pods_list = dk.pods:list().body
	local images_list = dk.images:list().body
	local vol = dk.volumes:list()
	local volumes_list = vol and vol.body and vol.body.Volumes or {}
	local networks_list = dk.networks:list().body or {}
	local podman_info = dk:info().body

	podman_info_table['3PodmanVersion']._value = podman_info.version.Version
	podman_info_table['4ApiVersion']._value = podman_info.version.APIVersion
	podman_info_table['5NCPU']._value = tostring(podman_info.host.cpus)
	podman_info_table['6Memory']._value = byte_format(podman_info.host.memFree) .. " / " .. byte_format(podman_info.host.memTotal)
	if podman_info.store.graphRoot then
		local statvfs = nixio.fs.statvfs(podman_info.store.graphRoot)
		local size = statvfs and (statvfs.bavail * statvfs.bsize) or 0
		podman_info_table['81GraphRoot']._value = podman_info.store.graphRoot .. " (" .. tostring(byte_format(size)) .. " " .. translate("available") .. ")"
	end
	podman_info_table['82RunRoot']._value = podman_info.store.runRoot

	for i, v in ipairs(podman_info.registries.search) do
		podman_info_table['9Registries']._value = podman_info_table['9Registries']._value == "-" and v or (podman_info_table['9Registries']._value .. ", " .. v)
	end

	podman_info_table['7SocketPath']._value = dk.options.socket_path

	s.images_used = 0
	for _, v in ipairs(images_list) do
		for ci,cv in ipairs(containers_list) do
			if v.Id == cv.ImageID then
				s.images_used = s.images_used + 1
				break
			end
		end
	end

	s.pods_running = 0
	for _, v in ipairs(pods_list) do
		if v.Status == "Running" then
			s.pods_running = s.pods_running + 1
		end
	end

	s.containers_running = tostring(podman_info.store.containerStore.running)
	s.containers_total = tostring(podman_info.store.containerStore.number)
	s.pods_running = tostring(s.pods_running)
	s.pods_total = tostring(#pods_list)
	s.images_used = tostring(s.images_used)
	s.images_total = tostring(podman_info.store.imageStore.number)
	s.networks_total = tostring(#networks_list)
	s.volumes_total = tostring(#volumes_list)
end

return m
