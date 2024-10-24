workspace "carcassone-clone"
    startproject "CarcassoneClone"
    configurations {
        "Debug",
        "Release"
    }

project "CarcassoneClone"
    kind "WindowedApp"
    language "C"
    cdialect "GNU11"
    toolset "clang"
    targetdir "./bin/%{prj.name}_%{cfg.buildcfg}"
    targetname "crclone"
    objdir "./bin-int"

    files {
        "./src/**.c"
    }
    includedirs {
        "./include",
        "./src"
    }
    externalincludedirs {
        "./include/debug"
    }
    links {
        "SDL2",
        "SDL2_ttf"
    }

    filter {"system:linux", "action:gmake"}
        buildoptions {
            "-pedantic",
            "-gdwarf-4"
        }

    filter "configurations:Debug"
        symbols "On"
        defines {
            "_CRCLONE_DEBUG"
        }

        warnings "Everything"
        externalwarnings "Off"
        disablewarnings {
            "unused-parameter",
            "newline-eof",
            "padded",
            "switch-enum"
        }

    filter "configurations:Release"
        optimize "On"

        warnings "Off"
        externalwarnings "Off"
