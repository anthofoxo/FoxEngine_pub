#include "shader.hpp"

#include "experimental/Badge.hpp"
#include "blob.hpp"

#include <glad/gl.h>

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <regex>
#include <algorithm>
#include <memory>
#include <iostream>
#include <unordered_map>

namespace FoxEngine
{
	struct ShaderDeleter final
	{
		void operator()(unsigned int value) const
		{
			glDeleteShader(value);
		}
	};

	static Experimental::Deleter<unsigned int, ShaderDeleter> CreateShader(unsigned int type, std::string_view string)
	{
		const char* cStr = string.data();
		int size = string.size();

		Experimental::Deleter<unsigned int, ShaderDeleter> shader = glCreateShader(type);
		glShaderSource(shader, 1, &cStr, &size);
		glCompileShader(shader);

		int logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0)
		{
			std::string string;
			string.resize(logLength);

			glGetShaderInfoLog(shader, logLength, nullptr, string.data());

			throw std::runtime_error{ string };
		}

		return shader;
	}

	class ShaderOGL33 final : public Shader
	{
	public:
		ShaderOGL33(const Shader::CreateInfo& info)
		{
			std::string common_pre = "#version 330 core\n\n";
			std::string common_post = "#line 1\n";

			std::string vertCommon = common_pre + "#define FE_VERT\n#define varying(type, name) out type name\n#define input(type, name, index) layout(location = index) in type name\n#define output(type, name, index)\n\n" + common_post;
			std::string fragCommon = common_pre + "#define FE_FRAG\n#define varying(type, name) in type name\n#define input(type, name, index)\n#define output(type, name, index) layout(location = index) out type name\n\n" + common_post;
			
			Blob source = Blob::FromFile(info.filename);

			std::string vertSource = vertCommon + std::string(reinterpret_cast<char*>(source.data()), source.size());
			std::string fragSource = fragCommon + std::string(reinterpret_cast<char*>(source.data()), source.size());

			auto vert = CreateShader(GL_VERTEX_SHADER, vertSource);
			auto frag = CreateShader(GL_FRAGMENT_SHADER, fragSource);

			mHandle = glCreateProgram();

			if (GLAD_GL_KHR_debug && !info.debugName.empty())
				glObjectLabel(GL_PROGRAM, mHandle, info.debugName.size(), info.debugName.data());

			glAttachShader(mHandle, vert);
			glAttachShader(mHandle, frag);

			glLinkProgram(mHandle);

			int logLength;
			glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);

			if (logLength > 0)
			{
				std::string string;
				string.resize(logLength);

				glGetProgramInfoLog(mHandle, logLength, nullptr, string.data());

				throw std::runtime_error{ string };
			}
		}

		virtual ~ShaderOGL33() noexcept
		{
			if (mHandle)
				glDeleteProgram(mHandle);
		}

		void Bind() override
		{
			glUseProgram(mHandle);
		}

		void Uniform1f(std::string_view name, float v0) override
		{
			Bind();
			int location = glGetUniformLocation(mHandle, name.data());
			glUniform1f(location, v0);
		}

		void Uniform2f(std::string_view name, float v0, float v1) override
		{
			Bind();
			int location = glGetUniformLocation(mHandle, name.data());
			glUniform2f(location, v0, v1);
		}

		void UniformMat4f(std::string_view name, const float* v0) override
		{
			Bind();
			int location = glGetUniformLocation(mHandle, name.data());
			glUniformMatrix4fv(location, 1, GL_FALSE, v0);
		}
	private:
		unsigned int mHandle;
	};

	std::unique_ptr<Shader> Shader::Create(const Shader::CreateInfo& info)
	{
		return std::make_unique<ShaderOGL33>(info);
	}
}