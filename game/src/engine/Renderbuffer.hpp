#pragma once

#include "Poly.hpp"
#include "Texture.hpp"

#include <string_view>

//

#include <glad/gl.h>

namespace FoxEngine
{
	class Renderbuffer
	{
	public:
		struct CreateInfo final
		{
			int width = 0;
			int height = 0;
			FoxEngine::ImageFormat format = FoxEngine::ImageFormat::D24;
			std::string_view debugName;
		};

		static FoxEngine::Poly<Renderbuffer> Create(const CreateInfo& info);

		constexpr Renderbuffer() noexcept = default;
		virtual ~Renderbuffer() noexcept = default;
		Renderbuffer(const Renderbuffer&) = delete;
		Renderbuffer& operator=(const Renderbuffer&) = delete;
		Renderbuffer(Renderbuffer&&) noexcept = delete;
		Renderbuffer& operator=(Renderbuffer&&) noexcept = delete;

		virtual unsigned int Handle() const noexcept = 0;
	};

	class RenderbufferOGL33 : public Renderbuffer
	{
	public:
		constexpr RenderbufferOGL33() noexcept = default;

		RenderbufferOGL33(const CreateInfo& info)
		{
			glGenRenderbuffers(1, &mHandle);
			glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
			glRenderbufferStorage(GL_RENDERBUFFER, TextureFormatToInternalFormat(info.format), info.width, info.height);
		}

		virtual ~RenderbufferOGL33() noexcept
		{
			if (mHandle)
				glDeleteRenderbuffers(1, &mHandle);
		}

		RenderbufferOGL33(const RenderbufferOGL33&) = delete;
		RenderbufferOGL33& operator=(const RenderbufferOGL33&) = delete;

		RenderbufferOGL33(RenderbufferOGL33&& other) noexcept
		{
			*this = std::move(other);
		}
		RenderbufferOGL33& operator=(RenderbufferOGL33&& other) noexcept
		{
			std::swap(mHandle, other.mHandle);
			return *this;
		}

		unsigned int Handle() const noexcept override { return mHandle; }
	private:
		unsigned int mHandle = 0;
	};

	FoxEngine::Poly<Renderbuffer> Renderbuffer::Create(const CreateInfo& info)
	{
		return Poly<Renderbuffer>(NullOf<RenderbufferOGL33>, info);
	}
}