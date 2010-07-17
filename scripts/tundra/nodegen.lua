module(..., package.seeall)

local util = require "tundra.util"
local path = require "tundra.path"
local native = require "tundra.native"

_generator = {
	evaluators = {},
}
_generator.__index = _generator

function generate(env, raw_nodes, default_names, passes)
	local state = setmetatable({
		units = {},
		unit_nodes = {},
		base_env = env,
		passes = passes,
	}, _generator)

	-- Build name=>decl mapping
	for _, unit in ipairs(raw_nodes) do
		assert(unit.Decl)
		local name = assert(unit.Decl.Name)
		assert(not state.units[name])
		state.units[name] = unit
	end

	local nodes_to_build = util.map(default_names, function (name) return state:get_node_of(name) end)

	return env:make_node {
		Label = "all",
		Dependencies = nodes_to_build,
	}
end

function _generator:get_node_of(name)
	local n = self.unit_nodes[name]
	if not n then
		self.unit_nodes[name] = "!"
		n = self:eval_unit(assert(self.units[name]))
		self.unit_nodes[name] = n
	else
		assert(n ~= "!")
	end
	return n
end

function _generator:resolve_pass(name)
	return self.passes[name]
end

function _generator:resolve_deps(env, deps)
	if not deps then
		return nil
	end

	local result = {}
	for i, dep in ipairs(deps) do
		result[i] = self:get_node_of(dep)
	end
	return result
end

function _generator:resolve_sources(env, items, accum)
	local header_exts = {}
	for _, ext in ipairs(env:get_list("HEADERS_EXTS")) do
		header_exts[ext] = true
	end
	for _, item in util.nil_ipairs(items) do
		local type_name = type(item)

		while type_name == "function" do
			item = item(env)
			type_name = type(item)
		end

		if type_name == "userdata" then
			accum[#accum + 1] = item
		elseif type_name == "table" then
			if getmetatable(item) then
				accum[#accum + 1] = item
			else
				self:resolve_sources(env, item, accum)
			end
		else
			assert(type_name == "string")
			local ext = path.get_extension(item)
			if not header_exts[ext] then
				accum[#accum + 1] = item
			end
		end
	end
	return accum
end

-- Analyze source list, returning list of input files and list of dependencies.
--
-- This is so you can pass a mix of actions producing files and regular
-- filenames as inputs to the next step in the chain and the output files of
-- such nodes will be used automatically.
--
-- list - list of source files and nodes that produce source files
-- suffixes - acceptable source suffixes to pick up from nodes in source list
-- transformer (optional) - transformer function to make nodes from plain filse
--
function _generator:analyze_sources(list, suffixes, transformer)
	if not list then
		return nil
	end

	list = util.flatten(list)
	local deps = {}

	local function transform(output, fn)
		if type(fn) ~= "string" then
			error(util.tostring(fn) .. " is not a string", 2)
		end
		if transformer then
			local t = transformer(fn)
			if t then
				deps[#deps + 1] = t
				t:insert_output_files(output, suffixes)
			else
				output[#output + 1] = fn
			end
		else
			output[#output + 1] = fn
		end
	end

	local files = {}
	for _, src in ipairs(list) do
		if native.is_node(src) then
			deps[#deps + 1] = src
			src:insert_output_files(files, suffixes)
		else
			files[#files + 1] = src
		end
	end

	local result = {}
	for _, src in ipairs(files) do
		transform(result, src)
	end

	return result, deps
end

function _generator:get_target(decl, suffix)
	local target = decl.Target
	if not target then
		target = "$(OBJECTDIR)/" .. decl.Name .. suffix
	end
	return target
end

function _generator:eval_unit(unit)
	local unit_env = self.base_env:clone()
	local decl = unit.Decl
	local unit_type = unit.Type
	local eval_fn = self.evaluators[unit_type]

	if not eval_fn then
		error(string.format("%s: unsupported unit type", unit_type))
	end

	return eval_fn(self, unit_env, decl)
end

function add_evaluator(name, fn)
	_generator.evaluators[name] = fn
end

function add_generator_set(id)
	local fn = TundraRootDir .. "/scripts/tundra/nodegen/" .. id .. ".lua"
	local chunk = assert(loadfile(fn))
	chunk(_generator)
end

