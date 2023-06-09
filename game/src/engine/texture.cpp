#include "texture.hpp"

#include "vendor/stb_image.h"

#include <glad/gl.h>

#include <utility>
#include <stdexcept>

namespace FoxEngine
{	
	static unsigned int TextureFilterToFilter(Texture::Filter filter)
	{
		using enum Texture::Filter;

		switch (filter)
		{
		case Nearest:
			return GL_NEAREST;
		case Linear:
			return GL_LINEAR;
		}

		throw std::runtime_error("Invalid texture filter");
	}

	static unsigned int TextureWrapToWrap(Texture::Wrap wrap)
	{
		using enum Texture::Wrap;

		switch (wrap)
		{
		case Repeat:
			return GL_REPEAT;
		case Clamp:
			return GL_CLAMP_TO_EDGE;
		}

		throw std::runtime_error("Invalid texture wrap");
	}

	unsigned int TextureFormatToInternalFormat(ImageFormat format)
	{
		using enum Texture::Format;

		switch (format)
		{
		case Rgba8:
			return GL_RGBA8;
		case D24:
			return GL_DEPTH_COMPONENT24;
		case Rgba16f:
			return GL_RGBA16F;
		}

		throw std::runtime_error("Invalid texture format");
	}

	static unsigned int TextureFormatToFormat(Texture::Format format)
	{
		using enum Texture::Format;

		switch (format)
		{
		case Rgba8:
		case Rgba16f:
			return GL_RGBA;
		case D24:
			return GL_DEPTH_COMPONENT;
		}

		throw std::runtime_error("Invalid texture format");
	}

	static unsigned int TextureFormatToType(Texture::Format format)
	{
		using enum Texture::Format;

		switch (format)
		{
		case Rgba8:
		case D24:
			return GL_UNSIGNED_BYTE;
		case Rgba16f:
			return GL_HALF_FLOAT;
		}

		throw std::runtime_error("Invalid texture format");
	}

	class TextureOGL33 final : public Texture
	{
	public:
		constexpr TextureOGL33() noexcept = default;
		TextureOGL33(const CreateInfo& info);
		virtual ~TextureOGL33() noexcept;
		TextureOGL33(const TextureOGL33&) = delete;
		TextureOGL33& operator=(const TextureOGL33&) = delete;
		TextureOGL33(TextureOGL33&& other) noexcept;
		TextureOGL33& operator=(TextureOGL33&& other) noexcept;

		void Upload(const UploadInfo& info) override;
		void Bind(unsigned int unit = 0) override;

		unsigned int Handle() const noexcept override { return mHandle; }
		unsigned int Target() const noexcept override { return mTarget; }
	private:
		unsigned int mTarget = 0;
		unsigned int mHandle = 0;
	};

	TextureOGL33::TextureOGL33(const CreateInfo& info)
	{
		unsigned int dims = 0;
		if (info.width > 0) ++dims;
		if (info.height > 0) ++dims;
		if (info.depth > 0) ++dims;

		switch (dims)
		{
		case 1:
			mTarget = GL_TEXTURE_1D;
			break;
		case 2:
			mTarget = GL_TEXTURE_2D;
			break;
		case 3:
			mTarget = GL_TEXTURE_3D;
			break;
		default:
			throw std::runtime_error("Invalid texture dimensions");
		}

		glGenTextures(1, &mHandle);
		glBindTexture(mTarget, mHandle);

		switch (dims)
		{
		case 1:
			glTexImage1D(mTarget, 0, TextureFormatToInternalFormat(info.format), info.width, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		case 2:
			glTexImage2D(mTarget, 0, TextureFormatToInternalFormat(info.format), info.width, info.height, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		case 3:
			glTexImage3D(mTarget, 0, TextureFormatToInternalFormat(info.format), info.width, info.height, info.depth, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		}

		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, TextureFilterToFilter(info.min));
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, TextureFilterToFilter(info.mag));
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, TextureWrapToWrap(info.wrap));
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, TextureWrapToWrap(info.wrap));
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, TextureWrapToWrap(info.wrap));

		if (!info.debugName.empty() && GLAD_GL_KHR_debug)
		{
			glObjectLabel(GL_TEXTURE, mHandle, info.debugName.size(), info.debugName.data());
		}
	}

	TextureOGL33::~TextureOGL33() noexcept
	{
		if (mHandle)
			glDeleteTextures(1, &mHandle);
	}

	TextureOGL33::TextureOGL33(TextureOGL33&& other) noexcept
	{
		*this = std::move(other);
	}
	TextureOGL33& TextureOGL33::operator=(TextureOGL33&& other) noexcept
	{
		std::swap(mHandle, other.mHandle);
		std::swap(mTarget, other.mTarget);
		return *this;
	}

	void TextureOGL33::Upload(const UploadInfo& info)
	{
		unsigned int dims = 0;
		if (info.width > 0) ++dims;
		if (info.height > 0) ++dims;
		if (info.depth > 0) ++dims;

		Bind();

		switch (dims)
		{
		case 1:
			glTexSubImage1D(mTarget, 0, info.xoff, info.width, TextureFormatToFormat(info.format), TextureFormatToType(info.format), info.pixels);
			break;
		case 2:
			glTexSubImage2D(mTarget, 0, info.xoff, info.yoff, info.width, info.height, TextureFormatToFormat(info.format), TextureFormatToType(info.format), info.pixels);
			break;
		case 3:
			glTexSubImage3D(mTarget, 0, info.xoff, info.yoff, info.zoff, info.width, info.height, info.depth, TextureFormatToFormat(info.format), TextureFormatToType(info.format), info.pixels);
			break;
		default:
			throw std::runtime_error("Invalid texture dimensions");
		}
	}

	void TextureOGL33::Bind(unsigned int unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(mTarget, mHandle);
	}

	Poly<Texture> Texture::Create(const CreateInfo& info)
	{
		return Poly<Texture>(NullOf<TextureOGL33>, info);
	}

	Poly<Texture> Texture::Create(std::string_view resource)
	{
		CreateInfo createInfo;
		createInfo.debugName = resource;

		unsigned char* pixels = stbi_load(resource.data(), &createInfo.width, &createInfo.height, nullptr, 4);
		if (!pixels) return {};

		Poly<Texture> texture = Create(createInfo);

		UploadInfo uploadInfo;
		uploadInfo.pixels = pixels;
		uploadInfo.width = createInfo.width;
		uploadInfo.height = createInfo.height;

		texture->Upload(uploadInfo);

		stbi_image_free(pixels);

		return texture;
	}
}