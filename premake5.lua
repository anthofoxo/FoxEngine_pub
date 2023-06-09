workspace "game"
    architecture "x86_64"
    startproject "game"
    configurations { "debug", "game_debug", "release", "dist" }
    flags "MultiProcessorCompile"
    language "C++"
    cppdialect "C++20"
    cdialect "C11"
    staticruntime "On"
    stringpooling "On"
    symbols "On"
    kind "StaticLib"
    targetdir "bin/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"
    objdir "bin-int/%{cfg.system}-%{cfg.buildcfg}/%{prj.name}"

    filter "configurations:debug"
        optimize "Debug"
    filter "configurations:not debug"
        optimize "Speed"
    filter "configurations:debug or game_debug"
        runtime "Debug"
    filter "configurations:release or dist"
        runtime "Release"
        vectorextensions "AVX2"
    filter "configurations:dist"
        editandcontinue "Off"
    filter "configurations:dist"
        flags { "LinkTimeOptimization", "NoBufferSecurityCheck" }
    filter "system:windows"
        systemversion "latest"
        defines { "WIN32_LEAN_AND_MEAN", "NOMINMAX" }
    filter { "system:windows", "configurations:debug or game_debug" }
        defines "_ITERATOR_DEBUG_LEVEL=1"
    filter {}

function feProject(name)
    project(name)
    location "vendor/%{prj.name}"
end

vendor_loc = "%{wks.location}/vendor/"

feProject "game"
    location "game"
    kind "ConsoleApp"

    files
	{
		"%{prj.location}/src/**.cpp",
		"%{prj.location}/src/**.hpp",
		"%{prj.location}/src/**.c",
		"%{prj.location}/src/**.h",
        "%{prj.location}/vendor/**.cpp",
		"%{prj.location}/vendor/**.hpp",
		"%{prj.location}/vendor/**.c",
		"%{prj.location}/vendor/**.h"
	}

    includedirs
    {
        "%{prj.location}",
        "%{prj.location}/src",

        vendor_loc .. "glfw/include",
		vendor_loc .. "glad2/include",
		--vendor_loc .. "zlib",
		--vendor_loc .. "zlib/contrib/minizip",
		vendor_loc .. "imgui",
		--vendor_loc .. "assimp_config_h_fix",
		--vendor_loc .. "assimp/include",
		vendor_loc .. "glm",
		vendor_loc .. "tinyfd",
		vendor_loc .. "entt/src",
		vendor_loc .. "spdlog/include",
		--vendor_loc .. "harfbuzz/src",
		--vendor_loc .. "msdfgen",
		--vendor_loc .. "freetype/include",
		vendor_loc .. "assimp_config_h_fix",
		vendor_loc .. "assimp/include",
    }

    links
    {
		"glfw",
		"glad2",
		
		"assimp",
		"tinyfd",
		--"zlib",
		"imgui"
		--"harfbuzz",
		--"msdfgen",
		--"freetype",
		--"zlib"
    }

    --defines "MSDFGEN_PUBLIC="
    defines "GLFW_INCLUDE_NONE"

    filter "system:windows"
        defines "SPDLOG_WCHAR_TO_UTF8_SUPPORT"
        links "opengl32"
    filter "system:linux"
        links
        {
            "dl",
            "pthread",
            "X11",
            "m"
        }
    filter "system:macosx"
        links
        {
            "CoreFoundation.framework",
			"Cocoa.framework",
			"IOKit.framework",
			"CoreVideo.framework"
        }
    filter "configurations:game_debug"
        optimize "Debug"
group "deps"

