#pragma once

#include <unordered_map>
#include <string>
#include <string_view>

namespace FoxEngine
{
	struct StringHash
	{
		using hash_type = std::hash<std::string_view>;
		using is_transparent = void;

		std::size_t operator()(const char* str) const { return hash_type{}(str); }
		std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
		std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
	};

	template<class T>
	using UnorderedStringMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;
}