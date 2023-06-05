#pragma once

#include <spdlog/fmt/bundled/core.h>
// #include <spdlog/fmt/bundled/args.h> // fmt::dynamic_format_arg_store<fmt::format_context>

#include <string_view>
#include <utility> // std::forward

namespace FoxEngine::Log
{
	// for lua to invoke
	/*void LogTrace(std::string_view format, void* L)
	{
		fmt::dynamic_format_arg_store<fmt::format_context> store;

		// dynamically populate argument store

		spdlog::trace(fmt::vformat(fmt::string_view(format.data(), format.size()), fmt::format_args(store)));
	}*/

	void Trace(std::string_view str);
	void Debug(std::string_view str);
	void Info(std::string_view str);
	void Warn(std::string_view str);
	void Error(std::string_view str);
	void Critical(std::string_view str);

	template<class... Args>
	std::string FormatArgs(std::string_view fmt, Args&&... args)
	{
		return fmt::format(fmt::runtime(fmt::string_view(fmt.data(), fmt.size())), std::forward<Args>(args)...);
	}

	template<class... Args>
	void Trace(std::string_view fmt, Args&&... args)
	{
		Trace(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void Debug(std::string_view fmt, Args&&... args)
	{
		Debug(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void Info(std::string_view fmt, Args&&... args)
	{
		Info(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void Warn(std::string_view fmt, Args&&... args)
	{
		Warn(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void Error(std::string_view fmt, Args&&... args)
	{
		Error(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}

	template<class... Args>
	void Critical(std::string_view fmt, Args&&... args)
	{
		Critical(FormatArgs<Args...>(fmt, std::forward<Args>(args)...));
	}
}