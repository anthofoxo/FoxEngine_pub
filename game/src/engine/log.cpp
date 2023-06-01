#pragma once

#include "vendor/debug-trap.h"

#include <tinyfiledialogs.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#	include <Windows.h>
#endif

namespace FoxEngine
{
	namespace
	{
		struct AutoLog final
		{
			AutoLog()
			{
				spdlog::set_level(spdlog::level::trace);
			}

			~AutoLog()
			{
				spdlog::shutdown();
			}
		};

		static AutoLog sAutoLog;
	}

	void LogTrace(std::string_view str)
	{
		spdlog::trace(str);
	}

	void LogDebug(std::string_view str)
	{
		spdlog::debug(str);
	}

	void LogInfo(std::string_view str)
	{
		spdlog::info(str);
	}

	void LogWarn(std::string_view str)
	{
		spdlog::warn(str);
	}

	void LogError(std::string_view str)
	{
		spdlog::error(str);
	}

	static bool IsDebuggerAttached()
	{
#ifdef _WIN32
		return ::IsDebuggerPresent();
#else
		// Assume no debugger is attached
		return false;
#endif
	}

	void LogCritical(std::string_view str)
	{
		spdlog::critical(str);
		
		if (IsDebuggerAttached())
		{
			psnip_trap();
			return;
		}

		const int response = tinyfd_messageBox("FoxEngine critical error! Is debugger attached?", str.data(), "yesno", "error", 0);
		
		if(response == 1)
			psnip_trap();
	}
}