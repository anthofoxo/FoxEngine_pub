#pragma once

#include "../Shader.hpp"
#include "../UnorderedMapString.hpp"

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
		bool CullsBackFaces() const noexcept override { return mCullsBackfaces; }
	private:
		bool mCullsBackfaces = true;
		unsigned int mHandle = 0;
		UnorderedStringMap<int> mUniforms;
	};
}