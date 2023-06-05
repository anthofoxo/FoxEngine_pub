#pragma once

// GLFW3.h
typedef struct GLFWwindow fe_window_handle;

namespace FoxEngine
{
	// May be moved behind an abstraction later if we change window apis
	class Window final
	{
	public:
		struct CreateInfo final
		{
			int width = 1280;
			int height = 720;
			const char* title = "FoxEngine";
		};
	public:
		static void PollEvents();
		static void SwapInterval(int interval);
		static bool LoadGLFunctions();

		constexpr Window() noexcept = default;
		Window(const CreateInfo& info);
		~Window() noexcept;

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;

		GLFWwindow* Handle() const noexcept { return mHandle; }

		void SwapBuffers() const;
		void MakeContextCurrent() const;
	private:
		GLFWwindow* mHandle = nullptr;
	};
}