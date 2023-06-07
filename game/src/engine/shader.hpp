#pragma once

#include <string_view>
#include "Poly.hpp"

// Shaders need a ton of work still, mostly for preprocessing

namespace FoxEngine
{
	class Shader
	{
	public:
		struct CreateInfo final
		{
			std::string_view filename;
			std::string_view debugName;
		};
	public:
		static Poly<Shader> Create(const CreateInfo& info);

		constexpr Shader() noexcept = default;
		virtual ~Shader() noexcept = default;
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) noexcept = delete;
		Shader& operator=(Shader&&) noexcept = delete;

		virtual void Bind() = 0;
		virtual void Uniform1f(std::string_view name, float v0) = 0;  // Deprecate once uniform buffers work
		virtual void Uniform2f(std::string_view name, float v0, float v1) = 0; // Deprecate once uniform buffers work
		virtual void UniformMat4f(std::string_view name, const float* v0) = 0;  // Deprecate once uniform buffers work
	};
}