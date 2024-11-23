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
        "./src/*.c",
        "./src/ui/*.c",
        "./src/game/*.c"
    }
    includedirs {
        "./include"
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
            "switch-enum",
            "gnu-zero-variadic-macro-arguments",
            "declaration-after-statement"
        }

    filter "configurations:Release"
        optimize "On"

        postbuildcommands {
            "doxygen dconfig",
            "cd ./docs/latex && pdflatex -interaction=batchmode refman.tex ; cd ../../",
            "mv ./docs/latex/refman.pdf programozoi_dokumentacio.pdf",
            "rm -r ./docs"
        }

        warnings "Off"
        externalwarnings "Off"
