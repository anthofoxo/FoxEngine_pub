#pragma once

#include <spdlog/fmt/bundled/core.h>
// #include <spdlog/fmt/bundled/args.h> // fmt::dynamic_format_arg_store<fmt::format_context>

#include <string_view>
#include <utility> // std::forward

namespace FoxEngine
{
	// for lua to invoke
	/*void LogTrace(std::string_view format, void* L)
	{
		fmt::dynamic_format_arg_store<fmt::format_context> store;

		// dynamically populate argument store

		spdlog::trace(fmt::vformat(fmt::string_view(format.data(), format.size()), fmt::format_args(store)));
	}*/

	void LogTrace(std::string_view str);
	void LogDebug(std::string_view str);
	void LogInfo(std::string_view str);
	void LogWarn(std::string_view str);
	void LogError(std::string_view str);
	void LogCritical(std::string_view str);

	template<class... Args>
	std::string FormatArgs(std::string_view fmt, Args&&... args)
	{
		return fmt::format(fmt::runtime(fmt::string_view(fmt.data(), fmt.size())), std::forward<Args>(args)...);
	}

	template<class... Args>
	void LogTrace(std::string_view fmt, Args&&... args)
	{
		LogTrace(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void LogDebug(std::string_view fmt, Args&&... args)
	{
		LogDebug(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void LogInfo(std::string_view fmt, Args&&... args)
	{
		LogInfo(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void LogWarn(std::string_view fmt, Args&&... args)
	{
		LogWarn(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void LogError(std::string_view fmt, Args&&... args)
	{
		LogError(FormatArgs<Args>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void LogCritical(std::string_view fmt, Args&&... args)
	{
		LogCritical(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}
}