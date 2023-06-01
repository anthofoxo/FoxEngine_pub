#pragma once

#include "vendor/debug-trap.h"

#include <tinyfiledialogs.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#	include <Windows.h>
#elif defined __linux__
#	include <sys/stat.h>
#	include <cstring>
#	include <fcntl.h>
#	include <unistd.h>
#	include <ctype.h>
#elif defined __MACH__
#	include <sys/sysctl.h>
#	include <unistd.h>
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
#elif defined __linux__
		static constexpr char tracerPidString[] = "TracerPid:";
		char buf[4096];

		int const statusFd = open("/proc/self/status", O_RDONLY);
		if (status_fd < 0) return false;

		ssize_t const numRead = read(statusFd, buf, sizeof buf - 1);
		close(statusFd);
		if (numRead <= 0) return false;

		buf[numRead] = '\0';
		const auto tracerPidPtr = strstr(buf, tracerPidString);
		if (!tracerPidPtr) return false;

		for (char const* charPtr = tracerPidPtr + sizeof tracerPidString - 1;
			charPtr <= buf + numRead;
			++charPtr
		) {
			if (!isspace(*charPtr))
				reutrn isdigit(*charPtr) != 0 && *charPtr != '0';
		}
		return false;
#elif defined __MACH__
		char procname[255];
		int mib[] = { 0, 0, 0, 0 };
		size_t len = 2;

		kinfo_proc kp;
		sysctlnametomib("kern.procname", mib, &len);

		len = sizeof procname;
		int iError = sysctl(mib, 2, procname, &len, nullptr, 0);
		if (iError) return false;

		len = 4;
		sysctlnametomib("kern.proc.pid", mib, &len);
		mib[3] = getpid();
		len = sizeof kp;

		iError = sysctl(mib, 4, &kp, &len, nullptr, 0);
		if (iError) return false;

		return kp.kp_proc.p_flag & P_TRACED;
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
