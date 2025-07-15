add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
if is_config("target_type", "server") then
    add_requires("levilamina 1.4.0", {configs = {target_type = "server"}})
else
    add_requires("levilamina 1.4.0", {configs = {target_type = "client"}})
end

add_requires("levibuildscript")
add_requires(
    "legacyremotecall 0.10.0"
)


if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("publish")
    set_default(false)
    set_showmenu(true)
option_end()

target("LK-Stats") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    -- add_rules("@levibuildscript/modpacker")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages("levilamina","legacyremotecall")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_files("src/**.cpp")
    add_includedirs("src")
    -- if is_config("target_type", "server") then
    --     add_includedirs("src-server")
    --     add_files("src-server/**.cpp")
    -- else
    --     add_includedirs("src-client")
    --     add_files("src-client/**.cpp")
    -- end
    on_load(function (target)
        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local versionStr =  major.."."..minor.."."..patch
        if suffix then
            prerelease = suffix:match("-(.*)")
            if prerelease then
                prerelease = prerelease:gsub("\n", "")
            end
            if prerelease then
                versionStr = versionStr.."-"..prerelease
            end
        end
        if not has_config("publish") then
            local hash = os.iorun("git rev-parse --short HEAD")
            versionStr = versionStr.."+"..hash:gsub("\n", "")
        end

        target:add("rules", "@levibuildscript/modpacker",{
               modVersion = versionStr
           })
    end)

    after_build(function(target)
        local langPath = path.join(os.projectdir(), "src/lang/")
        local outputPath = path.join(os.projectdir(), "bin/" .. target:name())
        os.mkdir(outputPath)
        os.cp(langPath, outputPath)
    end)
