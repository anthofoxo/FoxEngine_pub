#pragma once

#include "../Shader.hpp"

#include <string>
#include <unordered_map>

namespace FoxEngine
{
	class ShaderOGL33 final : public Shader
	{
	public:
		ShaderOGL33() noexcept = default;
		ShaderOGL33(const Shader::CreateInfo& info);
		virtual ~ShaderOGL33() noexcept;

		ShaderOGL33(const ShaderOGL33&) = delete;
		ShaderOGL33& operator=(const ShaderOGL33&) = delete;
		ShaderOGL33(ShaderOGL33&& other) noexcept;
		ShaderOGL33& operator=(ShaderOGL33&& other) noexcept;

		void Bind() override;

		void Uniform1f(std::string_view name, float v0) override;
		void Uniform2f(std::string_view name, float v0, float v1) override;
		void UniformMat4f(std::string_view name, const float* v0) override;
	private:
		struct StringHash
		{
			using hash_type = std::hash<std::string_view>;
			using is_transparent = void;

			std::size_t operator()(const char* str) const { return hash_type{}(str); }
			std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
			std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
		};

		unsigned int mHandle = 0;
		std::unordered_map<std::string, int, StringHash, std::equal_to<>> mUniforms;
	};
}