#pragma once

#include <cstddef>
#include <string_view>
#include <stdexcept>

// Perhaps change std::byte* to void*

namespace FoxEngine
{
	namespace Exception
	{
		class FileRead : public std::runtime_error
		{
		public:
			FileRead(const std::string& msg) : std::runtime_error{ msg } {}
		};
	}

	class Blob final
	{
	public:
		[[nodiscard]] static Blob FromFile(std::string_view filename);

		Blob() noexcept = default;
		explicit Blob(std::size_t size);
		Blob(std::byte* data, std::size_t size) noexcept;
		~Blob() noexcept;
		Blob(const Blob&) = delete;
		Blob& operator=(const Blob&) = delete;
		Blob(Blob&& other) noexcept;
		Blob& operator=(Blob&& other) noexcept;
		friend void swap(Blob& lhs, Blob& rhs) noexcept;

		explicit operator bool() const { return mData; }
		std::byte* data() { return mData; }
		const std::byte* data() const { return mData; }
		const std::size_t size() const { return mSize; }
	private:
		std::byte* mData{};
		std::size_t mSize{};
	};
}