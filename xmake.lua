add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
add_requires("levilamina 26.32.*", {configs = {target_type = get_config("target_type")}})

add_requires("levibuildscript")
add_requires("legacyremotecall v0.20.0", {configs = {target_type = get_config("target_type")}})
add_requires("nlohmann_json v3.11.3")
add_requires("parallel-hashmap v2.0.0")


if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("publish")
    set_default(false)
    set_showmenu(true)
option_end()

target("LK-Stats") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    if is_plat("windows") then
        add_defines("NOMINMAX", "UNICODE", "REMOTE_CALL_EXPORT")
        set_exceptions("none") -- To avoid conflicts with /EHa.
        add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
        add_cxflags(
            "/EHs",
            "-Wno-microsoft-cast",
            "-Wno-invalid-offsetof",
            "-Wno-c++2b-extensions",
            "-Wno-microsoft-include",
            "-Wno-overloaded-virtual",
            "-Wno-ignored-qualifiers",
            "-Wno-missing-field-initializers",
            "-Wno-potentially-evaluated-expression",
            "-Wno-pragma-system-header-outside-header",
            {tools = {"clang_cl"}}
        )
        set_toolchains("clang-cl")
    end
    add_packages("levilamina", "legacyremotecall", "nlohmann_json", "parallel-hashmap")
    -- set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_files("src/**.rc")
    set_configdir("$(builddir)/config")
    add_configfiles("src/(mod/Version.h.in)")
    add_includedirs("src", "$(builddir)/config")
    if is_config("target_type", "server") then
        -- add_defines("LL_PLAT_S")
    --  add_includedirs("src-server")
    --  add_files("src-server/**.cpp")
    else
        -- add_defines("LL_PLAT_C")
    --  add_includedirs("src-client")
    --  add_files("src-client/**.cpp")
    end
    on_load(function (target)
        import("core.base.json")
        local modVersionString = json.loadfile("tooth.json")["version"]
        local modVersionMajor, modVersionMinor, modVersionPatch =
            modVersionString:match("^(%d+)%.(%d+)%.(%d+)$")
        if not modVersionMajor then raise("tooth.json version must use major.minor.patch format") end

        local hash = os.iorun("git rev-parse --short HEAD"):gsub("\n", "")
        local versionStr = modVersionString .. "+" .. hash
        target:set("version", modVersionString)
        target:set("configvar", "LK_STATS_VERSION_MAJOR", modVersionMajor)
        target:set("configvar", "LK_STATS_VERSION_MINOR", modVersionMinor)
        target:set("configvar", "LK_STATS_VERSION_PATCH", modVersionPatch)
        target:set("configvar", "LK_STATS_VERSION_STRING", versionStr)

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

target("LK-Stats-Tests")
    set_default(false)
    set_kind("binary")
    set_languages("c++20")
    if is_plat("windows") then
        add_cxflags("/utf-8")
    end
    add_includedirs("src")
    add_packages("nlohmann_json")
    add_files("tests/*.cpp")
    add_files("src/mod/Stats/StatsData.cpp")
    add_files("src/mod/Stats/StatsJsonCodec.cpp")
    add_files("src/mod/Stats/StatsQuery.cpp")
    add_files("src/mod/Stats/StatsWriteQueue.cpp")

target("LK-Stats-Benchmarks")
    set_default(false)
    set_kind("binary")
    set_languages("c++20")
    if is_plat("windows") then
        add_cxflags("/utf-8")
    end
    add_includedirs("src")
    add_syslinks("psapi")
    add_files("benchmarks/StatsQueryBenchmark.cpp")
    add_files("src/mod/Stats/StatsData.cpp")
    add_files("src/mod/Stats/StatsQuery.cpp")
