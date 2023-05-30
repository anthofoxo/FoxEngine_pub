#pragma once

// Texture is not final !!!
// Entire texture api is due for change

namespace FoxEngine
{
	enum struct TextureWrap
	{
		REPEAT, CLAMP
	};

	enum struct TextureFilter
	{
		NEAREST, LINEAR
	};

	enum struct TextureFormat
	{
		RGBA8
	};

	struct TextureCreateInfo final
	{
		int width = 0;
		int height = 0;
		int depth = 0;
		TextureFormat format;
		TextureWrap wrap;
		TextureFilter min;
		TextureFilter mag;
		const char* debug_name;
	};

	class Texture final
	{
	public:
		Texture() noexcept = default;
		Texture(const TextureCreateInfo& info);
		~Texture() noexcept;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;
		friend void swap(Texture& lhs, Texture& rhs) noexcept;

		void Upload(int xoff, int width, TextureFormat format, const void* pixels);
		void Upload(int xoff, int yoff, int width, int height, TextureFormat format, const void* pixels);
		void Upload(int xoff, int yoff, int zoff, int width, int height, int depth, TextureFormat format, const void* pixels);

		void Bind();

		inline unsigned int Target() const noexcept { return m_target; }
		inline unsigned int Handle() const noexcept { return m_handle; }
	private:
		unsigned int m_target = 0;
		unsigned int m_handle = 0;
	};
}