add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires(
    "detours v4.0.1-xmake.1",
    "magic_enum v0.9.7"
)

if is_plat("windows") and not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("PacketViolationDebugger")
    set_kind("shared")
    set_languages("c++23")
    set_exceptions("none")
    set_optimize("aggressive")
    set_strip("all")
    add_packages(
        "detours",
        "magic_enum"
    )
    add_includedirs("src")
    add_files(
        "src/**.cpp",
        "src/**.rc"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "MAGIC_ENUM_RANGE_MIN=0",
        "MAGIC_ENUM_RANGE_MAX=512"
    )
    add_cxflags(
        "/EHsc", 
        "/utf-8", 
        "/W4", 
        "/O2", 
        "/Ob3", 
        "/GR-",
        "/Gw",
        "/Zo-",
        "/GL"
    )
    add_shflags(
        "/OPT:REF", 
        "/OPT:ICF",
        "/LTCG"
    )
    after_build(function (target)
        local output_dir = path.join(os.projectdir(), "bin")
        os.cp(target:targetfile(), output_dir)
        os.exec("upx " .. output_dir .. "\\" .. target:filename())
        cprint("${bright green}[Shared Library]: ${reset}Shared Library already generated to " .. output_dir)
    end)