#include "ShaderOGL.hpp"

#include "../Blob.hpp"
#include "../Log.hpp"
#include "../experimental/Badge.hpp"

#include <glad/gl.h>

#include <utility>
#include <regex>

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

	ShaderOGL33::ShaderOGL33(const Shader::CreateInfo& info)
	{
		std::string common_pre = "#version 330 core\n\n";
		std::string common_post = "#line 1\n";

		std::string vertCommon = common_pre + "#define FE_VERT\n#define varying(type, name) out type name\n#define input(type, name, index) layout(location = index) in type name\n#define output(type, name, index)\n\n" + common_post;
		std::string fragCommon = common_pre + "#define FE_FRAG\n#define varying(type, name) in type name\n#define input(type, name, index)\n#define output(type, name, index) layout(location = index) out type name\n\n" + common_post;

		Blob source = Blob::FromFile(info.filename);
		std::string stringSource = std::string(reinterpret_cast<char*>(source.data()), source.size());

		std::regex regex("^@pragma\\s+([A-Za-z_]+)\\s");
		auto words_begin = std::sregex_iterator(stringSource.begin(), stringSource.end(), regex);
		auto words_end = std::sregex_iterator();

		for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
			std::smatch match = *i;

			std::string matches = match[1].str();

			if (matches == "backface_nocull")
				mCullsBackfaces = false;			

			stringSource = match.prefix().str() + match.suffix().str();
			
			Log::Info("Foxengine shader pragma: {}", matches);
			
		}

		std::string vertSource = vertCommon + stringSource;
		std::string fragSource = fragCommon + stringSource;

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

		GLint uniform_count = 0;
		glGetProgramiv(mHandle, GL_ACTIVE_UNIFORMS, &uniform_count);

		if (uniform_count != 0)
		{
			GLint 	max_name_len = 0;
			GLsizei length = 0;
			GLsizei count = 0;
			GLenum 	type = GL_NONE;
			glGetProgramiv(mHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);

			auto uniform_name = std::make_unique<char[]>(max_name_len);

			Log::Info("Uniform lookups: {}", info.filename);

			for (GLint i = 0; i < uniform_count; ++i)
			{
				glGetActiveUniform(mHandle, i, max_name_len, &length, &count, &type, uniform_name.get());

				int location = glGetUniformLocation(mHandle, uniform_name.get());

				Log::Info("{}: {}", uniform_name.get(), location);

				mUniforms.emplace(std::make_pair(std::string(uniform_name.get(), length), location));
			}
		}
	}

	ShaderOGL33::~ShaderOGL33() noexcept
	{
		if (mHandle)
			glDeleteProgram(mHandle);
	}

	ShaderOGL33::ShaderOGL33(ShaderOGL33&& other) noexcept
	{
		*this = std::move(other);
	}

	ShaderOGL33& ShaderOGL33::operator=(ShaderOGL33&& other) noexcept
	{
		std::swap(mHandle, other.mHandle);
		std::swap(mUniforms, other.mUniforms);
		std::swap(mCullsBackfaces, other.mCullsBackfaces);

		return *this;
	}

	void ShaderOGL33::Bind()
	{
		glUseProgram(mHandle);
	}

	void ShaderOGL33::Uniform1f(std::string_view name, float v0)
	{
		auto it = mUniforms.find(name);
		if (it == mUniforms.end()) return;

		Bind();
		glUniform1f(it->second, v0);
	}

	void ShaderOGL33::Uniform2f(std::string_view name, float v0, float v1)
	{
		auto it = mUniforms.find(name);
		if (it == mUniforms.end()) return;

		Bind();
		glUniform2f(it->second, v0, v1);
	}

	void ShaderOGL33::UniformMat4f(std::string_view name, const float* v0)
	{
		auto it = mUniforms.find(name);
		if (it == mUniforms.end()) return;

		Bind();
		glUniformMatrix4fv(it->second, 1, GL_FALSE, v0);
	}
}