feProject "glfw"
    language "C"
    files
	{
		"%{prj.location}/src/context.c",
		"%{prj.location}/src/egl_context.c",
		"%{prj.location}/src/egl_context.h",
		"%{prj.location}/src/init.c",
		"%{prj.location}/src/input.c",
		"%{prj.location}/src/internal.h",
		"%{prj.location}/src/monitor.c",
		"%{prj.location}/src/osmesa_context.c",
		"%{prj.location}/src/osmesa_context.h",
		"%{prj.location}/src/vulkan.c",
		"%{prj.location}/src/window.c"
	}
	filter "system:windows"
		files
		{
			"%{prj.location}/src/wgl_context.c",
			"%{prj.location}/src/wgl_context.h",
			"%{prj.location}/src/win32_init.c",
			"%{prj.location}/src/win32_joystick.c",
			"%{prj.location}/src/win32_joystick.h",
			"%{prj.location}/src/win32_monitor.c",
			"%{prj.location}/src/win32_platform.h",
			"%{prj.location}/src/win32_thread.c",
			"%{prj.location}/src/win32_time.c",
			"%{prj.location}/src/win32_window.c"
		}
		defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }
	filter "system:linux"
		files
		{
			"%{prj.location}/src/glx_context.c",
			"%{prj.location}/src/glx_context.h",
			"%{prj.location}/src/linux_joystick.c",
			"%{prj.location}/src/linux_joystick.h",
			"%{prj.location}/src/posix_time.c",
			"%{prj.location}/src/posix_time.h",
			"%{prj.location}/src/posix_thread.c",
			"%{prj.location}/src/posix_thread.h",
			"%{prj.location}/src/x11_init.c",
			"%{prj.location}/src/x11_monitor.c",
			"%{prj.location}/src/x11_platform.h",
			"%{prj.location}/src/x11_window.c",
			"%{prj.location}/src/xkb_unicode.c",
			"%{prj.location}/src/xkb_unicode.h"
		}
		defines { "_GLFW_X11", "_GNU_SOURCE", "_POSIX_C_SOURCE=1999309L" }
	filter "system:macosx"
		files
		{
			"%{prj.location}/src/cocoa_init.m",
			"%{prj.location}/src/cocoa_joystick.m",
			"%{prj.location}/src/cocoa_joystick.h",
			"%{prj.location}/src/cocoa_monitor.m",
			"%{prj.location}/src/cocoa_platform.h",
			"%{prj.location}/src/cocoa_time.c",
			"%{prj.location}/src/cocoa_window.m",
			"%{prj.location}/src/nsgl_context.m",
			"%{prj.location}/src/nsgl_context.h",
			"%{prj.location}/src/posix_thread.c",
			"%{prj.location}/src/posix_thread.h"
		}
		defines "_GLFW_COCOA"
feProject "glad2"
    language "C"
    files "%{prj.location}/src/gl.c"
    includedirs "%{prj.location}/include"
feProject "tinyfd"
    language "C"
    files "%{prj.location}/tinyfiledialogs.c"
--feProject "zlib"
--    language "C"
--    files
--	{
--		"%{prj.location}/*.c",
--		"%{prj.location}/*.h",
--		"%{prj.location}/contrib/minizip/*.c",
--		"%{prj.location}/contrib/minizip/*.h",
--	}
--	includedirs "%{prj.location}"
feProject "imgui"
    language "C++"
    files
	{
		"%{prj.location}/*.cpp",
		"%{prj.location}/*.h",
		"%{prj.location}/backends/imgui_impl_opengl3.cpp",
		"%{prj.location}/backends/imgui_impl_opengl3.h",
		"%{prj.location}/backends/imgui_impl_glfw.cpp",
		"%{prj.location}/backends/imgui_impl_glfw.h",
		"%{prj.location}/misc/cpp/*.cpp",
		"%{prj.location}/misc/cpp/*.h",
	}
	includedirs 
	{
		"%{prj.location}",
		vendor_loc .. "glfw/include"
	}

