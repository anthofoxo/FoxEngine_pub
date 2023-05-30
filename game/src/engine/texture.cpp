#include "texture.hpp"

#include <glad/gl.h>

#include <utility>
#include <stdexcept>

namespace FoxEngine
{	
	static unsigned int TextureFilterToFilter(TextureFilter filter)
	{
		using enum TextureFilter;

		switch (filter)
		{
		case NEAREST:
			return GL_NEAREST;
		case LINEAR:
			return GL_LINEAR;
		}

		throw std::runtime_error("Invalid texture filter");
	}

	static unsigned int TextureWrapToWrap(TextureWrap wrap)
	{
		using enum TextureWrap;

		switch (wrap)
		{
		case REPEAT:
			return GL_REPEAT;
		case CLAMP:
			return GL_CLAMP_TO_EDGE;
		}

		throw std::runtime_error("Invalid texture wrap");
	}

	static unsigned int TextureFormatToInternalFormat(TextureFormat format)
	{
		using enum TextureFormat;

		switch (format)
		{
		case RGBA8:
			return GL_RGBA8;
		}

		throw std::runtime_error("Invalid texture format");
	}

	static unsigned int TextureFormatToFormat(TextureFormat format)
	{
		using enum TextureFormat;

		switch (format)
		{
		case RGBA8:
			return GL_RGBA;
		}

		throw std::runtime_error("Invalid texture format");
	}

	static unsigned int TextureFormatToType(TextureFormat format)
	{
		using enum TextureFormat;

		switch (format)
		{
		case RGBA8:
			return GL_UNSIGNED_BYTE;	
		}

		throw std::runtime_error("Invalid texture format");
	}

	Texture::Texture(const TextureCreateInfo& info)
	{
		unsigned int dims = 0;
		if (info.width > 0) ++dims;
		if (info.height > 0) ++dims;
		if (info.depth > 0) ++dims;

		switch (dims)
		{
		case 1:
			m_target = GL_TEXTURE_1D;
			break;
		case 2:
			m_target = GL_TEXTURE_2D;
			break;
		case 3:
			m_target = GL_TEXTURE_3D;
			break;
		default:
			throw std::runtime_error("Invalid texture dimensions");
		}

		glGenTextures(1, &m_handle);
		Bind();
		
		switch (dims)
		{
		case 1:
			glTexImage1D(m_target, 0, TextureFormatToInternalFormat(info.format), info.width, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		case 2:
			glTexImage2D(m_target, 0, TextureFormatToInternalFormat(info.format), info.width, info.height, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		case 3:
			glTexImage3D(m_target, 0, TextureFormatToInternalFormat(info.format), info.width, info.height, info.depth, 0, TextureFormatToFormat(info.format), TextureFormatToType(info.format), nullptr);
			break;
		}

		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, TextureFilterToFilter(info.min));
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, TextureFilterToFilter(info.mag));
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, TextureWrapToWrap(info.wrap));
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, TextureWrapToWrap(info.wrap));
		glTexParameteri(m_target, GL_TEXTURE_WRAP_R, TextureWrapToWrap(info.wrap));
	}

	Texture::~Texture() noexcept
	{
		if (m_handle)
			glDeleteTextures(1, &m_handle);
	}

	Texture::Texture(Texture&& other) noexcept
	{
		swap(*this, other);
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	void swap(Texture& lhs, Texture& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_target, rhs.m_target);
		swap(lhs.m_handle, rhs.m_handle);
	}

	void Texture::Bind()
	{
		glBindTexture(m_target, m_handle);
	}
		 
	void Texture::Upload(int xoff, int width, TextureFormat format, const void* pixels)
	{
		glTexSubImage1D(m_target, 0, xoff, width, TextureFormatToFormat(format), TextureFormatToType(format), pixels);
	}

	void Texture::Upload(int xoff, int yoff, int width, int height, TextureFormat format, const void* pixels)
	{
		glTexSubImage2D(m_target, 0, xoff, yoff, width, height, TextureFormatToFormat(format), TextureFormatToType(format), pixels);
	}

	void Texture::Upload(int xoff, int yoff, int zoff, int width, int height, int depth, TextureFormat format, const void* pixels)
	{
		glTexSubImage3D(m_target, 0, xoff, yoff, zoff, width, height, depth, TextureFormatToFormat(format), TextureFormatToType(format), pixels);
	}
}