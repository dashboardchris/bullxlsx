solution "xlnt"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    location ("./" .. _ACTION)
    configuration "Debug"
        flags { "Symbols" }

project "xlnt.test"
    kind "ConsoleApp"
    language "C++"
    targetname "xlnt.test"
    includedirs { 
       "../include",
       "../third-party/pugixml/src",
       "../third-party/zlib",
       "../third-party/zlib/contrib/minizip",
       "$(cxxtest_prefix)"
    }
    files { 
       "../tests/*.hpp",
       "../tests/runner-autogen.cpp"
    }
    links { 
	"pugixml",
        "xlnt",
        "zlib"
    }
    prebuildcommands { "/usr/local/Cellar/cxxtest/4.3/bin/cxxtestgen --runner=ErrorPrinter -o /Users/thomas/Development/xlnt/tests/runner-autogen.cpp /Users/thomas/Development/xlnt/tests/*.hpp" }
    flags { 
       "Unicode",
       "NoEditAndContinue",
       "NoManifest",
       "NoPCH"
    }
    configuration "Debug"
	targetdir "../bin"
    configuration "Release"
	targetdir "../bin"
    configuration "windows"
        defines { "WIN32" }
	links { "Shlwapi" }
    configuration "not windows"
        buildoptions { 
            "-std=c++11",
            "-Wno-unknown-pragmas"
        }

project "xlnt"
    kind "StaticLib"
    language "C++"
    targetdir "../lib/"
    links { 
        "zlib",
	"pugixml"
    }
    includedirs { 
       "../include/xlnt",
       "../third-party/pugixml/src",
       "../third-party/zlib/",
       "../third-party/zlib/contrib/minizip"
    }
    files {
       "../source/**.cpp",
       "../source/**.hpp",
       "../include/xlnt/**.hpp"
    }
    flags { 
       "Unicode",
       "NoEditAndContinue",
       "NoManifest",
       "NoPCH"
    }
    configuration "Debug"
        flags { "FatalWarnings" }
    configuration "windows"
        defines { "WIN32" }
    configuration "not windows"
        buildoptions { 
            "-std=c++11",
            "-Wno-unknown-pragmas"
        }

project "pugixml"
    kind "StaticLib"
    language "C++"
    targetdir "../lib/"
    includedirs { 
       "../third-party/pugixml/src"
    }
    files {
       "../third-party/pugixml/src/pugixml.cpp"
    }
    flags { 
       "Unicode",
       "NoEditAndContinue",
       "NoManifest",
       "NoPCH"
    }
    configuration "windows"
        defines { "WIN32" }

project "zlib"
    kind "StaticLib"
    language "C"
    targetdir "../lib/"
    includedirs { 
       "../third-party/zlib/",
       "../third-party/zlib/contrib/minizip"
    }
    files {
       "../third-party/zlib/*.c",
       "../third-party/zlib/contrib/minizip/*.c"
    }
    excludes {
       "../third-party/zlib/contrib/minizip/miniunz.c",
       "../third-party/zlib/contrib/minizip/minizip.c",
       "../third-party/zlib/contrib/minizip/iowin32.c"
    }
    flags { 
       "Unicode",
       "NoEditAndContinue",
       "NoManifest",
       "NoPCH"
    }
    configuration "windows"
        defines { "WIN32" }
