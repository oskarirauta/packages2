--[[
LuCI - Lua Configuration Interface
Copyright 2021 Oskari Rauta <https://github.com/oskarirauta/lucipodman>
]]--

require "nixio.util"
require "luci.util"
local jsonc = require "luci.jsonc"
local nixio = require "nixio"
local ltn12 = require "luci.ltn12"
local fs = require "nixio.fs"

local urlencode = luci.util.urlencode or luci.http and luci.http.protocol and luci.http.protocol.urlencode
local json_stringify = jsonc.stringify
local json_parse = jsonc.parse

local chunksource = function(sock, buffer)
	buffer = buffer or ""

	return function()
		local output
		local _, endp, count = buffer:find("^([0-9a-fA-F]+)\r\n")

		if not count then
			local newblock, code = sock:recv(1024)
			if not newblock then
				return nil, code
			end
			buffer = buffer .. newblock
			_, endp, count = buffer:find("^([0-9a-fA-F]+)\r\n")
		end

		count = tonumber(count, 16)
		if not count then
			return nil, -1, "invalid encoding"
		elseif count == 0 then -- finial
			return nil
		elseif count <= #buffer - endp then -- data >= count
			output = buffer:sub(endp + 1, endp + count)
			if count == #buffer - endp then -- [data]
				buffer = buffer:sub(endp + count + 1)
				count, code = sock:recvall(2) --read \r\n
				if not count then
					return nil, code
				end
			elseif count + 1 == #buffer - endp then  -- [data]\r
				buffer = buffer:sub(endp + count + 2)
				count, code = sock:recvall(1) --read \n
				if not count then
					return nil, code
				end
			else -- [data]\r\n[count]\r\n[data]...
				buffer = buffer:sub(endp + count + 3) -- cut buffer
			end
			return output
		else -- data < count
			output = buffer:sub(endp + 1, endp + count)
			buffer = buffer:sub(endp + count + 1)
			local remain, code = sock:recvall(count - #output) --need read remaining
			if not remain then
				return nil, code
			end
			output = output .. remain
			count, code = sock:recvall(2) --read \r\n
			if not count then
				return nil, code
			end
			return output
		end
	end
end

local chunksink = function (sock)
	return function(chunk, err)
		if not chunk then
			return sock:writeall("0\r\n\r\n")
		else
			return sock:writeall(("%X\r\n%s\r\n"):format(#chunk, tostring(chunk)))
		end
	end
end

local podman_stream_filter = function(buffer)
	buffer = buffer or ""
	if #buffer < 8 then
		return ""
	end
	local stream_type = ((string.byte(buffer, 1) == 1) and "stdout") or ((string.byte(buffer, 1) == 2) and "stderr") or ((string.byte(buffer, 1) == 0) and "stdin") or "stream_err"
	local valid_length = tonumber(string.byte(buffer, 5)) * 256 * 256 * 256 + tonumber(string.byte(buffer, 6)) * 256 * 256 + tonumber(string.byte(buffer, 7)) * 256 + tonumber(string.byte(buffer, 8))
	if valid_length > #buffer + 8 then
		return ""
	end
	return stream_type .. ": " .. string.sub(buffer, 9, valid_length + 8)
end

local open_socket = function(req_options)
	local socket
	if type(req_options) ~= "table" then
		return socket
	end

	socket = nixio.socket("unix", "stream")
	if socket:connect(req_options.socket_path) ~= true then
		return nil
	end

	if socket then
		return socket
	else
		return nil
	end
end

local send_http_socket = function(options, podman_socket, req_header, req_body, callback)
	if podman_socket:send(req_header) == 0 then
		return {
			request = {
				header=req_header,
				body=req_body
			},
			headers = {
				code=498,
				message="bad path",
				protocol="HTTP/1.1"
			},
			body = {
				message="can\'t send data to socket"
			}
		}
	end

	if req_body and type(req_body) == "function" and req_header and req_header:match("chunked") then
		-- chunked send
		req_body(chunksink(podman_socket))
	elseif req_body and type(req_body) == "function" then
		-- normal send by req_body function
		req_body(podman_socket)
	elseif req_body and type(req_body) == "table" then
		-- json
		podman_socket:send(json_stringify(req_body))
	elseif req_body then
		podman_socket:send(req_body)
	end

	local linesrc = podman_socket:linesource()
	-- read socket using source http://w3.impa.br/~diego/software/luasocket/ltn12.html
	-- http://lua-users.org/wiki/FiltersSourcesAndSinks
	-- handle response header
	local line = linesrc()
	if not line then
		podman_socket:close()
		return {
			request = {
				header=req_header,
				body=req_body
			},
			headers = {
				code=499,
				message="bad socket path",
				protocol="HTTP/1.1"
			},
			body = {
				message="no data receive from socket"
			}
		}
	end

	local response = {
		request = {
			header=req_header,
			body=req_body
		},
		code = 0,
		headers = {},
		body = {}
	}

	local p, code, msg = line:match("^([%w./]+) ([0-9]+) (.*)")
	response.protocol = p
	response.code = tonumber(code)
	response.message = msg
	line = linesrc()

	while line and line ~= "" do
		local key, val = line:match("^([%w-]+)%s?:%s?(.*)")
		if key and key ~= "Status" then
			if type(response.headers[key]) == "string" then
				response.headers[key] = {
					response.headers[key],
					val
				}
			elseif type(response.headers[key]) == "table" then
				response.headers[key][#response.headers[key] + 1] = val
			else
				response.headers[key] = val
			end
		end
		line = linesrc()
	end

	-- handle response body
	local body_buffer = linesrc(true)
	response.body = {}

	if type(callback) ~= "function" then
		if response.headers["Transfer-Encoding"] == "chunked" then
			local source = chunksource(podman_socket, body_buffer)
			code = ltn12.pump.all(source, (ltn12.sink.table(response.body))) and response.code or 555
			response.code = code
		else
			local body_source = ltn12.source.cat(ltn12.source.string(body_buffer), podman_socket:blocksource())
			code = ltn12.pump.all(body_source, (ltn12.sink.table(response.body))) and response.code or 555
			response.code = code
		end
	else
		if response.headers["Transfer-Encoding"] == "chunked" then
			local source = chunksource(podman_socket, body_buffer)
			callback(response, source)
		else
			local body_source = ltn12.source.cat(ltn12.source.string(body_buffer), podman_socket:blocksource())
			callback(response, body_source)
		end
	end
	podman_socket:close()
	return response
end

local gen_header = function(options, http_method, api_group, api_action, name_or_id, request)
	local header, query, path
	name_or_id = ((name_or_id ~= "") and name_or_id) or nil

	if request and type(request.query) == "table" then
		local k, v
		for k, v in pairs(request.query) do
			if type(v) == "table" then
				query = (query and query .. "&" or "?") .. k .. "=" .. urlencode(json_stringify(v))
			elseif type(v) == "boolean" then
				query = (query and query .. "&" or "?") .. k .. "=" .. (v and "true" or "false")
			elseif type(v) == "number" or type(v) == "string" then
				query = (query and query .. "&" or "?") .. k .. "=" .. v
			end
		end
	end

	path = (api_group and ("/" .. api_group) or "") .. (name_or_id and ("/" .. name_or_id) or "") .. (api_action and ("/" .. api_action) or "") .. (query or "")
	header = (http_method or "GET") .. " " .. (options.version and ("/" .. options.version) or "") .. ( options.path and ("/" .. options.path) or "" ) .. path .. " " .. options.protocol .. "\r\n"
	header = header .. "Host: " .. options.host .. "\r\n"
	header = header .. "User-Agent: " .. options.user_agent .. "\r\n"
	header = header .. "Connection: close\r\n"

	if request and type(request.header) == "table" then
		local k, v
		for k, v in pairs(request.header) do
			header = header .. k .. ": " .. v .. "\r\n"
		end
	end

	-- when requst_body is function, we need to custom header using custom header
	if request and request.body and type(request.body) == "function" then
		if not header:match("Content-Length:") then
			header = header .. "Transfer-Encoding: chunked\r\n"
		end
	elseif http_method == "POST" and request and request.body and type(request.body) == "table" then
		local conetnt_json = json_stringify(request.body)
		header = header .. "Content-Type: application/json\r\n"
		header = header .. "Content-Length: " .. #conetnt_json .. "\r\n"
	elseif request and request.body and type(request.body) == "string" then
		header = header .. "Content-Length: " .. #request.body .. "\r\n"
	end

	header = header .. "\r\n"

	return header
end

local call_podman = function(options, http_method, api_group, api_action, name_or_id, request, callback)
	local req_options = setmetatable({}, {
		__index = options
	})

	local req_header = gen_header(req_options,
		http_method,
		api_group,
		api_action,
		name_or_id,
		request)

	local req_body = request and request.body or nil
	local podman_socket = open_socket(req_options)

	if podman_socket then
		return send_http_socket(options, podman_socket, req_header, req_body, callback)
	else
		return {
			request = {
				header=req_header,
				body=req_body
			},
			headers = {
				code=497,
				message="bad socket path",
				protocol="HTTP/1.1"
			},
			body = {
				message="can\'t connect to socket"
			}
		}
	end
end

local debuglog = function(contents, debug_path)

	if debug_path and debug_path ~= "" then
		local file, err = io.open(debug_path, "a")
		if not err then
			file:write(contents)
			file:close()
		end
	end

end


local gen_api = function(_table, http_method, api_group, api_action)
	local _api_action = api_action

	if api_action == "get_archive" or api_action == "put_archive" then
		_api_action = "archive"
	elseif api_action == "df" then
		_api_action = "system/df"
	elseif api_action ~= "list" and api_action ~= "inspect" and api_action ~= "remove" then
		_api_action = api_action
	elseif (api_group == "containers" or api_group == "pods" or api_group == "images" or api_group == "networks" or api_group == "exec" ) and (api_action == "list" or api_action == "inspect") then
		_api_action = "json"
	end

	local fp = function(self, request, callback)
		local name_or_id = request and (request.name or request.id or request.name_or_id) or nil

		local _api_group = api_group
		if api_group == "pods" and api_action ~= "list" then
			_api_group = "pod"
		end

		if api_action == "list" then
			if (name_or_id ~= "" and name_or_id ~= nil) then
				if _api_group == "containers" then
					name_or_id = "all"
				elseif _api_group == "images" then
					name_or_id = nil
				else
					request.query = request and request.query or {}
					request.query.filters = request.query.filters or {}
					request.query.filters.name = request.query.filters.name or {}
					request.query.filters.name[#request.query.filters.name + 1] = name_or_id
					name_or_id = nil
				end
			end
		elseif api_action == "create" then
			if (name_or_id ~= "" and name_or_id ~= nil) then
				request.query = request and request.query or {}
				request.query.name = request.query.name or name_or_id
				name_or_id = nil
			end
		elseif api_action == "logs" then
			local body_buffer = ""
			local response = call_podman(self.options,
				http_method,
				_api_group,
				_api_action,
				name_or_id,
				request,
				callback)
			if response.code >= 200 and response.code < 300 then
				for i, v in ipairs(response.body) do
					body_buffer = body_buffer .. podman_stream_filter(response.body[i])
				end
				response.body = body_buffer
			end
			return response
		end

		local response = call_podman(self.options, http_method, _api_group, _api_action, name_or_id, request, callback)

		response.result = ""
		if response.headers and response.headers["Content-Type"] == "application/json" then

			if #response.body == 1 then
				response.result = response.body[1]
				response.body = json_parse(response.result)
			else
				for _, v in ipairs(response.body) do
					response.result = response.result .. v
				end
				response.body = json_parse(response.result)
				if #response.body == 1 then
					reponse.result = response.body[1]
					response.body = json_parse(response.result)
				end
			end
		end

		if self.options.debug then

			local text
			for line in response.request.header:gmatch("([^\n]*)\n?") do
				text = line
				break
			end
			if response.request.body and type(response.request.body) == "table" then
				text = text .. "\nrequest body:\n" .. json_stringify(response.request.body)
			elseif response.request.body then
				text = text .. "\nrequest body: " .. response.request.body
			end
			text = text .. "\ncode: " .. tostring(response.code or response.headers.code or "nil") .. (response.result ~= "" and (", response:\n" .. response.result) or "") .. "\n"
			debuglog(text, self.options.debug_path)

		end

		return response
	end

	if api_group then
		_table[api_group][api_action] = fp
	else
		_table[api_action] = fp
	end
end

local _podman = {
	containers = {},
	pods = {},
	exec = {},
	images = {},
	networks = {},
	volumes = {}
}

gen_api(_podman, "GET", "containers", "list")
gen_api(_podman, "POST", "containers", "create")
gen_api(_podman, "GET", "containers", "inspect")
gen_api(_podman, "GET", "containers", "top")
gen_api(_podman, "GET", "containers", "logs")
gen_api(_podman, "GET", "containers", "changes")
gen_api(_podman, "GET", "containers", "stats")
gen_api(_podman, "POST", "containers", "resize")
gen_api(_podman, "POST", "containers", "start")
gen_api(_podman, "POST", "containers", "stop")
gen_api(_podman, "POST", "containers", "restart")
gen_api(_podman, "POST", "containers", "kill")
gen_api(_podman, "POST", "containers", "update")
gen_api(_podman, "POST", "containers", "rename")
gen_api(_podman, "POST", "containers", "pause")
gen_api(_podman, "POST", "containers", "unpause")
gen_api(_podman, "POST", "containers", "update")
gen_api(_podman, "DELETE", "containers", "remove")
gen_api(_podman, "POST", "containers", "prune")
gen_api(_podman, "POST", "containers", "exec")
gen_api(_podman, "POST", "exec", "start")
gen_api(_podman, "POST", "exec", "resize")
gen_api(_podman, "GET", "exec", "inspect")
gen_api(_podman, "GET", "containers", "get_archive")
gen_api(_podman, "PUT", "containers", "put_archive")
-- TODO: export,attch

gen_api(_podman, "POST", "pods", "create")
gen_api(_podman, "GET", "pods", "exists")
gen_api(_podman, "GET", "pods", "inspect")
gen_api(_podman, "POST", "pods", "kill")
gen_api(_podman, "POST", "pods", "pause")
gen_api(_podman, "POST", "pods", "prune")
gen_api(_podman, "GET", "pods", "list")
gen_api(_podman, "POST", "pods", "restart")
gen_api(_podman, "DELETE", "pods", "remove")
gen_api(_podman, "POST", "pods", "start")
gen_api(_podman, "GET", "pods", "stats")
gen_api(_podman, "POST", "pods", "stop")
gen_api(_podman, "GET", "pods", "top")
gen_api(_podman, "POST", "pods", "unpause")

gen_api(_podman, "GET", "images", "list")
gen_api(_podman, "POST", "images", "create")
gen_api(_podman, "GET", "images", "inspect")
gen_api(_podman, "GET", "images", "history")
gen_api(_podman, "POST", "images", "tag")
gen_api(_podman, "DELETE", "images", "remove")
gen_api(_podman, "GET", "images", "search")
gen_api(_podman, "POST", "images", "prune")
gen_api(_podman, "GET", "images", "get")
gen_api(_podman, "POST", "images", "load")

gen_api(_podman, "GET", "networks", "list")
gen_api(_podman, "GET", "networks", "inspect")
gen_api(_podman, "DELETE", "networks", "remove")
gen_api(_podman, "POST", "networks", "create")
gen_api(_podman, "POST", "networks", "connect")
gen_api(_podman, "POST", "networks", "disconnect")
gen_api(_podman, "POST", "networks", "prune")

gen_api(_podman, "GET", "volumes", "list")
gen_api(_podman, "GET", "volumes", "inspect")
gen_api(_podman, "DELETE", "volumes", "remove")
gen_api(_podman, "POST", "volumes", "create")

gen_api(_podman, "GET", nil, "events")
gen_api(_podman, "GET", nil, "version")
gen_api(_podman, "GET", nil, "info")
gen_api(_podman, "GET", nil, "_ping")
gen_api(_podman, "GET", nil, "df")

function _podman.new(options)
	local podman = {}
	local _options = options or {}

	podman.options = {
		socket_path = _options.socket_path or "/run/podman/podman.sock",
		host = _options.host or "localhost",
		path = _options.path or "libpod",
		version = _options.version or "v3.0.0",
		user_agent = _options.user_agent or "LuCI",
		protocol = _options.protocol or "HTTP/1.1",
		debug = _options.debug or false,
		debug_path = _options.debug and _options.debug_path or nil
	}

	setmetatable(
		podman,
		{
		__index = function(t, key)
			if _podman[key] ~= nil then
				return _podman[key]
			else
				return _podman.containers[key]
			end
		end
		}
	)

	setmetatable(
		podman.containers,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	setmetatable(
		podman.pods,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	setmetatable(
		podman.networks,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	setmetatable(
		podman.images,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	setmetatable(
		podman.volumes,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	setmetatable(
		podman.exec,
		{
		__index = function(t, key)
			if key == "options" then
				return podman.options
			end
		end
		}
	)

	return podman
end

return _podman
