#include "Window.hpp"

#include "Log.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <utility>

namespace FoxEngine
{
	static bool sReady = false;
	static unsigned int sCount = 0;

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::SwapInterval(int interval)
	{
		glfwSwapInterval(interval);
	}

	bool Window::LoadGLFunctions()
	{
		return gladLoadGL(&glfwGetProcAddress);
	}

	Window::Window(const CreateInfo& info)
	{
		if (!sReady)
		{
			glfwSetErrorCallback([](int error, const char* description)
				{
					Log::Error("Glfw error {}: {}", error, description);
				});

			if (!glfwInit())
				Log::Critical("Failed to initialize glfw");

			sReady = true;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		mHandle = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);

		if(!mHandle)
			Log::Critical("Failed to create window");

		++sCount;
	}

	Window::~Window() noexcept
	{
		if (mHandle)
		{
			glfwDestroyWindow(mHandle);
			--sCount;
		}

		if (sReady && sCount == 0)
		{
			glfwTerminate();
			sReady = false;
		}
	}

	Window::Window(Window&& other) noexcept
	{
		*this = std::move(other);
	}
	
	Window& Window::operator=(Window&& other) noexcept
	{
		std::swap(mHandle, other.mHandle);
		return *this;
	}

	void Window::SwapBuffers() const
	{
		glfwSwapBuffers(mHandle);
	}

	void Window::MakeContextCurrent() const
	{
		glfwMakeContextCurrent(mHandle);
	}
}