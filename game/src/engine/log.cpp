#pragma once

#include <spdlog/spdlog.h>

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

	void LogCritical(std::string_view str)
	{
		// put a breakpoint here
		spdlog::critical(str);
	}
}