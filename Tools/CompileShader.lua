require "getopt_alt"
require "string_ext"

local all_options = getopt(arg, nil)
local verbose = all_options["verbose"]
local debug_shader = all_options["debug_shader"]
local target_platform = all_options["platform"]
local shader_file = string.gsub(all_options["file"], "\\", "/")
local output_dir = string.gsub(all_options["output"], "\\", "/")
local output_intermediate_dir = "Temp/"

local shader_pathcomponents = shader_file:split("/")
local shader_filename = shader_pathcomponents[#shader_pathcomponents]
local shader_filename_without_ext = string.gsub(shader_filename, ".hlsl", "")

local output_file = output_dir .. shader_filename_without_ext .. ".h"

if verbose then
	print("    Target Platform : " .. target_platform)
	print("    Shader File : " .. shader_file)
	print("    Output Directory : " .. output_dir)
	print("    Debug Shader : " .. tostring(debug_shader))
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

local optimization_flags = " /O3"
if debug_shader then
	optimization_flags = " /Od /Zi"
end

local function Compile(profile_name, shader_profile, entry)
	local new_filename = shader_filename_without_ext .. "_" .. profile_name
	local shader_varname = "g_" .. new_filename
	local output_header_file = " /Fh " .. output_dir .. output_intermediate_dir .. new_filename .. ".h"
	local output_pdb_file = " "
	local specify_root_sig_ver = " /force_rootsig_ver rootsig_1_1"
	-- TODO: not supported by WIN7
	specify_root_sig_ver = ""
	-- if debug_shader then
	-- 	output_pdb_file = " /Fd " .. output_dir .. new_filename .. ".pdb"
	-- end
	local cmd = fxc
	 			.. " /T " .. shader_profile
	 			.. " /E " .. entry
	 			.. common_flags
	 			.. optimization_flags
	 			.. " " .. shader_file
	 			.. output_header_file
	 			.. output_pdb_file
	 			.. " /Vn " .. shader_varname
	 			.. specify_root_sig_ver

	if verbose then
		print(cmd)
	end

	os.execute(cmd)

	local code = "#include \"" .. output_intermediate_dir .. new_filename .. ".h" .. "\""
	code = code .. "\n"
	code = code .. "static const D3D12_SHADER_BYTECODE " .. shader_varname .. "_bytecode = { " .. shader_varname .. ", sizeof(" .. shader_varname .. ") };"
	code = code .. "\n"
	return code
end

local all_profiles = {}
all_profiles["VS"] = {"vs_5_1", "VSMain"}
all_profiles["GS"] = {"gs_5_1", "GSMain"}
all_profiles["PS"] = {"ps_5_1", "PSMain"}
all_profiles["CS"] = {"cs_5_1", "CSMain"}

local inlcude_header_content = ""
for line in io.lines(shader_file) do
	for profile_name, profile_prop in pairs(all_profiles) do
		local shader_profile = profile_prop[1]
		local entry_func = profile_prop[2]
		if string.find(line, "%s*//") then
			-- Ignore comments
		elseif string.find(line, entry_func) then
			local inc = Compile(profile_name, shader_profile, entry_func, inc)
			inlcude_header_content =  inlcude_header_content .. inc .. "\n"
		end
	end
end

local f = io.open(output_file, "w")
f:write(inlcude_header_content)
f:close()