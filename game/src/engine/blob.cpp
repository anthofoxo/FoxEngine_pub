#include "blob.hpp"

#include <utility>
#include <fstream>

namespace FoxEngine
{
	Blob Blob::FromFile(std::string_view filename)
	{
		std::ifstream in{ filename.data(), std::ios::in | std::ios::binary };

		if (!in) throw Exception::FileRead{ "Failed to load file" };
		
		in.seekg(0, std::ios::end);
		Blob blob(in.tellg());
		in.seekg(0, std::ios::beg);

		in.read(reinterpret_cast<char*>(blob.data()), blob.size());

		return blob;
	}

	Blob::Blob(std::size_t size)
	{
		if (size == 0) return;

		mData = new std::byte[size];
		mSize = size;
	}

	Blob::Blob(std::byte* data, std::size_t size) noexcept
		: mData(data), mSize(size) {}

	Blob::~Blob() noexcept
	{
		delete[] mData;
	}

	Blob::Blob(Blob&& other) noexcept
	{
		swap(*this, other);
	}

	Blob& Blob::operator=(Blob&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	void swap(Blob& lhs, Blob& rhs) noexcept
	{
		using std::swap;
		swap(lhs.mData, rhs.mData);
		swap(lhs.mSize, rhs.mSize);
	}
}