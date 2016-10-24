function str_split(inputstr, sep)
	if sep == nil then
		sep = "%s"
	end

	local t={} ; i=1
	for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
		t[i] = str
		i = i + 1
	end
	return t
end

verbose = false
debug_shader = false
target_platform = nil
shader_file = nil
output_dir = nil
defines = {}

i = 1
while i <= #arg do

	if arg[i] == "-v" or arg[i] == "--verbose" then
		verbose = true

	elseif arg[i] == "-d" or arg[i] == "--debug_shader" then
		debug_shader = true

	elseif arg[i] == "-p" or arg[i] == "--platform" then
		target_platform = arg[i + 1]
		i = i + 1

	elseif arg[i] == "-d" or arg[i] == "--define" then
		local s = arg[i + 1]
		i = i + 1

		for v0 in string.gmatch(s, '([^;]+)') do
			local tbl = {}
			table.insert(defines, tbl)
			for v1 in string.gmatch(v0, '([^,]+)') do
				table.insert(tbl, v1)
			end
		end

	elseif arg[i] == "-f" or arg[i] == "--file" then
		shader_file = arg[i + 1]
		shader_file = string.gsub(shader_file, "\\", "/")
		i = i + 1

	elseif arg[i] == "-o" or arg[i] == "--output" then
		output_dir = arg[i + 1]
		output_dir = string.gsub(output_dir, "\\", "/")
		i = i + 1

	else
		print("unknow parameter : " .. arg[i])
	end

	i = i + 1
end

-- insert null definition
table.insert(defines, {""})

if verbose then
	print("Target Platform : " .. target_platform)
	print("Shader File : " .. shader_file)
	for _, v0 in pairs(defines) do
		local defineStr = ""
		for _, v1 in pairs(v0) do
			defineStr = defineStr .. v1 .. ", "
		end
		print("Define : ", defineStr)
	end
end


local is_durango = target_platform == "Durango"
local fxc = nil
if is_durango then
    fxc = os.getenv("DurangoXDK").."bin/pixsc/FXC.exe"
else
    fxc = "C:/Program Files (x86)/Windows Kits/10/bin/x64/fxc.exe"
end
fxc = string.format('%q', string.gsub(fxc, "\\", "/"))

local common_flags = " /nologo"
if is_durango then
else
	common_flags = common_flags .. " /enable_unbounded_descriptor_tables"
end

-- TODO: fix it
if is_durango then
	debug_shader = false
end

local optimization_flags = " /O3"
if debug_shader then
	optimization_flags = " /Od /Zi"
end

local shader_pathcomponents = str_split(shader_file, "/")
local shader_filename = shader_pathcomponents[#shader_pathcomponents]
local shader_filename_without_ext = string.gsub(shader_filename, ".hlsl", "")

local function Compile(profile_name, shader_profile, entry)
	for _, v0 in pairs(defines) do
		local filename_extension = ""
		local definitions = " "
		for _, def in pairs(v0) do
			if string.len(def) > 0 then
				definitions = definitions .. " -D " .. def
				filename_extension = filename_extension .. def .. "_"
			end
		end
		if (string.len(filename_extension)) > 0 then
			filename_extension = string.sub(filename_extension, 1, string.len(filename_extension) - 1)
			filename_extension = "_" .. filename_extension
		end

		local new_filename = shader_filename_without_ext .. "_" .. profile_name .. filename_extension
		local shader_varname = " /Vn g_" .. new_filename
		local output_header_file = " /Fh " .. output_dir .. new_filename .. ".h"
		local output_pdb_file = ""
		if debug_shader then
			output_pdb_file = " /Fd " .. output_dir .. new_filename .. ".pdb"
		end
		local cmd = fxc
		 			.. " /T " .. shader_profile
		 			.. " /E " .. entry
		 			.. common_flags
		 			.. optimization_flags
		 			.. " " .. shader_file
		 			.. definitions
		 			.. output_header_file
		 			.. output_pdb_file
		 			.. shader_varname

		if verbose then
			print(cmd)
		end

		os.execute(cmd)
	end
end

local all_profiles = {}
all_profiles["VS"] = {"vs_5_1", "VSMain"}
all_profiles["GS"] = {"gs_5_1", "GSMain"}
all_profiles["PS"] = {"ps_5_1", "PSMain"}
all_profiles["CS"] = {"cs_5_1", "CSMain"}

for line in io.lines(shader_file) do
	for profile_name, profile_prop in pairs(all_profiles) do
		local shader_profile = profile_prop[1]
		local entry_func = profile_prop[2]
		if string.find(line, "%s*//") then
			-- Ignore comments
		elseif string.find(line, entry_func) then
			Compile(profile_name, shader_profile, entry_func)
		end
	end
end
