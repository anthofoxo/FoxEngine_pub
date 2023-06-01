#pragma once

#include <utility>
#include <type_traits>

// Experimental stuff, not ready for use everywhere

namespace FoxEngine::Experimental
{
	template<class T, class Action>
	struct Deleter
	{
		Deleter(const T& t) : value(t) {}
		Deleter(T&& t) : value(std::move(t)) {}

		operator T& () { return value; }

		~Deleter() { Action{}(value); }

		T value;
	};

	template<class T>
	concept HandleType = std::is_integral_v<T> || std::is_pointer_v<T>;

	template<class T>
	class Badge
	{
	public:
		using Type = T;
	private:
		friend T;
		constexpr Badge() noexcept = default;
		Badge(Badge const&) = delete;
		Badge& operator=(Badge const&) = delete;
		Badge(Badge&&) noexcept = delete;
		Badge& operator=(Badge&&) noexcept = delete;
	};

	template<HandleType Type, class Owner>
	class Handle final
	{
	public:
		inline Handle() noexcept : mHandle({}) {}
		inline Handle(Badge<Owner>, Type value) noexcept : mHandle(value) {}
		inline explicit operator Type() const noexcept { return mHandle; }
	private:
		Type mHandle;
	};
}