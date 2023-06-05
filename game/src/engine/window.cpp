#include "window.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <utility>
#include <stdexcept>

namespace FoxEngine
{
	static bool s_ready = false;
	static unsigned int s_count = 0;

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
		if (!s_ready)
		{
			// we sould register error callbacks
			//static void glfw_error_callback(int error, const char* description)
			//{
			//	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
			//}

			if (!glfwInit())
				throw std::runtime_error("Failed to initialize glfw");

			s_ready = true;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_handle = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);

		if(!m_handle)
			throw std::runtime_error("Failed to create window");

		++s_count;
	}

	Window::~Window() noexcept
	{
		if (m_handle)
		{
			glfwDestroyWindow(m_handle);
			--s_count;
		}

		if (s_ready && s_count == 0)
		{
			glfwTerminate();
			s_ready = false;
		}
	}

	Window::Window(Window&& other) noexcept
	{
		swap(*this, other);
	}
	
	Window& Window::operator=(Window&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	void swap(Window& lhs, Window& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_handle, rhs.m_handle);
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_handle);
	}

	void Window::MakeContextCurrent()
	{
		glfwMakeContextCurrent(m_handle);
	}
}