#pragma once

#include <string_view>
#include <memory>

namespace FoxEngine
{
	class Texture
	{
	public:
		enum struct Wrap
		{
			Repeat, Clamp
		};

		enum struct Filter
		{
			Nearest, Linear
		};

		enum struct Format
		{
			Rgba8
		};

		struct CreateInfo final
		{
			int width = 0;
			int height = 0;
			int depth = 0;
			Format format = Format::Rgba8;
			Wrap wrap = Wrap::Repeat;
			Filter min = Filter::Linear;
			Filter mag = Filter::Linear;
			std::string_view debugName;
		};

		struct UploadInfo final
		{
			int xoff = 0;
			int yoff = 0;
			int zoff = 0;
			int width = 0;
			int height = 0;
			int depth = 0;
			Format format = Format::Rgba8;
			const void* pixels = nullptr;
		};

		static std::unique_ptr<Texture> Create(const CreateInfo& info);
		static std::unique_ptr<Texture> Create(std::string_view resource);
	public:
		Texture() noexcept = default;
		virtual ~Texture() noexcept = default;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		virtual void Upload(const UploadInfo& info) = 0;
		virtual void Bind(unsigned int unit = 0) = 0;
	};
}