feProject "assimp"
	warnings "Off"
	defines
	{
		-- The following formats are enabled
		--"ASSIMP_BUILD_NO_OBJ_IMPORTER",
		--"ASSIMP_BUILD_NO_COLLADA_IMPORTER",
		--"ASSIMP_BUILD_NO_BLEND_IMPORTER",
		-- The following does not work
		"ASSIMP_BUILD_NO_C4D_IMPORTER",
		"ASSIMP_BUILD_NO_STEP_IMPORTER",
		"ASSIMP_BUILD_NO_IFC_IMPORTER",
		-- The following are not currently used
		"ASSIMP_BUILD_NO_X_IMPORTER",
		"ASSIMP_BUILD_NO_MD3_IMPORTER",
		"ASSIMP_BUILD_NO_MDL_IMPORTER",
		"ASSIMP_BUILD_NO_MD2_IMPORTER",
		"ASSIMP_BUILD_NO_PLY_IMPORTER",
		"ASSIMP_BUILD_NO_ASE_IMPORTER",
		"ASSIMP_BUILD_NO_HMP_IMPORTER",
		"ASSIMP_BUILD_NO_SMD_IMPORTER",
		"ASSIMP_BUILD_NO_MDC_IMPORTER",
		"ASSIMP_BUILD_NO_MD5_IMPORTER",
		"ASSIMP_BUILD_NO_STL_IMPORTER",
		"ASSIMP_BUILD_NO_LWO_IMPORTER",
		"ASSIMP_BUILD_NO_DXF_IMPORTER",
		"ASSIMP_BUILD_NO_NFF_IMPORTER",
		"ASSIMP_BUILD_NO_RAW_IMPORTER",
		"ASSIMP_BUILD_NO_OFF_IMPORTER",
		"ASSIMP_BUILD_NO_AC_IMPORTER",
		"ASSIMP_BUILD_NO_BVH_IMPORTER",
		"ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
		"ASSIMP_BUILD_NO_IRR_IMPORTER",
		"ASSIMP_BUILD_NO_Q3D_IMPORTER",
		"ASSIMP_BUILD_NO_B3D_IMPORTER",
		"ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
		"ASSIMP_BUILD_NO_CSM_IMPORTER",
		"ASSIMP_BUILD_NO_3D_IMPORTER",
		"ASSIMP_BUILD_NO_LWS_IMPORTER",
		"ASSIMP_BUILD_NO_OGRE_IMPORTER",
		"ASSIMP_BUILD_NO_MS3D_IMPORTER",
		"ASSIMP_BUILD_NO_COB_IMPORTER",
		"ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
		"ASSIMP_BUILD_NO_NDO_IMPORTER",
		"ASSIMP_BUILD_NO_M3_IMPORTER",
		"ASSIMP_BUILD_NO_XGL_IMPORTER",
		-- Build openddl as a static lib to avoid dynamic lib errors
		"OPENDDL_STATIC_LIBARY"
	}
	includedirs
	{
		-- We provide our own assimp/config.h file so you dont have to touch cmake
		"%{wks.location}/vendor/assimp_config_h_fix",
		"%{prj.location}/include",
		"%{prj.location}/code",
		"%{prj.location}",
		-- Contrib
		"%{prj.location}/contrib/irrXML",
		"%{prj.location}/contrib/rapidjson/include",
		"%{prj.location}/contrib/unzip",
		"%{prj.location}/contrib/zlib",
		"%{prj.location}/contrib/openddlparser/include",
	}
	files
	{
		"%{prj.location}/code/**.c",
		"%{prj.location}/code/**.cpp",
		"%{prj.location}/code/**.h",
		-- Contrib
		"%{prj.location}/contrib/irrXML/**.cpp",
		"%{prj.location}/contrib/irrXML/**.h",
		"%{prj.location}/contrib/unzip/*.c",
		"%{prj.location}/contrib/unzip/*.h",
		"%{prj.location}/contrib/zlib/*.c",
		"%{prj.location}/contrib/zlib/*.h",
		"%{prj.location}/contrib/openddlparser/code/*.cpp",
	}
	filter "system:windows"
		defines "_CRT_SECURE_NO_WARNINGS"
